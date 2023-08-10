//
// Created by ziqi on 2023/4/15.
//
#include <fstream>
#include <filesystem>
#include <llvm/Support/raw_ostream.h>
#include "JsonParser.h"


bool Parser::JsonParser::parse(const std::string &path) {
    using namespace llvm;
    using json = nlohmann::json;
    std::ifstream jfile(path.c_str(), std::ios::in);
    if (!jfile.is_open()) {
        outs() << "settings.json is not open, will use default settings\n";
        return true;
    } else {
        try {
            json config = json::parse(jfile);
            /**============================================
             * Analyze config
             =============================================*/
            auto ana_conf = config.at("AnalysisConfig");

            /// mode is either inter or intra
            anaParams.analysesMode = getOrDefault<std::string>("mode", "intra", ana_conf);

            /// set entry function in the module;
            anaParams.entryFunction = getOrDefault<std::string>("entry", "", ana_conf);

            /// set debug mode
            anaParams.debug = getOrDefault<bool>("verbose", false, ana_conf);
            /// parse done
            return true;
        } catch (const std::exception &e) {
            errs() << "error occurred when parsing json\n";
            return false;
        }
    }
}
