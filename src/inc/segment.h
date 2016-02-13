#pragma once

#include<iosfwd>
#include<string>

struct segment {
   double pos, vel, acc, jerk, heading, dt, x, y;

    segment() {
      pos = vel = acc = jerk = heading = dt = x = y = 0;
    }

    segment(double pos, double vel, double acc, double jerk,
            double heading, double dt, double x, double y) {
      this->pos = pos;
      this->vel = vel;
      this->acc = acc;
      this->jerk = jerk;
      this->heading = heading;
      this->dt = dt;
      this->x = x;
      this->y = y;
    }

    segment invert() const {
      segment result(*this);
      result.y *= -1.0;
      result.heading *= -1.0;

      return result;
    }

    void scale(double factor) {
      pos *= factor;
      vel *= factor;
      acc *= factor;
      jerk *= factor;
    }

    std::ostream& print(std::ostream& os) const;

    explicit operator std::string() const {
      return std::string("pos: ") + std::to_string(pos) + 
        "; vel: " + std::to_string(vel) + 
        "; acc: " + std::to_string(acc) + 
        "; jerk: " + std::to_string(jerk) + 
        "; heading: " + std::to_string(heading);
    }
};

inline std::ostream& operator<<(std::ostream& os, const segment& s) {
  return s.print(os);
}

