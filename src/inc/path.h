#pragma once

#include <utility>
#include "json/json.h"
#include "trajectory.h"

class path {
public:
  path() {}
  path(const trajectory& l, const trajectory& r) {
    path_pair.first = l;
    path_pair.second = r;
  }

  trajectory& left() { return path_pair.first; }
  trajectory& right() { return path_pair.second; }

  const trajectory& left() const { return path_pair.first; }
  const trajectory& right() const { return path_pair.second; }

  std::ostream& print(std::ostream& os) const;
  Json::Value to_json() const;

private:
  std::pair<trajectory, trajectory> path_pair;

};

inline std::ostream& operator<<(std::ostream& os, const path& p) {
  return p.print(os);
}
