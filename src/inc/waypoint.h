#pragma once
#include <vector>
#include <math.h>

#include "frc_angle_math.h"

class waypoint {
public:
    waypoint(double x, double y, double theta=0) {
      this->x = x;
      this->y = y;
      this->theta = theta;
    }
   
    waypoint invert() const {
      waypoint result(*this);

      result.y *= -1;
      result.theta = frc_math::bound_angle_0_to_2pi_radians(2*M_PI - theta);

      return result;
    }

    double x;
    double y;
    double theta;
};

class waypoint_sequence {
private:
  std::vector<waypoint> waypoints;

public:
  waypoint_sequence() {}

  const waypoint& operator[](int index) const { return waypoints[index]; }
  waypoint& operator[](int index) { return waypoints[index]; }
  
  void append(const waypoint& w) {
    waypoints.push_back(w);
  }

  waypoint_sequence& operator<<(const waypoint& w) {
    append(w);
    return *this;
  }
 
  size_t size() const { return waypoints.size(); }

  int get_waypoint_count() const {
    return waypoints.size();
  }

  const waypoint& operator[](unsigned index) {
    if (index > waypoints.size()) 
      throw std::range_error("waypoint index invalid");

    return waypoints[index];
  }

  waypoint_sequence invertY() {
    waypoint_sequence result;

    for (const waypoint& w : waypoints) {
      result.append(w.invert());
    }

    return result;
  }
};
