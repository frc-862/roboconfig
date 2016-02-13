#pragma once

#include<iostream>
#include<cmath>
#include<vector>
#include<string>

#include "waypoint.h"
#include "segment.h"
#include "frc_angle_math.h"

enum SplineType {
  null, CubicHermite, QuinticHermite
};

class spline {
public:
  SplineType type;
  double a;  // ax^5
  double b;  // + bx^4
  double c;  // + cx^3
  double d;  // + dx^2
  double e;  // + ex
  // f is always 0 for the spline formulation we support.

  // The offset from the world frame to the spline frame.
  // Add these to the output of the spline to obtain world coordinates.
  double y_offset;
  double x_offset;
  double knot_distance;
  double theta_offset;
  double arc_length;

  spline() : type(SplineType::null) {
    // All splines should be made via the static interface
    arc_length = -1;
  }

  static bool almostEqual(double x, double y) {
    return std::abs(x - y) < 1E-6;
  }

  static bool reticulateSplines(waypoint start, waypoint goal, spline& result, SplineType type) {
    return reticulateSplines(start.x, start.y, start.theta, goal.x, goal.y,
            goal.theta, result, type);
  }

  static bool reticulateSplines(double x0, double y0, double theta0,
          double x1, double y1, double theta1, spline& result, SplineType type) {
    
    result.type = type;

    // Transform x to the origin
    result.x_offset = x0;
    result.y_offset = y0;
    double x1_hat = std::sqrt((x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0));
    if (x1_hat == 0) {
      throw std::logic_error("Zero length spline is invalid");
    }
    result.knot_distance = x1_hat;
    result.theta_offset = std::atan2(y1 - y0, x1 - x0);
    double theta0_hat = frc_math::get_difference_angle_radians(
            result.theta_offset, theta0);
    double theta1_hat = frc_math::get_difference_angle_radians(
            result.theta_offset, theta1);
    // We cannot handle vertical slopes in our rotated, translated basis.
    // This would mean the user wants to end up 90 degrees off of the straight
    // line between p0 and p1.
    if (almostEqual(std::abs(theta0_hat), M_PI / 2)
            || almostEqual(std::abs(theta1_hat), M_PI / 2)) {
      throw std::logic_error("Vertical slopes are not supported " + std::to_string(theta0_hat) + " or " + std::to_string(theta1_hat));
    }
    // We also cannot handle the case that the end angle is facing towards the
    // start angle (total turn > 90 degrees).
    if (std::abs(frc_math::get_difference_angle_radians(theta0_hat,
            theta1_hat))
            >= M_PI / 2) {
      throw std::logic_error("Total turn must be less than 90 degrees");
    }
    // Turn angles into derivatives (slopes)
    double yp0_hat = std::tan(theta0_hat);
    double yp1_hat = std::tan(theta1_hat);

    if (type == (SplineType) SplineType::CubicHermite) {
      // Calculate the cubic spline coefficients
      result.a = 0;
      result.b = 0;
      result.c = (yp1_hat + yp0_hat) / (x1_hat * x1_hat);
      result.d = -(2 * yp0_hat + yp1_hat) / x1_hat;
      result.e = yp0_hat;
    } else if (type == (SplineType) SplineType::QuinticHermite) {
      result.a = -(3 * (yp0_hat + yp1_hat)) / (x1_hat * x1_hat * x1_hat * x1_hat);
      result.b = (8 * yp0_hat + 7 * yp1_hat) / (x1_hat * x1_hat * x1_hat);
      result.c = -(6 * yp0_hat + 4 * yp1_hat) / (x1_hat * x1_hat);
      result.d = 0;
      result.e = yp0_hat;
    }

    return true;
  }

  double calculateLength() {
    if (arc_length >= 0) {
      return arc_length;
    }

    const int kNumSamples = 100000;
    double arc_length = 0;
    double t, dydt;
    double integrand, last_integrand
            = std::sqrt(1 + derivativeAt(0) * derivativeAt(0)) / kNumSamples;
    for (int i = 1; i <= kNumSamples; ++i) {
      t = ((double) i) / kNumSamples;
      dydt = derivativeAt(t);
      integrand = std::sqrt(1 + dydt * dydt) / kNumSamples;
      arc_length += (integrand + last_integrand) / 2;
      last_integrand = integrand;
    }
    arc_length = knot_distance * arc_length;
    return arc_length;
  }

  double getPercentageForDistance(double distance) {
    const int kNumSamples = 100000;
    double arc_length = 0;
    double t = 0;
    double last_arc_length = 0;
    double dydt;
    double integrand, last_integrand
            = std::sqrt(1 + derivativeAt(0) * derivativeAt(0)) / kNumSamples;
    distance /= knot_distance;
    for (int i = 1; i <= kNumSamples; ++i) {
      t = ((double) i) / kNumSamples;
      dydt = derivativeAt(t);
      integrand = std::sqrt(1 + dydt * dydt) / kNumSamples;
      arc_length += (integrand + last_integrand) / 2;
      if (arc_length > distance) {
        break;
      }
      last_integrand = integrand;
      last_arc_length = arc_length;
    }

    // Interpolate between samples.
    double interpolated = t;
    if (arc_length != last_arc_length) {
      interpolated += ((distance - last_arc_length)
              / (arc_length - last_arc_length) - 1) / (double) kNumSamples;
    }
    return interpolated;
  }

  std::vector<double> getXandY(double percentage) {
    std::vector<double> result(2);

    percentage = std::max<double>(std::min<double>(percentage, 1), 0);
    double x_hat = percentage * knot_distance;
    double y_hat = (a * x_hat + b) * x_hat * x_hat * x_hat * x_hat
            + c * x_hat * x_hat * x_hat + d * x_hat * x_hat + e * x_hat;

    double cos_theta = std::cos(theta_offset);
    double sin_theta = std::sin(theta_offset);

    result[0] = x_hat * cos_theta - y_hat * sin_theta + x_offset;
    result[1] = x_hat * sin_theta + y_hat * cos_theta + y_offset;
    return result;
  }

  double valueAt(double percentage) {
    percentage = std::max<double>(std::min<double>(percentage, 1), 0);
    double x_hat = percentage * knot_distance;
    double y_hat = (a * x_hat + b) * x_hat * x_hat * x_hat * x_hat
            + c * x_hat * x_hat * x_hat + d * x_hat * x_hat + e * x_hat;

    double cos_theta = std::cos(theta_offset);
    double sin_theta = std::sin(theta_offset);

    double value = x_hat * sin_theta + y_hat * cos_theta + y_offset;
    return value;
  }

  double derivativeAt(double percentage) {
    percentage = std::max<double>(std::min<double>(percentage, 1), 0);

    double x_hat = percentage * knot_distance;
    double yp_hat = (5 * a * x_hat + 4 * b) * x_hat * x_hat * x_hat + 3 * c * x_hat * x_hat
            + 2 * d * x_hat + e;

    return yp_hat;
  }

  double secondDerivativeAt(double percentage) {
    percentage = std::max<double>(std::min<double>(percentage, 1), 0);

    double x_hat = percentage * knot_distance;
    double ypp_hat = (20 * a * x_hat + 12 * b) * x_hat * x_hat + 6 * c * x_hat + 2 * d;

    return ypp_hat;
  }

  double angleAt(double percentage) {
    double angle = frc_math::bound_angle_0_to_2pi_radians(
            std::atan(derivativeAt(percentage)) + theta_offset);
    return angle;
  }

  double angleChangeAt(double percentage) {
    return frc_math::bound_angle_neg_pi_to_pi_radians(
            std::atan(secondDerivativeAt(percentage)));
  }

  std::string toString() {
    return std::string("a=") + std::to_string(a) + "; b=" + std::to_string(b) + 
      "; c=" + std::to_string(c) + "; d=" + std::to_string(d) + "; e=" + std::to_string(e);
  }


};
