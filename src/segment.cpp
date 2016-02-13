#include<iostream>

#include "segment.h"

std::ostream& segment::print(std::ostream& os) const {
  os << "pos: " << pos << "; vel: " << vel << "; acc: " << acc << "; jerk: " << jerk << "; heading: " << heading;
  return os;
}

