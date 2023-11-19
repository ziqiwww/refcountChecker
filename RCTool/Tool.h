//
// Created by ziqi on 2023/4/17.
//

#ifndef REFCNTANALYSIS_TOOL_H
#define REFCNTANALYSIS_TOOL_H

#include "JsonParser.h"

class Tool {
private:
    ToolParam params;

    std::vector<PluginParam> plugins;

public:
    explicit Tool(const std::string &settingsPath = "settings.json");

    int run();
};


#endif //REFCNTANALYSIS_TOOL_H
