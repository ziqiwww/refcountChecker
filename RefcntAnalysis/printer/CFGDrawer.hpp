//
// Created by ziqi on 2023/4/15.
//

#ifndef REFCNTANALYSIS_CFGDRAWER_H
#define REFCNTANALYSIS_CFGDRAWER_H

#include <llvm/IR/CFG.h>
#include "llvm/IR/Function.h"
#include "llvm/Support/GraphWriter.h"
#include "llvm/Support/FileSystem.h"

namespace llvm {
    template<>
    struct DOTGraphTraits<Function *> : public DefaultDOTGraphTraits {
        DOTGraphTraits(bool isSimple = false) : DefaultDOTGraphTraits(isSimple) {}

        static std::string getGraphName(Function *function) {
            return "CFG for " + function->getName().str();
        }

        std::string getNodeLabel(BasicBlock *basicBlock, Function *function) {
            std::string label = basicBlock->getName().str() + "\\n";
            llvm::Instruction *firstInstruction = &basicBlock->front();
            llvm::Instruction *lastInstruction = &basicBlock->back();

            label += "1: " + std::to_string(firstInstruction->getOpcode()) + "\\n";
            label += "n: " + std::to_string(lastInstruction->getOpcode());

            return label;
        }
    };
}

void printCFG(llvm::Function *function) {
    std::string functionName = function->getName().str();
    std::string cfgFilename = functionName + ".dot";
    std::error_code error;

    llvm::raw_fd_ostream outputStream("./result/img/cfg/" + cfgFilename, error, llvm::sys::fs::OpenFlags::OF_Text);

    if (!error) {
        llvm::DOTGraphTraits<llvm::Function *> graphTraits;
        llvm::WriteGraph(outputStream, function, false, functionName);
    } else {
        llvm::errs() << "Error: could not open file for writing CFG: " << cfgFilename << '\n';
    }
}


#endif //REFCNTANALYSIS_CFGDRAWER_H
