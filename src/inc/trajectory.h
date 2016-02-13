#pragma once

#include <vector>
#include <exception>
#include <iosfwd>
#include <iterator>
#include <string>
#include "waypoint.h"
#include "segment.h"

class trajectory {
private:
  std::vector<segment> segments;
  bool inverted_y;
  double dt;

public:
  trajectory(int length=0) : segments(length), inverted_y(false), dt(0.05) { }
  trajectory(const std::vector<segment>& s) : segments(s), inverted_y(false), dt(0.05) { }
 
  double delta_time() const { return dt; }
  void set_delta_time(double _dt) { dt = _dt; }
  void set_invertedY(bool inverted) { inverted_y = inverted; }
  size_t size() const { return segments.size(); }

  auto begin() const { return segments.begin(); }
  auto begin() { return segments.begin(); }
  auto end() const { return segments.end(); }
  auto end() { return segments.end(); }

  segment operator[](unsigned index) const {
    if (index < segments.size()) {
      if (inverted_y) {
        return segments[index].invert();
      } else {
        return segments[index];
      }
    } else {
      return segment();
    }
  }

  segment& operator[](unsigned index) {
    if (index >= segments.size())
      throw std::range_error("segment index invalid");

    if (inverted_y)
      throw std::logic_error("you cannot get an inverted segment reference");

    return segments[index];
  }

  void scale(double factor) {
    for(segment& s : segments) {
      s.scale(factor);
    }
  }

  void append(const trajectory& traj) {
      segments.insert(std::end(segments), std::begin(traj.segments), std::end(traj.segments));
  }

  const std::vector<segment> get_segments() const { return segments; }

  std::ostream& print(std::ostream& os) const;
  std::ostream& printEuclidean(std::ostream& os) const;
};

inline std::ostream& operator<<(std::ostream& os, const trajectory& t) {
  return t.print(os);
}


