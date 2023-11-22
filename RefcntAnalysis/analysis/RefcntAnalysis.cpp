#include <llvm/Support/Debug.h>
#include <llvm/Support/GraphWriter.h>
#include <llvm/InitializePasses.h>
#include <unistd.h>
#include <deque>
#include "RefcntPass.h"
#include "config.h"
#include "printer/CFGDrawer.hpp"

// formated debug output helper
#define DEBUG_PRINT_FORMAT(...) \
    do { \
        if (params.debug) { \
            errs() << raw_fd_ostream::YELLOW \
                   << format( __VA_ARGS__) \
                   << raw_fd_ostream::RESET; \
        } \
    } while (0)

#define DEBUG_PRINT_STR(str) \
    do { \
        if (params.debug) { \
            errs() << raw_fd_ostream::YELLOW \
                   << str \
                   << raw_fd_ostream::RESET; \
        } \
    } while (0)


void llvm::initializeRefcntPassPass(llvm::PassRegistry &Registry) {
    initializeAAResultsWrapperPassPass(Registry);
}

void RefcntPass::init_pass() {
    Parser::JsonParser parser;
    if (parser.parse("../../settings.json")) {
        DEBUG_PRINT_STR("<init_pass> use settings from settings.json\n");
    } else {
        DEBUG_PRINT_STR("<init_pass> use default settings\n");
    }
    params = parser.anaParams;
    initAPIInfo();

    DEBUG_PRINT_STR("<init_pass> initialization done\n");
}

bool RefcntPass::transferNode(BasicBlock *bb, AAHelper &aaHelper) {
    RCFact::Fact oldOutFact = outFacts[bb];
    for (const Instruction &inst: *bb) {
        unsigned op = inst.getOpcode();
        switch (op) {
            case Instruction::Call: {
                auto *callInst = dyn_cast<CallInst>(&inst);
                llvm::Function *calledFunction = callInst->getCalledFunction();

                auto name = calledFunction->getName();
                DEBUG_PRINT_FORMAT("<transferNode> call: %s\n", name.data());

                if (name == INCREF_STR || name == XINCREF_STR) {
                    Value *memref = aaHelper.getMemRef(callInst->getArgOperand(0));
                    assert(memref != nullptr);
                    DEBUG_PRINT_FORMAT("<transferNode> memref: %s\n", memref->getName().data());
                    if (outerMemSet.find(memref) != outerMemSet.end()) {
                        DEBUG_PRINT_STR("<transferNode> outer memref should not be managed\n");
                        errList.emplace_back(0, memref, bb->getParent(), callInst->getArgOperand(0),
                                             RefcntErrorMsg::NMC);
                        break;
                    }
                    // update outfacts
                    int incnt = inFacts[bb].at(memref);
                    if (incnt == RCFact::Fact::NAC)break;
                    outFacts[bb].update(memref, incnt == RCFact::Fact::UNDEF ? 1 : incnt + 1);
                    if (outFacts[bb].at(memref) > RCFact::MAX_REF_CNT) {
                        DEBUG_PRINT_STR("<transferNode> maximum refcount exceeded, treat as MLK.\n");
                        RefcntErrorMsg msg(0, memref, bb->getParent(), callInst->getArgOperand(0), RefcntErrorMsg::MLK);
                        errList.emplace_back(msg);
                        outFacts[bb].update(memref, RCFact::Fact::NAC);
                    }
                } else if (name == DECREF_STR || name == XDECREF_STR) {
                    Value *memref = aaHelper.getMemRef(callInst->getArgOperand(0));
                    assert(memref != nullptr);
                    DEBUG_PRINT_FORMAT("<transferNode> memref: %s\n", memref->getName().data());
                    if (outerMemSet.find(memref) != outerMemSet.end()) {
                        DEBUG_PRINT_STR("<transferNode> outer memref should not be managed\n");
                        errList.emplace_back(0, memref, bb->getParent(), callInst->getArgOperand(0),
                                             RefcntErrorMsg::NMC);
                        break;
                    }
                    int incnt = inFacts[bb].at(memref);
                    if (incnt == RCFact::Fact::NAC)break;
                    outFacts[bb].update(memref, incnt == RCFact::Fact::UNDEF ? RCFact::Fact::NAC : incnt - 1);
                    if (outFacts[bb].at(memref) < 0) {
                        RefcntErrorMsg msg(0, memref, bb->getParent(), callInst->getArgOperand(0), RefcntErrorMsg::UAF);
                        errList.emplace_back(msg);
                    }
                } else if (name == NEWREF_STR) {
                    Value *memref = aaHelper.getMemRef(callInst->getArgOperand(0));
                    assert(memref != nullptr);
                    DEBUG_PRINT_FORMAT("<transferNode> memref: %s\n", memref->getName().data());
                    int incnt = inFacts[bb].at(memref);
                    if (incnt == RCFact::Fact::NAC)break;
                    outFacts[bb].update(memref, 1);
                } else {
                    // TODO: check if it's a special API, aka in retStrongAPI/retWeakAPI/argBorrowAPI/argStealAPI
                    // in inter analysis, we should first call intra analysis on the called function, the function will
                    // show if the return value is a strong/weak reference, and if the argument is a borrow/steal reference
                    // below is an example:
                    // case 1: return strong reference
                    //if (name == "PyModule_Create2" || name == "PyErr_NewException") {
                    auto apiType = getAPIType(name.str());
                    if (apiType.tag == IGNORE) {
                        // like PyList_Size()
                        break;
                    } else if (apiType.tag == STRONGRET) {
                        const auto *ret = dyn_cast<Value>(callInst);
                        Value *memref = aaHelper.getMemRef(ret);
                        assert(memref != nullptr);
                        DEBUG_PRINT_FORMAT("<transferNode> memref: %s\n", memref->getName().data());
                        int incnt = inFacts[bb].at(memref);
                        if (incnt == RCFact::Fact::NAC)break;
                        outFacts[bb].update(memref, 1);
                    } else if (apiType.tag == WEAKRET) {
                        const auto *ret = dyn_cast<Value>(callInst);
                        Value *memref = aaHelper.getMemRef(ret);
                        assert(memref != nullptr);
                        DEBUG_PRINT_FORMAT("<transferNode> memref: %s\n", memref->getName().data());
                        int incnt = inFacts[bb].at(memref);
                        if (incnt == RCFact::Fact::NAC)break;
                        outFacts[bb].update(memref, 0);
                        break;
                    } else if (apiType.tag == PARAMSTEAL) {
                        for (int i = 0; i < apiType.target_param_num; i++) {
                            auto *curParam = callInst->getArgOperand(apiType.param_index[i]);
                            Value *memref = aaHelper.getMemRef(curParam);
                            assert(memref != nullptr);
                            DEBUG_PRINT_FORMAT("<transferNode> memref: %s\n", memref->getName().data());
                            int incnt = inFacts[bb].at(memref);
                            if (incnt == RCFact::Fact::NAC)break;

                            // stolen parameters should be kept a record
                            outerMemSet.emplace(memref);
                            outFacts[bb].update(memref, 0);
                        }
                    } else if (apiType.tag == PARAMBORROW) {
                        for (int i = 0; i < apiType.target_param_num; i++) {
                            auto *curParam = callInst->getArgOperand(apiType.param_index[i]);
                            Value *memref = aaHelper.getMemRef(curParam);
                            assert(memref != nullptr);
                            DEBUG_PRINT_FORMAT("<transferNode> memref: %s\n", memref->getName().data());
                            break;
                        }
                    } else if (apiType.tag == PARAMSTRONG) {
                        for (int i = 0; i < apiType.target_param_num; i++) {
                            auto *curParam = callInst->getArgOperand(apiType.param_index[i]);
                            Value *memref = aaHelper.getMemRef(curParam);
                            assert(memref != nullptr);
                            DEBUG_PRINT_FORMAT("<transferNode> memref: %s\n", memref->getName().data());
                            break;
                        }
                    } else {
                        if (params.analysisMode == "inter" &&
                            funcVisited.find(calledFunction) == funcVisited.end()) { // buggy
                            // user defined function, store in out facts and errlist of current analysis.
                            std::unordered_map<BasicBlock *, RCFact::Fact> curInFacts = inFacts;
                            std::unordered_map<BasicBlock *, RCFact::Fact> curOutFacts = outFacts;
                            std::list<RefcntErrorMsg> curErrList = errList;
                            std::unordered_set<Value *> curOuterMemSet = outerMemSet;
                            inFacts.clear(), outFacts.clear(), errList.clear(), outerMemSet.clear();
                            refcntAnalysis(calledFunction);
                            // check the error list of called function if its argument is a borrow/steal reference
                            // TODO: check arguments and return variables to identify the function's type

                            // recover facts and errlist
                            inFacts = curInFacts;
                            outFacts = curOutFacts;
                            outerMemSet = curOuterMemSet;
                            errList = curErrList;
                            std::vector<Value *> argVec;
                            std::vector<Value *> paramVec;
                            for (llvm::Function::arg_iterator arg = calledFunction->arg_begin();
                                 arg != calledFunction->arg_end(); ++arg) {
                                Value *param = &*arg;
                                Value *memref = aaHelper.getMemRef(param);
                                paramVec.emplace_back(memref);
                            }
                            for (unsigned int i = 0, e = callInst->getNumOperands(); i != e; ++i) {
                                Value *operand = callInst->getOperand(i);
                                Value *memref = aaHelper.getMemRef(operand);
                                if (isa<Argument>(operand)) argVec.emplace_back(memref);
                            }
                            assert(argVec.size() == paramVec.size());
                            std::map<Value *, Value *> mapArg;
                            for (int i = 0; i < argVec.size(); ++i)
                                mapArg.insert(std::pair<Value *, Value *>(paramVec[i], argVec[i]));
                            for (auto inf = endFuncTab[calledFunction].begin();
                                 inf != endFuncTab[calledFunction].end(); inf++) {
                                long cnt = inf->ref;
                                Value *memref = inf->memref;
                                auto iter = mapArg.find(memref);
                                if (iter != mapArg.end() && cnt == -1) { // buggy
                                    Value *callerVar = iter->second;
                                    outFacts[bb].update(callerVar, -1);
                                    for (auto it = errTab[calledFunction].begin();
                                         it != errTab[calledFunction].end(); it++) {
                                        if (it->memref == memref) errTab[calledFunction].erase(it);
                                    }
                                }
                                if (memref == curRetVal && cnt == 1) {
                                    auto *ret = dyn_cast<Value>(callInst);
                                    Value *ret_memref = aaHelper.getMemRef(ret);
                                    outFacts[bb].update(ret_memref, 1);
                                    for (auto it = errTab[calledFunction].begin();
                                         it != errTab[calledFunction].end(); it++) {
                                        if (it->memref == ret_memref) errTab[calledFunction].erase(it);
                                    }
                                }
                            }
                        } else {
                            // leave out this instruction
                        }
                    }
                }
                break;
            } // end of case Call
            case Instruction::Ret: {
                auto *returnInst = dyn_cast<ReturnInst>(&inst);
                Value *retVal = returnInst->getReturnValue();
                Value *memref = aaHelper.getMemRef(retVal);
                curRetVal = memref;
            }
            default:
                break;
        }
    }
    return oldOutFact != outFacts[bb];
}

bool RefcntPass::transferEdge(BasicBlock *src, BasicBlock *dst) {
    return false;
}

void RefcntPass::refcntAnalysis(Function *cur_func) {
    // start from the entry function,
    //
    // TODO: do additional set up before traversing entry

    funcVisited.insert(cur_func);
    for (BasicBlock &bb: *cur_func) {
        inFacts.insert({&bb, RCFact::Fact()});
        outFacts.insert({&bb, RCFact::Fact()});
    }
    // get alias analysis result
    auto &AA = getAnalysis<AAResultsWrapperPass>(*cur_func).getAAResults();
    AAHelper aaHelper(AA, cur_func);
    // get all load and store instructions and do the following things:
    // example: %5 = load i32, ptr %i, align 4, then call aaHelper.addAAValueAs(%5, %i)
    // example: store ptr %list, ptr %list.addr, align 8, then call aaHelper.addAAValueAs(%list.addr, %list)
    // should check that value to be stored should not be constant
    for (BasicBlock &bb: *cur_func) {
        for (Instruction &inst: bb) {
            unsigned op = inst.getOpcode();
            switch (op) {
                case Instruction::Load: {
                    auto *loadInst = dyn_cast<LoadInst>(&inst);
                    Value *memref = aaHelper.getMemRef(loadInst->getPointerOperand());
                    assert(memref != nullptr);
                    DEBUG_PRINT_FORMAT("<refcntAnalysis> memref: %s\n", memref->getName().data());
                    aaHelper.addAAValueMemRef(loadInst, memref);
                    break;
                }
                case Instruction::Store: {
                    auto *storeInst = dyn_cast<StoreInst>(&inst);
                    Value *memref = aaHelper.getMemRef(storeInst->getPointerOperand());
                    assert(memref != nullptr);
                    DEBUG_PRINT_FORMAT("<refcntAnalysis> memref: %s\n", memref->getName().data());
                    aaHelper.addAAValueMemRef(storeInst->getValueOperand(), memref);
                    break;
                }
                default:
                    break;
            }
        }
    }
    DEBUG_PRINT_STR("<doAnalysis> intra analysis started\n");
    DEBUG_PRINT_FORMAT("<doAnalysis> function: %s\n", cur_func->getName().data());
    DEBUG_PRINT_FORMAT(
            "<intraAnalysis>---------AA sets begin---------\n%s\n<doAnalysis>---------AA sets end---------\n",
            aaHelper.toString().data());
    //if (params.debug) printCFG(cur_func);

    if (cur_func->empty())
        return;
    // prepare to traverse CFG
    using std::deque, std::set;
    deque<BasicBlock *> workList;
    // we do forward analysis
    // initialize worklist with all blocks
    for (BasicBlock &bb: *cur_func) {
        workList.emplace_back(&bb);
    }
    while (!workList.empty()) {
        // currently analyzed block
        BasicBlock *block = workList.front();
        workList.pop_front();
        // meet out facts into in fact from predecessors
        inFacts[block] = RCFact::Fact();
        for (BasicBlock *pred: predecessors(block)) {
            inFacts[block].factUnion(outFacts[pred]);
        }
        if (!transferNode(block, aaHelper))continue;
        for (BasicBlock *succ: successors(block)) {
            workList.emplace_back(succ);
        }
    }
    // add variables in memref set whose out fact is greater than 0
    endFuncInf.clear();
    for (auto &kv: outFacts) {
        for (auto &pair: kv.second.getCntMap()) {
            if (pair.second != 0) {
                for (auto &v: aaHelper.getMemRefSet(pair.first)) {
                    if (pair.second > 0) {
                        RefcntErrorMsg msg(0, pair.first, cur_func, v, RefcntErrorMsg::MLK);
                        errList.emplace_back(msg);
                    }
                    EndFuncRef efr;
                    efr.ref = pair.second;
                    efr.memref = v;
                    endFuncInf.emplace_back(efr);
                }
            }
        }
    }

    // replace values without name to alias values with name
    std::list<RefcntErrorMsg> namedErrList;
    for (auto &msg: errList) {
        if (msg.var->hasName())namedErrList.emplace_back(msg);
        else {
            auto aliasSet = aaHelper.getNamedAlias(msg.var);
            for (auto alias: aliasSet) {
                RefcntErrorMsg newMsg(msg);
                newMsg.var = alias;
                namedErrList.emplace_back(newMsg);
            }
        }
    }
    errList = namedErrList;
    endFuncTab[cur_func] = endFuncInf;
    errTab[cur_func] = errList;
    bugReport(cur_func);

}

void RefcntPass::CheckBeforeRet() {
}

bool RefcntPass::runOnModule(Module &M) {
    DEBUG_PRINT_STR("\n====================\nContent below is generated by LLVM Refcnt pass\n====================\n");
    init_pass();

    // find entry function
    Function *entry_func = M.getFunction(params.entryFunction);
    outs() << "====================analyses starts=======================\n";
    if (entry_func != nullptr) {
        // Ok. we have entry function now.
        DEBUG_PRINT_FORMAT("<runOnModule> analysis mode: %s\n", params.analysisMode.data());
        DEBUG_PRINT_FORMAT("<runOnModule> entry function: %s\n", entry_func->getName().data());

        // start analysis
        refcntAnalysis(entry_func);
    } else outs() << "No entry function!\n";

    outs() << "=====================analyses ends========================\n";
    // We do not modify the code, so return false.
    return false;
}


char RefcntPass::ID = 0;

void RefcntPass::getAnalysisUsage(AnalysisUsage &AU) const {
    Pass::getAnalysisUsage(AU);
    AU.addRequired<AAResultsWrapperPass>();
    AU.setPreservesAll();
}

void RefcntPass::bugReport(Function *cur_f) {
    // report bugs found in functions recorded in errorLists and save to report.log
    if (errList.empty()) {
        DEBUG_PRINT_FORMAT("<bugReport> no bugs, Function: %s\n", cur_f->getName().data());
        return;
    }
    DEBUG_PRINT_FORMAT("<bugReport> bugs found, Function: %s\n", cur_f->getName().data());
    std::unordered_set<std::string> board;
    for (const auto &msg: errList) {
        board.emplace(msg.toString());
    }
    for (auto &msg_str: board) {
        DEBUG_PRINT_FORMAT("<bugReport> %s\n", msg_str.c_str());
    }
}

void RefcntPass::initAPIInfo() {
    apiInfo["PyType_GenericAlloc"] = {STRONGRET};
    apiInfo["PyType_GenericNew"] = {STRONGRET};
    apiInfo["PyType_GetName"] = {STRONGRET};
    apiInfo["PyType_GetQualName"] = {STRONGRET};
    apiInfo["PyType_FromModuleAndSpec"] = {STRONGRET};
    apiInfo["PyType_FromSpecWithBases"] = {STRONGRET};
    apiInfo["PyType_FromSpec"] = {STRONGRET};
    apiInfo["PyLong_FromLong"] = {STRONGRET};
    apiInfo["PyLong_FromUnsignedLong"] = {STRONGRET};
    apiInfo["PyLong_FromSsize_t"] = {STRONGRET};
    apiInfo["PyLong_FromSize_t"] = {STRONGRET};
    apiInfo["PyLong_FromLongLong"] = {STRONGRET};
    apiInfo["PyLong_FromUnsignedLongLong"] = {STRONGRET};
    apiInfo["PyLong_FromDouble"] = {STRONGRET};
    apiInfo["PyLong_FromString"] = {STRONGRET};
    apiInfo["PyLong_FromUnicodeObject"] = {STRONGRET};
    apiInfo["PyLong_FromVoidPtr"] = {STRONGRET};
    apiInfo["PyBool_FromLong"] = {STRONGRET};
    apiInfo["PyFloat_FromString"] = {STRONGRET};
    apiInfo["PyFloat_FromDouble"] = {STRONGRET};
    apiInfo["PyFloat_GetInfo"] = {STRONGRET};
    apiInfo["PyComplex_FromCComplex"] = {STRONGRET};
    apiInfo["PyComplex_FromDoubles"] = {STRONGRET};
    apiInfo["PyBytes_FromString"] = {STRONGRET};
    apiInfo["PyBytes_FromStringAndSize"] = {STRONGRET};
    apiInfo["PyBytes_FromFormat"] = {STRONGRET};
    apiInfo["PyBytes_FromFormatV"] = {STRONGRET};
    apiInfo["PyBytes_FromObject"] = {STRONGRET};
    apiInfo["PyByteArray_FromObject"] = {STRONGRET};
    apiInfo["PyByteArray_FromStringAndSize"] = {STRONGRET};
    apiInfo["PyByteArray_Concat"] = {STRONGRET};
    apiInfo["PyUnicode_FromFormatV"] = {STRONGRET};
    apiInfo["PyUnicode_FromEncodedObject"] = {STRONGRET};
    apiInfo["PyUnicode_Substring"] = {STRONGRET};
    apiInfo["PyUnicode_FromUnicode"] = {STRONGRET};
    apiInfo["PyUnicode_FromObject"] = {STRONGRET};
    apiInfo["PyUnicode_DecodeLocaleAndSize"] = {STRONGRET};
    apiInfo["PyUnicode_DecodeLocale"] = {STRONGRET};
    apiInfo["PyUnicode_EncodeLocale"] = {STRONGRET};
    apiInfo["PyUnicode_DecodeFSDefaultAndSize"] = {STRONGRET};
    apiInfo["PyUnicode_DecodeFSDefault"] = {STRONGRET};
    apiInfo["PyUnicode_EncodeFSDefault"] = {STRONGRET};
    apiInfo["PyUnicode_FromWideChar"] = {STRONGRET};
    apiInfo["PyUnicode_Decode"] = {STRONGRET};
    apiInfo["PyUnicode_AsEncodedString"] = {STRONGRET};
    apiInfo["PyUnicode_DecodeUTF8"] = {STRONGRET};
    apiInfo["PyUnicode_DecodeUTF8Stateful"] = {STRONGRET};
    apiInfo["PyUnicode_AsUTF8String"] = {STRONGRET};
    apiInfo["PyUnicode_DecodeUTF32"] = {STRONGRET};
    apiInfo["PyUnicode_DecodeUTF32Stateful"] = {STRONGRET};
    apiInfo["PyUnicode_AsUTF32String"] = {STRONGRET};
    apiInfo["PyUnicode_DecodeUTF16"] = {STRONGRET};
    apiInfo["PyUnicode_DecodeUTF16Stateful"] = {STRONGRET};
    apiInfo["PyUnicode_AsUTF16String"] = {STRONGRET};
    apiInfo["PyUnicode_DecodeUTF7"] = {STRONGRET};
    apiInfo["PyUnicode_DecodeUTF7Stateful"] = {STRONGRET};
    apiInfo["PyUnicode_DecodeUnicodeEscape"] = {STRONGRET};
    apiInfo["PyUnicode_AsUnicodeEscapeString"] = {STRONGRET};
    apiInfo["PyUnicode_DecodeRawUnicodeEscape"] = {STRONGRET};
    apiInfo["PyUnicode_AsRawUnicodeEscapeString"] = {STRONGRET};
    apiInfo["PyUnicode_DecodeLatin1"] = {STRONGRET};
    apiInfo["PyUnicode_DecodeASCII"] = {STRONGRET};
    apiInfo["PyUnicode_AsASCIIString"] = {STRONGRET};
    apiInfo["PyUnicode_DecodeCharmap"] = {STRONGRET};
    apiInfo["PyUnicode_AsCharmapString"] = {STRONGRET};
    apiInfo["PyUnicode_Translate"] = {STRONGRET};
    apiInfo["PyUnicode_DecodeMBCS"] = {STRONGRET};
    apiInfo["PyUnicode_DecodeMBCSStateful"] = {STRONGRET};
    apiInfo["PyUnicode_AsMBCSString"] = {STRONGRET};
    apiInfo["PyUnicode_EncodeCodePage"] = {STRONGRET};
    apiInfo["PyUnicode_Concat"] = {STRONGRET};
    apiInfo["PyUnicode_Split"] = {STRONGRET};
    apiInfo["PyUnicode_Splitlines"] = {STRONGRET};
    apiInfo["PyUnicode_Join"] = {STRONGRET};
    apiInfo["PyUnicode_Replace"] = {STRONGRET};
    apiInfo["PyUnicode_RichCompare"] = {STRONGRET};
    apiInfo["PyUnicode_Format"] = {STRONGRET};
    apiInfo["PyUnicode_InternFromString"] = {STRONGRET};
    apiInfo["PyTuple_New"] = {STRONGRET};
    apiInfo["PyTuple_Pack"] = {STRONGRET};
    apiInfo["PyTuple_GetSlice"] = {STRONGRET};
    apiInfo["PyStructSequence_NewType"] = {STRONGRET};
    apiInfo["PyStructSequence_New"] = {STRONGRET};
    apiInfo["PyList_New"] = {STRONGRET};
    apiInfo["PyList_GetSlice"] = {STRONGRET};
    apiInfo["PyList_AsTuple"] = {STRONGRET};
    apiInfo["PyList_Size"] = {IGNORE};
    apiInfo["PyDict_New"] = {STRONGRET};
    apiInfo["PyDictProxy_New"] = {STRONGRET};
    apiInfo["PyDict_Copy"] = {STRONGRET};
    apiInfo["PyDict_Items"] = {STRONGRET};
    apiInfo["PyDict_Keys"] = {STRONGRET};
    apiInfo["PyDict_Values"] = {STRONGRET};
    apiInfo["PySet_New"] = {STRONGRET};
    apiInfo["PyFrozenSet_New"] = {STRONGRET};
    apiInfo["PySet_Pop"] = {STRONGRET};
    apiInfo["PyFunction_New"] = {STRONGRET};
    apiInfo["PyFunction_NewWithQualName"] = {STRONGRET};
    apiInfo["PyInstanceMethod_New"] = {STRONGRET};
    apiInfo["PyMethod_New"] = {STRONGRET};
    apiInfo["PyCell_New"] = {STRONGRET};
    apiInfo["PyCell_Get"] = {STRONGRET};
    apiInfo["PyCell_GET"] = {STRONGRET};
    apiInfo["PyCode_New"] = {STRONGRET};
    apiInfo["PyCode_NewWithPosOnlyArgs"] = {STRONGRET};
    apiInfo["PyCode_NewEmpty"] = {STRONGRET};
    apiInfo["PyCode_GetCode"] = {STRONGRET};
    apiInfo["PyCode_GetVarnames"] = {STRONGRET};
    apiInfo["PyCode_GetCellvars"] = {STRONGRET};
    apiInfo["PyCode_GetFreevars"] = {STRONGRET};
    apiInfo["PyFile_FromFd"] = {STRONGRET};
    apiInfo["PyFile_GetLine"] = {STRONGRET};
    apiInfo["PyModule_NewObject"] = {STRONGRET};
    apiInfo["PyModule_New"] = {STRONGRET};
    apiInfo["PyModule_GetNameObject"] = {STRONGRET};
    apiInfo["PyModule_GetFilenameObject"] = {STRONGRET};
    apiInfo["PyModule_Create"] = {STRONGRET};
    apiInfo["PyModule_Create2"] = {STRONGRET};
    apiInfo["PyModule_FromDefAndSpec"] = {STRONGRET};
    apiInfo["PyModule_FromDefAndSpec2"] = {STRONGRET};
    apiInfo["PySeqIter_New"] = {STRONGRET};
    apiInfo["PyCallIter_New"] = {STRONGRET};
    apiInfo["PyDescr_NewGetSet"] = {STRONGRET};
    apiInfo["PyDescr_NewMember"] = {STRONGRET};
    apiInfo["PyDescr_NewMethod"] = {STRONGRET};
    apiInfo["PyDescr_NewWrapper"] = {STRONGRET};
    apiInfo["PyDescr_NewClassMethod"] = {STRONGRET};
    apiInfo["PyWrapper_New"] = {STRONGRET};
    apiInfo["PySlice_New"] = {STRONGRET};
    apiInfo["PyMemoryView_FromObject"] = {STRONGRET};
    apiInfo["PyMemoryView_FromMemory"] = {STRONGRET};
    apiInfo["PyMemoryView_FromBuffer"] = {STRONGRET};
    apiInfo["PyMemoryView_GetContiguous"] = {STRONGRET};
    apiInfo["PyWeakref_NewRef"] = {STRONGRET};
    apiInfo["PyWeakref_NewProxy"] = {STRONGRET};
    apiInfo["PyCapsule_New"] = {STRONGRET};
    apiInfo["PyGen_New"] = {STRONGRET};
    apiInfo["PyGen_NewWithQualName"] = {STRONGRET};
    apiInfo["PyCoro_New"] = {STRONGRET};
    apiInfo["PyContext_New"] = {STRONGRET};
    apiInfo["PyContext_Copy"] = {STRONGRET};
    apiInfo["PyContext_CopyCurrent"] = {STRONGRET};
    apiInfo["PyContextVar_New"] = {STRONGRET};
    apiInfo["PyContextVar_Set"] = {STRONGRET};
    apiInfo["PyContextVar_Set"] = {STRONGRET};
    apiInfo["PyDate_FromDate"] = {STRONGRET};
    apiInfo["PyDateTime_FromDateAndTime"] = {STRONGRET};
    apiInfo["PyDateTime_FromDateAndTimeAndFold"] = {STRONGRET};
    apiInfo["PyErr_NewException"] = {STRONGRET};
    apiInfo["PyTime_FromTime"] = {STRONGRET};
    apiInfo["PyTime_FromTimeAndFold"] = {STRONGRET};
    apiInfo["PyDelta_FromDSU"] = {STRONGRET};
    apiInfo["PyTimeZone_FromOffset"] = {STRONGRET};
    apiInfo["PyTimeZone_FromOffsetAndName"] = {STRONGRET};
    apiInfo["PyDateTime_FromTimestamp"] = {STRONGRET};
    apiInfo["PyDate_FromTimestamp"] = {STRONGRET};

    apiInfo["PyTuple_GetItem"] = {WEAKRET};
    apiInfo["PyTuple_GET_ITEM"] = {WEAKRET};
    apiInfo["PyStructSequence_GetItem"] = {WEAKRET};
    apiInfo["PyStructSequence_GET_ITEM"] = {WEAKRET};
    apiInfo["PyList_GetItem"] = {WEAKRET};
    apiInfo["PyList_GET_ITEM"] = {WEAKRET};
    apiInfo["PyDict_GetItem"] = {WEAKRET};
    apiInfo["PyDict_GetItemWithError"] = {WEAKRET};
    apiInfo["PyDict_GetItemString"] = {WEAKRET};
    apiInfo["PyDict_SetDefault"] = {WEAKRET};
    apiInfo["PyFunction_GetCode"] = {WEAKRET};
    apiInfo["PyFunction_GetGlobals"] = {WEAKRET};
    apiInfo["PyFunction_GetModule"] = {WEAKRET};
    apiInfo["PyFunction_GetDefaults"] = {WEAKRET};
    apiInfo["PyFunction_GetClosure"] = {WEAKRET};
    apiInfo["PyFunction_GetAnnotations"] = {WEAKRET};
    apiInfo["PyInstanceMethod_Function"] = {WEAKRET};
    apiInfo["PyInstanceMethod_GET_FUNCTION"] = {WEAKRET};
    apiInfo["PyMethod_Function"] = {WEAKRET};
    apiInfo["PyMethod_GET_FUNCTION"] = {WEAKRET};
    apiInfo["PyMethod_Self"] = {WEAKRET};
    apiInfo["PyMethod_GET_SELF"] = {WEAKRET};
    apiInfo["PyModule_GetDict"] = {WEAKRET};
    apiInfo["PyModuleDef_Init"] = {WEAKRET};
    apiInfo["PyState_FindModule"] = {WEAKRET};
    apiInfo["PyWeakref_GetObject"] = {WEAKRET};
    apiInfo["PyWeakref_GET_OBJECT"] = {WEAKRET};

    apiInfo["PyBytes_Concat"] = {PARAMSTEAL, 1, 0};
    apiInfo["PyBytes_ConcatAndDel"] = {PARAMSTEAL, 2, 0, 1};
    apiInfo["PyUnicode_InternInPlace"] = {PARAMSTEAL, 1, 0};
    apiInfo["PyTuple_SetItem"] = {PARAMSTEAL, 1, 2};
    apiInfo["PyTuple_SET_ITEM"] = {PARAMSTEAL, 1, 2};
    apiInfo["PyStructSequence_SetItem"] = {PARAMSTEAL, 1, 2};
    apiInfo["PyStructSequence_SET_ITEM"] = {PARAMSTEAL, 1, 2};
    apiInfo["PyList_SetItem"] = {PARAMSTEAL, 1, 2};
    apiInfo["PyList_SET_ITEM"] = {PARAMSTEAL, 1, 2};
    apiInfo["PyCell_Set"] = {PARAMSTEAL, 1, 1};
    apiInfo["PyModule_AddObject"] = {PARAMSTEAL, 1, 2};


    apiInfo["PyContextVar_Get"] = {PARAMSTRONG, 1, 2};

    apiInfo["PyDict_Next"] = {PARAMBORROW, 2, 3};
}

APIType RefcntPass::getAPIType(std::string name) {
    if (apiInfo.find(name) == apiInfo.end()) {
        DEBUG_PRINT_STR(("<getAPIType> Not supported API: " + name + "\n"));
        return {TYPEUNDEFINE};
    }
    return apiInfo[name];
}

static RegisterPass<RefcntPass> X("refcnt", "refcount Analysis",
                                  false, true);

