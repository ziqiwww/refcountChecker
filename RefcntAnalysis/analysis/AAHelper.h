//
// Created by ziqi on 2023/8/9.
//

#ifndef REFCNTANALYSIS_AAHELPER_H
#define REFCNTANALYSIS_AAHELPER_H

#include <llvm/Analysis/AliasAnalysis.h>
#include <llvm/Analysis/BasicAliasAnalysis.h>
#include <llvm/Analysis/ScopedNoAliasAA.h>
#include<llvm/Analysis/AliasSetTracker.h>
#include <unordered_map>

class AAHelper {
private:
    llvm::AliasAnalysis &AA;

    llvm::Function *func;

    std::unordered_map<llvm::Value *, std::set<llvm::Value *>> AASet;

public:
    explicit AAHelper(llvm::AliasAnalysis &aa, llvm::Function *func);

    /// get representative value(formatted 'AAMemRef__No__') of v
    llvm::Value *getMemRef(const llvm::Value *v);

    const std::set<llvm::Value *> &getMemRefSet(const llvm::Value *v);

    void addAAValueMemRef(llvm::Value *v, llvm::Value *memref);

    /// print AASet
    std::string toString();
};


#endif //REFCNTANALYSIS_AAHELPER_H
