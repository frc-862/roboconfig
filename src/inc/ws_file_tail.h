//
// Created by Patrick Hurley on 12/6/15.
//

#ifndef ROBOCONFIG_WS_FILE_TAIL_H
#define ROBOCONFIG_WS_FILE_TAIL_H

#include <string>
#include <fstream>
#include <json/json.h>

class WSFileTail {
public:
    WSFileTail(const std::string& fn);

    bool good() { return in.good(); }
    Json::Value lines();

private:
    Json::Reader reader;
    std::string filename;
    std::ifstream in;
    int last_position;
};

#endif //ROBOCONFIG_WS_FILE_TAIL_H
