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
        if (aaset.empty()) continue;
        std::string repname = "AAMemRef__No__" + std::to_string(i++);
        llvm::Value *rep = new llvm::GlobalVariable(*func->getParent(), aaset.begin()->getValue()->getType(), false,
                                                    llvm::GlobalValue::ExternalLinkage, nullptr, repname);

        for (auto &v: aaset) {
            AASet[rep].insert(v.getValue());
        }
    }
}

llvm::Value *AAHelper::getMemRef(const llvm::Value *v) {
    for (auto &kv: AASet) {
        if (kv.first->getName().str() == v->getName().str() ||
            kv.second.find(const_cast<llvm::Value *>(v)) != kv.second.end())
            return kv.first;
        else {
            for (auto &v2: kv.second) {
                if (!AA.isNoAlias(v, v2)) {

                    kv.second.insert(const_cast<llvm::Value *>(v));
                    return kv.first;
                }
            }
        }
    }
    return nullptr;
}

std::string AAHelper::toString() {
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

const std::set<llvm::Value *> &AAHelper::getMemRefSet(const llvm::Value *v) {
    return AASet[getMemRef(v)];
}

void AAHelper::addAAValueMemRef(llvm::Value *v, llvm::Value *memref) {
    assert(memref->hasName());
    if (AASet.find(memref) == AASet.end()) {
        AASet[memref] = std::set<llvm::Value *>();
    }
    AASet[memref].insert(v);
}
