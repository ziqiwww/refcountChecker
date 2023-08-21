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
    // TODO: initialize pass data structures and use json to configure
    Parser::JsonParser parser;
    if (parser.parse("../../settings.json")) {
        DEBUG_PRINT_STR("<init_pass> use settings from settings.json\n");
    } else {
        DEBUG_PRINT_STR("<init_pass> use default settings\n");
    }
    params = parser.anaParams;

    DEBUG_PRINT_STR("<init_pass> initialization done\n");
}

bool RefcntPass::transferNode(BasicBlock *bb, AAHelper &aaHelper) {
    for (const Instruction &inst: *bb) {
        unsigned op = inst.getOpcode();
        switch (op) {
            // TODO: discuss which instructions we care about most
            case Instruction::Call: {
                auto *callInst = dyn_cast<CallInst>(&inst);
                llvm::Function *calledFunction = callInst->getCalledFunction();

                auto name = calledFunction->getName();
                DEBUG_PRINT_FORMAT("<transferNode> call: %s\n", name.data());

                // TODO: discuss how to report bugs found below
                if (name == INCREF_STR || name == XINCREF_STR) {
                    Value *memref = aaHelper.getMemRef(callInst->getArgOperand(0));
                    assert(memref != nullptr);
                    DEBUG_PRINT_FORMAT("<transferNode> memref: %s\n", memref->getName().data());
                    // update outfacts
                    int incnt = inFacts[bb].at(memref);
                    if (incnt == RCFact::Fact::NAC)break;
                    outFacts[bb].update(memref, incnt == RCFact::Fact::UNDEF ? 1 : incnt + 1);
                } else if (name == DECREF_STR || name == XDECREF_STR) {
                    Value *memref = aaHelper.getMemRef(callInst->getArgOperand(0));
                    assert(memref != nullptr);
                    DEBUG_PRINT_FORMAT("<transferNode> memref: %s\n", memref->getName().data());
                    int incnt = inFacts[bb].at(memref);
                    if (incnt == RCFact::Fact::NAC)break;
                    outFacts[bb].update(memref, incnt == RCFact::Fact::UNDEF ? RCFact::Fact::NAC : incnt - 1);
                    // TODO: refcount bug found, decref on a memref with 0 refcnt
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
                    // FIXME: find a way to get return value, this branch[if(name == "PyModule_Create2")] can be uncommented after fixed
//                    if(name == "PyModule_Create2"){
//                        // get return value:
//                        Value *ret = callInst;
//                        Value *memref = aaHelper.getMemRef(ret);
//                        assert(memref != nullptr);
//                        DEBUG_PRINT_FORMAT("<transferNode> memref: %s\n", memref->getName().data());
//                        int incnt = inFacts[bb].at(memref);
//                        if (incnt == RCFact::Fact::NAC)break;
//                        // as PyModule_Create2 returns a strong reference, we increment the refcnt as 1
//                        outFacts[bb].update(memref, 1);
//                    }
                }
                break;
            }

            default:
                break;
        }
    }
    return inFacts[bb] == outFacts[bb];
}

bool RefcntPass::transferEdge(BasicBlock *src, BasicBlock *dst) {
    return false;
}

void RefcntPass::refcntAnalysis(Function *fun_entry) {
    // start from the entry function,
    //
    // TODO: do additional set up before traversing entry
    for (BasicBlock &bb: *fun_entry) {
        inFacts.insert({&bb, RCFact::Fact()});
        outFacts.insert({&bb, RCFact::Fact()});
    }
    if (params.analysesMode == "inter") {
        // start analysis
        interAnalysis(fun_entry);
    } else {
        intraAnalysis(fun_entry);
    }
}

void RefcntPass::intraAnalysis(Function *cur_func) {
    // get alias analysis result
    auto &AA = getAnalysis<AAResultsWrapperPass>(*cur_func).getAAResults();
    AAHelper aaHelper(AA, cur_func);
    DEBUG_PRINT_STR("<intraAnalysis> intra analysis started\n");
    DEBUG_PRINT_FORMAT("<intraAnalysis> function: %s\n", cur_func->getName().data());
    DEBUG_PRINT_FORMAT(
            "<intraAnalysis>---------AA sets begin---------\n%s\n<intraAnalysis>---------AA sets end---------\n",
            aaHelper.AASetVerboseStr().data());

    if (params.debug)
        printCFG(cur_func);

    // mainly focus on
    if (cur_func->empty())
        return;
    // prepare to traverse CFG
    using std::deque, std::set;
    deque<BasicBlock *> workList;
    // we do forward analysis
    workList.emplace_back(&cur_func->getEntryBlock());
    while (!workList.empty()) {
        // currently analyzed block
        BasicBlock *block = workList.front();
        workList.pop_front();
        if (!transferNode(block, aaHelper))continue;
        for (BasicBlock *succ: successors(block)) {
            workList.emplace_back(succ);
        }
    }
    CheckBeforeRet();
}

void RefcntPass::interAnalysis(Function *cur_func) {
    // TODO: do interprocedure analysis for cur func, may call intraAnalysis
}

void RefcntPass::CheckBeforeRet() {
}

bool RefcntPass::runOnModule(Module &M) {

    init_pass();

    // find entry function
    Function *entry_func = M.getFunction(params.entryFunction);
    outs() << "====================analyses starts=======================\n";
    if (entry_func != nullptr) {
        // Ok. we have entry function now.
        DEBUG_PRINT_FORMAT("<runOnModule> analysis mode: %s\n", params.analysesMode.data());
        DEBUG_PRINT_FORMAT("<runOnModule> entry function: %s\n", entry_func->getName().data());

        // start analysis
        refcntAnalysis(entry_func);
    }

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


static RegisterPass<RefcntPass> X("refcnt", "refcount Analysis",
                                  false, true);
