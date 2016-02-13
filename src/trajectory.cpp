#include <iostream>
#include "trajectory.h"
#include "path.h"

Json::Value path::to_json() const {
  Json::Value result = Json::nullValue;

  double time = 0;
  for (size_t i = 0; i < path_pair.first.size(); ++i) {
    const segment& sl = path_pair.first[i];
    const segment& sr = path_pair.second[i];

    Json::Value element;
    element["x"] = (sl.x + sr.x) / 2.0;
    element["y"] = (sl.y + sr.y) / 2.0;
    element["left"] = sl.pos;
    element["lvel"] = sl.vel;
    element["lacc"] = sl.acc;
    element["ljerk"] = sl.jerk;
    element["heading"] = frc_math::radian2degree(frc_math::bound_angle_neg_pi_to_pi_radians(sl.heading));
    element["dt"] = time;
    element["right"] = sr.pos;
    element["rvel"] = sr.vel;
    element["racc"] = sr.acc;
    element["rjerk"] = sr.jerk;
    result.append(element);
    
    time += path_pair.first.delta_time();
  }

  return result;
}

std::ostream& path::print(std::ostream& os) const {
   os << "Segment\tLeft Pos\tLeft Vel\tLeft Acc\tLeft Jerk\t" << 
     "Right Pos\tRight Vel\tRight Acc\tRight Jerk\tHeading\n";

   for (size_t i = 0; i < path_pair.first.size(); ++i) {
     const segment& sl = path_pair.first[i];
     const segment& sr = path_pair.second[i];

     os << i << "\t" <<
       sl.pos << "\t" <<
       sl.vel << "\t" << 
       sl.acc << "\t" <<
       sl.jerk << "\t" << 
       sr.pos << "\t" <<
       sr.vel << "\t" << 
       sr.acc << "\t" <<
       sr.jerk << "\t" << 
       sr.heading << "\n";
   }

   return os;
}

std::ostream& trajectory::print(std::ostream& os) const {
   os << "Segment\tPos\tVel\tAcc\tJerk\tHeading\n";

   for (size_t i = 0; i < segments.size(); ++i) {
     const segment& s = segments[i];

     os << i << "\t" <<
       s.pos << "\t" <<
       s.vel << "\t" << 
       s.acc << "\t" <<
       s.jerk << "\t" << 
       s.heading << "\n";
   }

   return os;
}

  std::ostream& trajectory::printEuclidean(std::ostream& os) const {
     os << "Segment\tx\ty\tHeading\n";

     for (size_t i = 0; i < segments.size(); ++i) {
       const segment& s = segments[i];

       os << i << "\t" <<
         s.x << "\t" <<
         s.y << "\t" << 
         s.heading << "\n";
     }

     return os;
  }
