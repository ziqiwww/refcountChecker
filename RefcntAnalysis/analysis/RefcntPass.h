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
#include <set>
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
        };
        long pos;
        std::string funcname;
        std::string varname;
        RefcntErrorType errorType;
    };

    struct RefcntPass : public ModulePass {


    private:
        /// tool configuration
        Parser::AnaParam params;

        /// analysis runtime information for intra analysis
        /// values in Fact should be representative value with name 'AAMemRef__No__#'
        std::unordered_map<BasicBlock *, RCFact::Fact> inFacts;
        std::unordered_map<BasicBlock *, RCFact::Fact> outFacts;

        std::list<RecntErrorMsg> errList;

        /**
         * @brief initialize data structures and prepare arguments
         */
        void init_pass();

        /**
         * @brief intraprocedual analysis
         * @param cur_func analysis function
         */
        void intraAnalysis(Function *cur_func);

        /**
         * @brief inter analysis
         * @param cur_func
         */
        void interAnalysis(Function *cur_func);

        /**
         * change refcount information, if out fact changes, return true
         */
        bool transferNode(BasicBlock *bb, AAHelper &aaHelper);

        /**
         * meet the precursors out facts of bb and change the in fact
         * @param bb
         */
        void meetinto(BasicBlock *bb);

        /**
         * may be used for intra analysis, in forward analysis, change dst's in according to src's out
         */
        bool transferEdge(BasicBlock *src, BasicBlock *dst);

        /**
         * @brief do more bug report before leaving a function,
         * feel free to modify and add parameters if needed.
         */
        void CheckBeforeRet();

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

        void refcntAnalysis(Function *fun_entry);

    };
}

#endif // REFCNTPASS_H