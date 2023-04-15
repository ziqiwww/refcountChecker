//
// Created by ziqi on 2023/4/15.
//
#include <fstream>
#include <filesystem>
#include <llvm/Support/raw_ostream.h>
#include "JsonParser.h"
#include "nlohmann/json.hpp"

bool Parser::JsonParser::parse(const std::string &path) {
    using namespace llvm;
    using json = nlohmann::json;
    std::ifstream jfile("./settings.json", std::ios::in);
    if (!jfile.is_open()) {
        outs() << "settings.json is not open, will use default settings\n";
        return true;
    } else {
        try {
            json config = json::parse(jfile);
            /**============================================
             * Tool config
             =============================================*/
            auto tool_conf = config.at("ToolConfig");

            /**============================================
             * Analyze config
             =============================================*/
            auto ana_conf = config.at("AnalyzeConfig");

            /// mode is either inter or intra
            auto mode = ana_conf.at("analysesMode");
            if (mode == "inter" || mode == "intra")
                anaParams.analysesMode = mode;
            else
                outs() << "mode setting is invalid, use interprocedual as default setting\n";

//            /// targetModulePath, useless for now
//            auto mp = ana_conf.at("targetModulePath");
//            if (mp != nullptr) {
//                std::filesystem::path modulePath(mp);
//
//                if (exists(modulePath) && is_regular_file(modulePath)) {
//                    outs() << "analyse module at " << modulePath << '\n';
//                } else {
//                    outs() << modulePath << " is not valid.\n";
//                }
//            }
            /// set entry function in the module, main is the default;
            auto entry = ana_conf.at("entryFunction");
            if (entry != nullptr)
                anaParams.entryFunction = entry;
            /// parse done
            return true;
        } catch (const std::exception &e) {
            errs() << "error occurred when parsing json\n";
            return false;
        }
    }
}
