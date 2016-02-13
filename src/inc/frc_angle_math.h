#pragma once

namespace frc_math {

  inline double bound_angle_0_to_2pi_radians(double angle) {
    // simple algorithm borrowed (like the rest of the code) from
    // Team 254
    while (angle > 2.0 * M_PI) {
      angle -= 2.0 * M_PI;
    }

    while (angle < 0) {
      angle += 2.0 * M_PI;
    }

    return angle;
  }

  inline double bound_angle_neg_pi_to_pi_radians(double angle) {
    // Naive algorithm
    while (angle >= M_PI) {
      angle -= 2.0 * M_PI;
    }
    while (angle < -M_PI) {
      angle += 2.0 * M_PI;
    }
    return angle;
  }

  inline double get_difference_angle_radians(double from, double to) {
    return bound_angle_neg_pi_to_pi_radians(to - from);
  }

  inline double radian2degree(double radian) {
    return radian * (180 / M_PI);
  }

  inline double degree2radian(double degree) {
    return degree * (M_PI / 180);
  }
};
