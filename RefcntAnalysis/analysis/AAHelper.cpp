//
// Created by ziqi on 2023/8/9.
//

#include "AAHelper.h"

AAHelper::AAHelper(llvm::AliasAnalysis &aa, llvm::Function *func) : AA(aa), func(func) {
    llvm::BatchAAResults batchAA(AA);
    llvm::AliasSetTracker tracker(batchAA);
    for (auto &bb: *func) {
        for (auto &inst: bb) {
            tracker.add(&inst);
        }
    }
    int i = 0;
    for (auto &aaset: tracker) {
        std::string repname = "AARepresentativeV__No__" + std::to_string(i++);
        llvm::Value *rep = new llvm::GlobalVariable(*func->getParent(), aaset.begin()->getValue()->getType(), false,
                                                    llvm::GlobalValue::ExternalLinkage, nullptr, repname);
        for (auto &v: aaset) {
            AASet[rep].insert(v.getValue());
        }
    }
}

llvm::Value *AAHelper::getRepresentative(llvm::Value *v) {
    for (auto &kv: AASet) {
        if (kv.second.find(v) != kv.second.end())
            return kv.first;
    }
    return nullptr;
}

std::string AAHelper::AASetVerboseStr() {
    std::string str;
    for (auto &kv: AASet) {
        str += kv.first->getName().str() + ":\n";
        for (auto &v: kv.second) {
            str += v->getName().str() + ", ";
        }
        str += "\n";
    }
    return str;
}
