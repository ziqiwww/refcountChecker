//
// Created by ziqi on 2023/4/15.
//

#ifndef REFCNTANALYSIS_CFGDRAWER_H
#define REFCNTANALYSIS_CFGDRAWER_H

#include <llvm/IR/CFG.h>
#include "llvm/IR/Function.h"
#include "llvm/Support/GraphWriter.h"
#include "llvm/Support/FileSystem.h"
#include "../common/config.h"


void printCFG(llvm::Function *function) {
    std::string functionName = function->getName().str();
    std::string cfgFilename = functionName + ".dot";
    std::error_code error;
    llvm::raw_fd_ostream outputStream(PROJECT_ROOT + "result/img/cfg/" + cfgFilename, error,
                                      llvm::sys::fs::OpenFlags::OF_Text);
    GraphWriter<Function *> writer(outputStream, function, true);
    writer.writeGraph();
}


#endif //REFCNTANALYSIS_CFGDRAWER_H
