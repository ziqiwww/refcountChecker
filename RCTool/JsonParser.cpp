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
            params.setClang(clang);
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

bool ToolParam::setClang(const std::string &clang) {
    if (!clang.empty()) {
        clangPath = clang;
        return true;
    }
    char buffer[1024];
    std::FILE *pipe = popen("which clang", "r");

    if (pipe == nullptr) {
        std::cerr << "Failed to get default clang path\n" << std::endl;
        return false;
    }

    while (std::fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        // Remove the newline character from the end of the path
        buffer[std::strcspn(buffer, "\n")] = '\0';
        std::cout << "Use default clang path: " << buffer << std::endl;
        clangPath = buffer;
    }
    pclose(pipe);
    return true;
}
