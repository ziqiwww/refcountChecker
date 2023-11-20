//
// Created by ziqi on 2023/4/17.
//
#include <fstream>
#include <iostream>
#include "JsonParser.h"
#include "../externals/nlohmann/json.hpp"

bool JsonParser::parse(const std::string &path) {
    using json = nlohmann::json;
    std::ifstream jfile(path, std::ios::in);
    if (!jfile.is_open()) {
        std::cout << path << " doesn't exist, please make one using the template\n";
        return false;
    } else {
        try {
            json config = json::parse(jfile);
            /**============================================
             * Tool config
             =============================================*/
            auto tool_conf = config.at("ToolConfig");
            std::string clang = tool_conf.at("clang");
            params.setOrDefault(params.clangPath, clang);
            std::string opt = tool_conf.at("opt");
            params.setOrDefault(params.optPath, opt);
            auto module = tool_conf.at("module");
            if (module == nullptr) {
                std::cerr << "module path can not be empty, add \"module\" entry to settings\n";
                return false;
            } else {
                params.modulePath = module;
            }
            auto args = tool_conf.at("args");
            for (auto &arg: args) {
                params.args.emplace_back(arg);
            }

            std::string AAstrategy = config.at("AnalysisConfig").at("AAstrategy");
            params.AAstrategy = ToolParam::aaTypeToCmdArg(AAstrategy);

            /**============================================
             * Plugin config
             =============================================*/
            auto plugin_conf = config.at("Plugins");
            for (auto &plugin: plugin_conf) {
                PluginParam param;
                param.pluginName = plugin.at("name");
                param.pluginCmd = plugin.at("cmd");
                plugins.emplace_back(param);
            }
            /// parse done
            return true;
        } catch (const std::exception &e) {
            std::cerr << "error occurred when parsing json\n";
            return false;
        }
    }
}

ToolParam::ToolParam() {
    // -emit-llvm -S -fno-discard-value-names
    args.emplace_back("-emit-llvm");                // emit llvm ir
    args.emplace_back("-S");                        // generate .ll file
    args.emplace_back("-fno-discard-value-names");  // keep value names from the source file
}

bool ToolParam::setOrDefault(std::string &target, const std::string &src) {
    if (!src.empty()) {
        target = src;
        return true;
    }
    std::string query = "which " + src;
    char buffer[1024];
    std::FILE *pipe = popen(query.c_str(), "r");

    if (pipe == nullptr) {
        std::cerr << "Failed to get default setting path: " << src << std::endl;
        return false;
    }

    while (std::fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        // Remove the newline character from the end of the path
        buffer[std::strcspn(buffer, "\n")] = '\0';
        std::cout << "Use default setting path: " << buffer << std::endl;
        target = buffer;
    }
    pclose(pipe);
    return true;
}

std::string ToolParam::aaTypeToCmdArg(const std::string &type) {
    if (type == "scev") {
        return "--scev-aa";
    } else if (type == "scoped-noalias") {
        return "--scoped-noalias-aa";
    } else if (type == "tbaa") {
        return "--tbaa";
    } else if (type == "objcarc") {
        return "--objcarc-aa";
    } else if (type == "basicaa") {
        return "--basic-aa";
    } else {
        std::cerr << "unsupported AA type: " << type << ", use basic aa as default" << std::endl;
        return "--basic-aa";
    }
}
