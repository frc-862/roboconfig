#include "config.hpp"

#include<fstream>
#include "tinydir.h"

Json::Value Config::get() {
  Json::Value result = Json::arrayValue;
  tinydir_dir dir;
  tinydir_open(&dir, path.c_str());

  for (; dir.has_next; tinydir_next(&dir))
  {
    tinydir_file file;
    tinydir_readfile(&dir, &file);

    if (!file.is_dir)
      result.append(file.name);
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
    std::ifstream stream(path + "/" + name + ".json");
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
  std::ofstream of(path + "/" + name + ".json");
  of << value.toStyledString();
}

void Config::set(const std::string& name, const std::string& json_text) {
  Json::Value value;
  if (reader.parse(json_text, value))
  {
    set(name, value);
  }
}
  
