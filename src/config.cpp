#include<iostream>
#include <unistd.h>
#include "config.h"

#include<fstream>
#include "tinydir.h"

Config::Config(const std::string& p, const std::string& af) : path(p), activation_file(af) {
  if (path[path.size() - 1] != '/') path += "/";
}

Json::Value Config::get() {
    Json::Value result = Json::arrayValue;
    tinydir_dir dir;
    tinydir_open(&dir, path.c_str());

    for (; dir.has_next; tinydir_next(&dir))
    {
        tinydir_file file;
        tinydir_readfile(&dir, &file);

        if (!file.is_dir)
        {
            std::string name = file.name;
            size_t ext = name.find_last_of('.');
            if (ext != std::string::npos) {
                name.erase(ext);
            }
            result.append(name);
        }
    }
    tinydir_close(&dir);

    return result;
}

Json::Value Config::get(const std::string& name) {
    Json::Value result;
    std::lock_guard<std::mutex> lock(mutex);

    auto value = config_store.find(name);
    if (value == config_store.end()) {
        // read (if it exists) the config file
        std::ifstream stream(path + name + ".json");
        if (stream.good()) {
            stream >> result;
            config_store[name] = result;
        }
    } else {
        // return what we have
        result = value->second;
    }

    return result;
}

void Config::set(const std::string& name, const Json::Value& value) {
    std::lock_guard<std::mutex> lock(mutex);
    config_store[name] = value;
    std::ofstream of(path + name + ".json");
    of << value.toStyledString();
    //Json::StreamWriterBuilder wbuilder;
    //wbuilder["indentation"] = "\t";
    //std::string document = Json::writeString(wbuilder, value);
    //of << document;
}

bool Config::set(const std::string& name, const std::string& json_text) {
    Json::Value value;
    if (reader.parse(json_text, value))
    {
        set(name, value);
        return true;
    }
    return false;
}

bool Config::remove(const std::string &name) {
    std::lock_guard<std::mutex> lock(mutex);
    config_store[name] = Json::nullValue;
    return unlink((path + name + ".json").c_str()) == 0;
}

bool Config::activate(const std::string& name) {
  char buf[1024];

	std::string from = path + name + ".json";
  char* rc = realpath(from.c_str(), buf);
  if (rc == buf) {
    unlink(activation_file.c_str());
    return symlink(buf, activation_file.c_str()) == 0;
  }

  return false;
}

std::string Config::activated() {
  char buf[1024];
  ssize_t rc = readlink(activation_file.c_str(), buf, sizeof(buf));
  buf[sizeof(buf) - 1] = 0;
 
  if (rc == -1) return "";

  std::string result(buf);
  auto pos = result.rfind(".");
  if (pos != std::string::npos)
    result.erase(pos);
  pos = result.rfind("/");
  if (pos != std::string::npos)
    result.erase(0, pos + 1);

  return result;
}

