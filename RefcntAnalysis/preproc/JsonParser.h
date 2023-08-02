//
// Created by ziqi on 2023/4/15.
//

#ifndef REFCNTANALYSIS_JSONPARSER_H
#define REFCNTANALYSIS_JSONPARSER_H

#include <string>
#include "nlohmann/json.hpp"

namespace Parser {
    struct AnaParam {
        /// basic properties
        std::string analysesMode = "intra";
        std::string entryFunction;
        bool debug = true;
    };

    class JsonParser {

    private:
        template<class T>
        static T getOrDefault(const std::string &key, const T def, const nlohmann::json &j) {
            if (j.contains(key)) {
                return j.at(key);
            } else {
                return def;
            }
        }

    public:

        /// analyze parameters
        AnaParam anaParams;

        bool parse(const std::string &path);

    };
}


#endif //REFCNTANALYSIS_JSONPARSER_H
