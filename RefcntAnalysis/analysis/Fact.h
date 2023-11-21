//
// Created by ziqi on 2023/4/16.
//

#ifndef REFCNTANALYSIS_FACT_H
#define REFCNTANALYSIS_FACT_H

#include <map>
#include<list>
#include <llvm/IR/CFG.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>

namespace RCFact {
    constexpr int MAX_REF_CNT = 1024;

    class Fact {
    private:
        std::unordered_map<llvm::Value *, int> cntMap;
    public:
        enum {
            UNDEF = -1,
            NAC = -2
        };

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

        bool operator==(const Fact &f);

        bool operator!=(const Fact &f);


        std::string toString() const;

        [[nodiscard]] const std::unordered_map<llvm::Value *, int> &getCntMap() const {
            return cntMap;
        }

    };

}


#endif //REFCNTANALYSIS_FACT_H
