//
// Created by ziqi on 2023/8/9.
//

#include "AAHelper.h"

AAHelper::AAHelper(llvm::AliasAnalysis &aa, llvm::Function *func) : AA(aa), func(func) {
    llvm::BatchAAResults batchAA(AA);
    llvm::AliasSetTracker tracker(batchAA);
    for (auto &bb: *func) {
        tracker.add(bb);
    }
    int i = 0;
    for (auto &aaset: tracker) {
        std::string repname = "AAMemRef__No__" + std::to_string(i++);
        llvm::Value *rep = new llvm::GlobalVariable(*func->getParent(), aaset.begin()->getValue()->getType(), false,
                                                    llvm::GlobalValue::ExternalLinkage, nullptr, repname);
        for (auto &v: aaset) {
            AASet[rep].insert(v.getValue());
        }
    }
}

llvm::Value *AAHelper::getMemRef(llvm::Value *v) {
    for (auto &kv: AASet) {
        if (kv.second.find(v) != kv.second.end())
            return kv.first;
        else {
            for (auto &v2: kv.second) {
                if (AA.alias(v, v2) != llvm::AliasResult::NoAlias)
                    return kv.first;
            }
        }
    }
    return nullptr;
}

std::string AAHelper::AASetVerboseStr() {
    std::string str;
    for (auto &kv: AASet) {
        str += kv.first->getName().str() + "(size:" + std::to_string(kv.second.size()) + "): \n";
        for (auto &v: kv.second) {
            str += v->getName().str() + ", ";
        }
        str += "\n";
    }
    return str;
}
