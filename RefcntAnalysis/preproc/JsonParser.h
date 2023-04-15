//
// Created by ziqi on 2023/4/15.
//

#ifndef REFCNTANALYSIS_JSONPARSER_H
#define REFCNTANALYSIS_JSONPARSER_H

#include <string>

namespace Parser {
    struct TooParam{

    };
    struct AnaParam{
        /// basic properties
        std::string analysesMode = "inter";
        std::string entryFunction = "main";
    };
    class JsonParser {
    public:
        /// tool parameters

        /// analyze parameters
        AnaParam anaParams;

        bool parse(const std::string &path);

    };
}


#endif //REFCNTANALYSIS_JSONPARSER_H
