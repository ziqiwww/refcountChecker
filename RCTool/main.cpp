//
// Created by ziqi on 2023/4/17.
//
#include "Tool.h"

int main(int argc, char **argv) {
    std::string settingsPath = "../../settings.json";
    if (argc == 2)settingsPath = std::string(argv[1]);
    Tool tool(settingsPath);
    return tool.run();
}