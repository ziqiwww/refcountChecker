//
// Created by ziqi on 2023/4/16.
//

#ifndef REFCNTANALYSIS_FACT_H
#define REFCNTANALYSIS_FACT_H

#include <map>
#include <llvm/IR/CFG.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>

#define NAC (-1)
#define UNDEF (-2)

namespace RCFact {

    enum RCError {

        MEM_LEAK = 1, // memory leak error, delta refcount > 0
        DUP_DEC       // duplicate decrease, delta refcount < 0
    };

    class Result {
    private:
        llvm::Function *func;
        llvm::Value *value;
        int delta;

    public:
        explicit Result(llvm::Function *f = nullptr, llvm::Value *v = nullptr, int d = 0) : func(f), value(v),
                                                                                            delta(d) {}

        [[nodiscard]] RCError getError() const { return delta > 0 ? MEM_LEAK : DUP_DEC; }

    };

    class Fact {
    private:
        std::map<llvm::Value *, int> cntMap;
    public:

        Fact() = default;

        Fact(const Fact &f);

        /**
         * update count information in fact map
         * @param v value
         * @param c count delta to update
         * @return true if value exists, false if not.
         */
        bool update(llvm::Value *v, int c);

        /**
         * remove the value from the fact
         */
        bool remove(llvm::Value *v);

        /**
         * union fact, return true if set is changed
         */
        bool factUnion(const Fact &f);

        /**
         * union two fact and return a new one
         */
        static Fact factUnion(const Fact &f1, const Fact &f2);

        /**
         * get refcount of v, return -1 if value miss
         */
        int at(llvm::Value *v) const;

        Fact &operator=(const Fact &f);

    };

}


#endif //REFCNTANALYSIS_FACT_H
