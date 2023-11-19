//
// Created by ziqi on 2023/4/17.
//

#include "Tool.h"
#include "config.h"
#include <iostream>

Tool::Tool(const std::string &settingsPath) {
    JsonParser parser;
    // FIXME: settings path
    bool err = parser.parse(settingsPath);
    if (!err) {
        std::cerr << "error occurred when parsing json\n";
        exit(1);
    }
    params = parser.params;
    plugins = parser.plugins;
}


int Tool::run() {
    // generate IR
    std::string ircmd(params.clangPath + ' ');
    for (const auto &arg: params.args) {
        ircmd.append(arg + ' ');
    }
    ircmd.append(params.modulePath + ' ');
    std::string ll_path = params.modulePath + ".ll";
    ircmd.append("-o " + ll_path);
    if (system(ircmd.c_str()) != 0)return 1;
    // supported aa: --basic-aa, --scev-aa, --scoped-noalias-aa, --tbaa, --objcarc-aa, --basicaa, --aa-eval
    std::string opt_cmd(params.optPath + ' ');
    opt_cmd.append("-load ../lib/libRefcntAnalysis.so -refcnt --scev-aa -aa-eval -print-all-alias-modref-info ");
    opt_cmd.append(ll_path + ' ');
    opt_cmd.append("-enable-new-pm=0");
    //change /dev/null to print out modified ir:
    // https://stackoverflow.com/questions/29758987/using-llvm-opt-with-built-in-passes
    // plugins
    for (const auto &plugin: plugins) {
        std::string plugin_cmd(plugin.pluginCmd);
        system(plugin_cmd.c_str());
    }
    // analysis begins
    return system(opt_cmd.c_str());
}


