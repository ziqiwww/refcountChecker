//
// Created by ziqi on 2023/8/9.
//

#ifndef REFCNTANALYSIS_AAHELPER_H
#define REFCNTANALYSIS_AAHELPER_H

#include <llvm/Analysis/AliasAnalysis.h>
#include <llvm/Analysis/BasicAliasAnalysis.h>
#include<llvm/Analysis/AliasSetTracker.h>
#include <unordered_map>

class AAHelper {
private:
    llvm::AliasAnalysis &AA;

    llvm::Function *func;

    std::unordered_map<llvm::Value *, std::set<llvm::Value *>> AASet;

public:
    explicit AAHelper(llvm::AliasAnalysis &aa, llvm::Function *func);

    // get representative value(formatted 'AARepresentativeV#no') of v
    llvm::Value *getRepresentative(llvm::Value *v);

    // print AASet
    std::string AASetVerboseStr();
};


#endif //REFCNTANALYSIS_AAHELPER_H
