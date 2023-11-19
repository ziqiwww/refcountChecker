//
// Created by ziqi on 2023/4/17.
//

#ifndef REFCNTANALYSIS_JSONPARSER_H
#define REFCNTANALYSIS_JSONPARSER_H

#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>

struct ToolParam {
    std::string clangPath;
    std::vector<std::string> args;
    std::string modulePath;

    ToolParam();

    bool setClang(const std::string &clang);
};

struct PluginParam {
    std::string pluginName;
    std::string pluginCmd;

    PluginParam();
};

class JsonParser {
public:
    /// tool parameters

    /// analyze parameters
    ToolParam params;

    std::vector<PluginParam> plugins;

    bool parse(const std::string &path);

};


#endif //REFCNTANALYSIS_JSONPARSER_H
