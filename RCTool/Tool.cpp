//
// Created by ziqi on 2023/4/17.
//

#include "Tool.h"

#define PROJECT_ROOT (std::string("../../"))

bool Tool::setParam() {
    JsonParser parser;
    // FIXME: settings path
    bool err = parser.parse(PROJECT_ROOT + "settings.json");
    if (!err)return false;
    params = parser.params;
    return true;
}

int Tool::run() {
    if (!setParam())return 1;
    // generate IR
    std::string ircmd(params.clangPath + ' ');
    for (const auto &arg: params.args) {
        ircmd.append(arg + ' ');
    }
    ircmd.append(params.modulePath + ' ');
    std::string ll_path = params.modulePath + ".ll";
    ircmd.append("-o " + ll_path);
    int ir_ret = system(ircmd.c_str());
    if (ir_ret != 0)return 1;
    // analyze begin
    std::string opt_cmd("opt -load ../lib/libRefcntAnalysis.so -refcnt ");
    opt_cmd.append(ll_path + ' ');
    opt_cmd.append("-enable-new-pm=0");
    return system(opt_cmd.c_str());
}
