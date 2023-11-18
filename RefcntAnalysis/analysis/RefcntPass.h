//
// Created by ziqi on 06/03/2023
//

#ifndef REFCNTPASS_H
#define REFCNTPASS_H

#include <llvm/IR/CFG.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Use.h>
#include <llvm/IR/Value.h>
#include <llvm/Pass.h>
#include "AAHelper.h"
#include <llvm/Support/raw_ostream.h>
#include <unordered_set>
#include <vector>
#include <map>
#include <list>
#include "Fact.h"
#include "preproc/JsonParser.h"

#define INCREF_STR "_Py_INCREF"
#define XINCREF_STR "_Py_XINCREF"
#define DECREF_STR "_Py_DECREF"
#define XDECREF_STR "_Py_XDECREF"
#define NEWREF_STR "_Py_NewRef"

using namespace llvm;

namespace llvm {
    void initializeRefcntPassPass(PassRegistry &);
}

namespace {


    struct RecntErrorMsg {
        enum RefcntErrorType {
            UAF,    // use after free
            MLK,    // memory leak
            NMC     // not managed correctly for values in outerMemSet
        };
        long pos;
        Value *memref; // name of the pointing memory
        Function *func;
        Value *var;
        RefcntErrorType errorType;

        RecntErrorMsg(long pos, Value *memref, Function *func, Value *var, RefcntErrorType errorType) :
                pos(pos),
                memref(memref),
                func(func),
                var(var),
                errorType(errorType) {}

        [[nodiscard]] std::string toString() const {
            std::string type;
            switch (errorType) {
                case UAF:
                    type = "UAF";   // use after free
                    break;
                case MLK:
                    type = "MLK";   // memory leak
                    break;
                case NMC:
                    type = "NMC";   // not managed correctly for values in outerMemSet
                    break;
            }
            return "Error:" + type + ";Memory:" + memref->getName().str() + ";Function:" + func->getName().str() +
                   ";Line:" + std::to_string(pos) +
                   ";Variable:" + var->getName().str() +
                   ";";
        }
    };

    typedef struct EndFuncRef {
        long ref;
        Value *memref;
    } EndFuncRef;

    enum APITag {
        STRONGRET,
        WEAKRET,
        PARAMSTEAL,
        PARAMBORROW,
        PARAMSTRONG,
        TYPEUNDEFINE
    };

    struct APIType {
        APITag tag;
        ///For "parameter type" API
        int target_param_num;
        int param_index[3];

        APIType() {}

        APIType(APITag t, int sn = 0, int pi1 = -1, int pi2 = -1, int pi3 = -1) : tag(t), target_param_num(sn) {
            param_index[0] = pi1;
            param_index[1] = pi2;
            param_index[2] = pi3;
        }

    };

    struct RefcntPass : public ModulePass {


    private:
        /// tool configuration
        Parser::AnaParam params;

        /// analysis runtime information for intra analysis
        /// values in Fact should be representative value with name 'AAMemRef__No__#'
        std::unordered_map<BasicBlock *, RCFact::Fact> inFacts;
        std::unordered_map<BasicBlock *, RCFact::Fact> outFacts;

        // record memref that the current analysed function should not manage
        std::unordered_set<Value *> outerMemSet;

        std::list<RecntErrorMsg> errList;

        std::unordered_map<Function *, std::list<RecntErrorMsg>> errTab;

        std::unordered_map<std::string, APIType> apiInfo;

        std::unordered_set<Function *> funcVisited;

        std::list<EndFuncRef> endFuncInf;

        std::unordered_map<Function *, std::list<EndFuncRef>> endFuncTab;

        Value *curRetVal;

        /**
         * @brief initialize data structures and prepare arguments
         */
        void init_pass();


        /**
         * change refcount information, if out fact changes, return true
         */
        bool transferNode(BasicBlock *bb, AAHelper &aaHelper);

        /**
         * may be used for intra analysis, in forward analysis, change dst's in according to src's out
         */
        bool transferEdge(BasicBlock *src, BasicBlock *dst);

        /**
         * @brief do more bug report before leaving a function,
         * feel free to modify and add parameters if needed.
         */
        void CheckBeforeRet();

        /**
         * @brief report bugs found in functions recorded in errorLists and save to report.log
         * @param errLists
         */
        void bugReport();

        void initAPIInfo();


        APIType getAPIType(std::string name);

    public:
        /**
         * @brief analysis ID
         */
        static char ID;

        RefcntPass() : ModulePass(ID) {}

        /**
         * @brief llvm will enter the pass through this function, do initialize and
         * @param M
         * @return
         */
        bool runOnModule(Module &M) override;

        void getAnalysisUsage(llvm::AnalysisUsage &AU) const override;

        /**
         * @brief start analysis
         * @param cur_func analysis function
         */
        void refcntAnalysis(Function *cur_func);

    };
}

#endif // REFCNTPASS_H