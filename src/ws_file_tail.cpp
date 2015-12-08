#include <algorithm>
#include <iostream>
#include "ws_file_tail.h"

using namespace std;

WSFileTail::WSFileTail(const std::string &fn) : filename(fn), last_position(0) {
    in.open(filename);
}

Json::Value WSFileTail::lines() {
    if (!in.is_open()) return Json::nullValue;
    in.close();
    in.open(filename);
    in.seekg(last_position, ios::beg);

    std::string line;
    Json::Value lines = Json::arrayValue;

    while(std::getline(in, line))
        lines.append(line);

    in.clear();
    in.seekg(0, ios::end);
    last_position = (int) in.tellg() - 1; // why - 1? (backup past virtual eof character?)

    if (lines.size() == 0) return Json::nullValue;

    return lines;
}