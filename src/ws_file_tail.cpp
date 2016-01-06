#include <algorithm>
#include <iostream>
#include "ws_file_tail.h"

using namespace std;

WSFileTail::WSFileTail(const std::string &fn) : filename(fn), last_position(0) {
    //in.open(filename);
}

Json::Value WSFileTail::lines() {
    const int max_lines = 1000;

    in.open(filename);
    in.seekg(last_position, ios::beg);

    std::string line;
    Json::Value lines = Json::arrayValue;

    while(std::getline(in, line)) {
      last_position = in.tellg();

        Json::Value jline;
        if (reader.parse(line.c_str(), jline))        
          lines.append(jline);
        else if (line.size() > 0)
          lines.append(line);

        if (lines.size() > max_lines) break;
    }

    // TODO -- check if we can tellg as we read lines and not loose info
    //in.clear();
    //in.seekg(0, ios::end);
    //last_position = (int) in.tellg() - 1; // why - 1? (backup past virtual eof character?)

    in.close();
    if (lines.size() == 0) return Json::nullValue;

    return lines;
}

