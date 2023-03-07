#include <llvm/IR/CFG.h>
#include <llvm/Support/Debug.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <unistd.h>
#include <deque>
#include "RefcntPass.h"
#include "config.h"

void print_one_entry(Instruction *inst, int idx) {
    errs() << "\t\t\U0001F504 ";
    errs() << "#" << idx;
    errs() << " " << inst->getParent()->getParent()->getName();
    inst->getParent();
    errs() << " : " << *inst << "\n";
}

void RefcntPass::init_pass() {
    // TODO: initialize pass data structures and use json to configure

    using json = nlohmann::json;
    std::ifstream jfile("./settings.json", std::ios::in);
    if (!jfile.is_open()) {
        outs() << "settings.json is not open, will use default settings\n";
    } else {
        try {
            json config = json::parse(jfile);
            auto mode = config.at("analysesMode");
            if (mode == "inter" || mode == "intra")
                AnalysesMode = mode;
            else
                outs() << "mode setting is invalid, will use default setting(interprocedual)\n";
            auto entry = config.at("entryFunction");
            if (entry != nullptr)
                EntryFunction = entry;
        } catch (const std::exception &e) {
            errs() << "error occurred when parsing json\n";
        }
    }

#ifdef DEBUG

    outs() << raw_fd_ostream::YELLOW
           << "initialization done\n"
           << raw_fd_ostream::RESET;
#endif
}

void RefcntPass::refcntAnalysis(Function *fun_entry) {
    // start from the entry function,
    //
    // TODO: do additional set up before traverse entry

    // preparation finished
    if (AnalysesMode == "inter") {
        interAnalysis(fun_entry);
    } else {
        intraAnalysis(fun_entry);
    }
}

void RefcntPass::intraAnalysis(Function *cur_func) {
    // TODO: do interprocedure analysis for cur func
    // mainly focus on
    if (cur_func->size() == 0)
        return;
    // prepare to traverse CFG
    using std::deque, std::set;
    deque<BasicBlock *> workList;
    set<BasicBlock *> visited;
    workList.emplace_back(&cur_func->getEntryBlock());
    while (!workList.empty()) {
        // currently analyzed block
        BasicBlock *block = workList.front();
        workList.pop_front();
        if (visited.find(block) != visited.end()) continue;
        visited.insert(block);

        for (const Instruction &inst : *block) {
            unsigned op = inst.getOpcode();
            switch (op) {
                // TODO: discuss which instructions we care about most
            case Instruction::Call: {
                auto *call = dyn_cast<CallInst>(&inst);
                auto name = call->getName();

                // TODO: discuss how to report bugs found below
                if (name == INCREF_STR) {

                } else if (name == XINCREF_STR) {

                } else if (name == DECREF_STR) {

                } else if (name == XDECREF_STR) {

                } else {
                    // TODO: check if it's a special API, aka in retStrongAPI/retWeakAPI/argBorrowAPI/argStealAPI
                    // suggest using another function to do so which processes the api's behavior
                }
                break;
            }
            default:
                break;
            }
        }
    }
    CheckBeforeRet();
}

void RefcntPass::interAnalysis(Function *cur_func) {
    // TODO: do intraprocedure analysis for cur func, may call intraAnalysis
}

void RefcntPass::CheckBeforeRet() {
}

bool RefcntPass::runOnModule(Module &M) {

    init_pass();

    Function *entry_func = nullptr;
    Function *main_func = nullptr;
    // looping through functions to find the main functions.
    for (auto &f : M) {
        auto f_name = f.getName();
        if (!strcmp(f_name.data(), EntryFunction.c_str())) {
            entry_func = &f;
            break;
        } else if (strcmp(f_name.data(), "main") == 0) {
            main_func = &f;
        }
    }
    if (entry_func == nullptr) {
        if (main_func != nullptr) {
            outs() << "entry function is invalid, use default(main) as entry\n";
            entry_func = main_func;
        } else
            errs() << "no entry or main function is determined\n";
    }
#ifdef DEBUG
    outs() << raw_fd_ostream::YELLOW
           << "directory" << std::filesystem::current_path() << "\n"
           << raw_fd_ostream::RESET;
#endif
    outs() << "====================analyses starts=======================\n";
    if (entry_func != nullptr) {
// Ok. we have main function now.
#ifdef DEBUG
        // FIXME: abstract debug into one function
        outs() << raw_fd_ostream::YELLOW
               << "analyses mode: " << AnalysesMode << ", entry function: " << EntryFunction << "\n"
               << raw_fd_ostream::RESET;
#endif
        refcntAnalysis(entry_func);
    }

    outs() << "=====================analyses ends========================\n";
    // We do not modify the code, so return false.
    return false;
}

char RefcntPass::ID = 0;

static RegisterPass<RefcntPass> X("refcnt", "refcount Analysis",
                                  false, false);
