//
// Created by ziqi on 2023/4/16.
//

#include "Fact.h"

using namespace RCFact;

Fact::Fact(const Fact &f) {
    this->cntMap = f.cntMap;
}

bool Fact::update(llvm::Value *v, int c) {
    if (cntMap.find(v) == cntMap.end()) {
        cntMap.insert({v, c});
        return false;
    }
    cntMap[v] += c;
    return true;
}

bool Fact::remove(llvm::Value *v) {
    return cntMap.erase(v) != 0;
}

bool Fact::factUnion(const Fact &f) {
    bool changed = false;
    for (const auto &pair: f.cntMap) {
        if (cntMap.find(pair.first) != cntMap.end()) {
            if (cntMap.at(pair.first) != pair.second || pair.second == NAC) {
                // a conflict occurs
                cntMap[pair.first] = NAC;
                changed = true;
            }
        } else {
            cntMap.insert(pair);
            changed = true;
        }
    }
    return changed;
}

Fact Fact::factUnion(const Fact &f1, const Fact &f2) {
    Fact dst(f1);
    dst.factUnion(f2);
    return dst;
}

int Fact::at(llvm::Value *v) const {
    if (cntMap.find(v) != cntMap.end())
        return cntMap.at(v);
    return UNDEF;
}

Fact &Fact::operator=(const Fact &f) {
    if (this != &f) {
        cntMap = f.cntMap;
    }
    return *this;
}

bool Fact::operator==(const Fact &f) {
    return cntMap == f.cntMap;
}

bool Fact::operator!=(const Fact &f) {
    return !(*this == f);
}

std::string Fact::toString() const {
    std::string str;
    for (const auto &pair: cntMap) {
        str += pair.first->getName().str() + ": " + std::to_string(pair.second) + "\n";
    }
    return str;
}


