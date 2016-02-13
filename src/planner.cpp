#include<algorithm>
#include<cmath>
#include<iostream>
#include "planner.h"
#include "spline.h"

Strategy select_strategy(double start_vel, double goal_vel, double max_vel) {
  if (start_vel == goal_vel && start_vel == max_vel) {
    return Strategy::Step;
  } else if (start_vel == goal_vel && start_vel == 0) {
    return Strategy::SCurves;
  } else {
    return Strategy::Trapezoidal;
  }
}

static trajectory secondOrderFilter(
          int f1_length,
          int f2_length,
          double dt,
          double start_vel,
          double max_vel,
          double total_impulse,
          int length,
          IntegrationMethod integration) {

    if (length <= 0) {
      //throw std::logic_error("Unable to build 0 length trajectory");
      return trajectory();
    }

    trajectory traj(length);

    segment last;
    // First segment is easy
    last.pos = 0;
    last.vel = start_vel;
    last.acc = 0;
    last.jerk = 0;
    last.dt = dt;

    // f2 is the average of the last f2_length samples from f1, so while we
    // can recursively compute f2's sum, we need to keep a buffer for f1.
    std::vector<double> f1(length);
    f1[0] = (start_vel / max_vel) * f1_length;
    double f2;
    for (int i = 0; i < length; ++i) {
      // Apply input
      double input = std::min<double>(total_impulse, 1);
      if (input < 1) {
        // The impulse is over, so decelerate
        input -= 1;
        total_impulse = 0;
      } else {
        total_impulse -= input;
      }

      // Filter through F1
      double f1_last;
      if (i > 0) {
        f1_last = f1[i - 1];
      } else {
        f1_last = f1[0];
      }
      f1[i] = std::max<double>(0.0, std::min<double>(f1_length, f1_last + input));

      f2 = 0;
      // Filter through F2
      for (int j = 0; j < f2_length; ++j) {
        if (i - j < 0) {
          break;
        }

        f2 += f1[i - j];
      }
      f2 = f2 / f1_length;

      // Velocity is the normalized sum of f2 * the max velocity
      traj[i].vel = f2 / f2_length * max_vel;

      if (integration == (IntegrationMethod) IntegrationMethod::RectangularIntegration) {
        traj[i].pos = traj[i].vel * dt + last.pos;
      } else if (integration == (IntegrationMethod) IntegrationMethod::TrapezoidalIntegration) {
        traj[i].pos = (last.vel
                + traj[i].vel) / 2.0 * dt + last.pos;
      }
      traj[i].x = traj[i].pos;
      traj[i].y = 0;

      // Acceleration and jerk are the differences in velocity and
      // acceleration, respectively.
      traj[i].acc = (traj[i].vel - last.vel) / dt;
      traj[i].jerk = (traj[i].acc - last.acc) / dt;
      traj[i].dt = dt;

      last = traj[i];
    }

    return traj;
  }

trajectory generate_trajectory(const config& config,
          Strategy strategy,
          double start_vel,
          double start_heading,
          double goal_pos,
          double goal_vel,
          double goal_heading)
{
  if (strategy == (Strategy) Strategy::Automatic) 
    strategy = select_strategy(start_vel, goal_vel, config.max_vel);

  trajectory traj;

  if (strategy == (Strategy) Strategy::Step) {
    double impulse = (goal_pos / config.max_vel) / config.dt;

    // Round down, meaning we may undershoot by less than max_vel*dt.
    // This is due to discretization and avoids a strange final
    // velocity.
    int time = (int) (std::floor(impulse));
    traj = secondOrderFilter(1, 1, config.dt, config.max_vel,
        config.max_vel, impulse, time, IntegrationMethod::TrapezoidalIntegration);

  } else if (strategy == Strategy::Trapezoidal) {
    // How fast can we go given maximum acceleration and deceleration?
    double start_discount = .5 * start_vel * start_vel / config.max_acc;
    double end_discount = .5 * goal_vel * goal_vel / config.max_acc;

    double adjusted_max_vel = std::min<double>(config.max_vel,
        std::sqrt(config.max_acc * goal_pos - start_discount
          - end_discount));
    double t_rampup = (adjusted_max_vel - start_vel) / config.max_acc;
    double x_rampup = start_vel * t_rampup + .5 * config.max_acc
      * t_rampup * t_rampup;
    double t_rampdown = (adjusted_max_vel - goal_vel) / config.max_acc;
    double x_rampdown = adjusted_max_vel * t_rampdown - .5
      * config.max_acc * t_rampdown * t_rampdown;
    double x_cruise = goal_pos - x_rampdown - x_rampup;

    // The +.5 is to round to nearest
    int time = (int) ((t_rampup + t_rampdown + x_cruise
          / adjusted_max_vel) / config.dt + .5);

    // Compute the length of the linear filters and impulse.
    int f1_length = (int) std::ceil((adjusted_max_vel
          / config.max_acc) / config.dt);
    double impulse = (goal_pos / adjusted_max_vel) / config.dt
      - start_vel / config.max_acc / config.dt
      + start_discount + end_discount;
    traj = secondOrderFilter(f1_length, 1, config.dt, start_vel,
        adjusted_max_vel, impulse, time, IntegrationMethod::TrapezoidalIntegration);

  } else if (strategy == (Strategy) Strategy::SCurves) {
    // How fast can we go given maximum acceleration and deceleration?
    double adjusted_max_vel = std::min<double>(config.max_vel,
        (-config.max_acc * config.max_acc + std::sqrt(config.max_acc
                                                      * config.max_acc * config.max_acc * config.max_acc
                                                      + 4 * config.max_jerk * config.max_jerk * config.max_acc
                                                      * goal_pos)) / (2 * config.max_jerk));

    // Compute the length of the linear filters and impulse.
    int f1_length = (int) std::ceil((adjusted_max_vel
          / config.max_acc) / config.dt);
    int f2_length = (int) std::ceil((config.max_acc
          / config.max_jerk) / config.dt);
    double impulse = (goal_pos / adjusted_max_vel) / config.dt;
    int time = (int) (std::ceil(f1_length + f2_length + impulse));
    traj = secondOrderFilter(f1_length, f2_length, config.dt, 0,
        adjusted_max_vel, impulse, time, IntegrationMethod::TrapezoidalIntegration);

  } else {
    throw std::logic_error("Unable to build trajectory");
  }

  // Now assign headings by interpolating along the path.
  // Don't do any wrapping because we don't know units.
  double total_heading_change = goal_heading - start_heading;
  for (size_t i = 0; i < traj.size(); ++i) {
    traj[i].heading = start_heading + total_heading_change
      * (traj[i].pos)
      / traj[traj.size() - 1].pos;
  }

  return traj;
}

trajectory generate_trajectory(const config& config, const waypoint_sequence& seq) {
    if (seq.get_waypoint_count() < 2) {
      throw std::logic_error("You must have at least two waypoints to generate a trajectory");
    }

    // Compute the total length of the path by creating splines for each pair
    // of waypoints.
    std::vector<spline> splines(seq.size() - 1);
    std::vector<double> spline_lengths(splines.size());
    double total_distance = 0;
    for (size_t i = 0; i < splines.size(); ++i) {
      if (!spline::reticulateSplines(seq[i], seq[i + 1], splines[i], SplineType::QuinticHermite)) {
        throw std::logic_error("Failed to build spline");
      }
      spline_lengths[i] = splines[i].calculateLength();
      total_distance += spline_lengths[i];
    }

    // Generate a smooth trajectory over the total distance.
    trajectory traj = generate_trajectory(config, Strategy::SCurves, 
        0.0, seq[0].theta, total_distance, 0.0, seq[0].theta);

    // Assign headings based on the splines.
    size_t cur_spline = 0;
    double cur_spline_start_pos = 0;
    double length_of_splines_finished = 0;
    for (size_t i = 0; i < traj.size(); ++i) {
      double cur_pos = traj[i].pos;

      bool found_spline = false;
      while (!found_spline) {
        double cur_pos_relative = cur_pos - cur_spline_start_pos;
        if (cur_pos_relative <= spline_lengths[cur_spline]) {
          double percentage = splines[cur_spline].getPercentageForDistance(
                  cur_pos_relative);
          traj[i].heading = splines[cur_spline].angleAt(percentage);
          std::vector<double> coords = splines[cur_spline].getXandY(percentage);
          traj[i].x = coords[0];
          traj[i].y = coords[1];
          found_spline = true;
        } else if (cur_spline < splines.size() - 1) {
          length_of_splines_finished += spline_lengths[cur_spline];
          cur_spline_start_pos = length_of_splines_finished;
          ++cur_spline;
        } else {
          traj[i].heading = splines[splines.size() - 1].angleAt(1.0);
          std::vector<double> coords = splines[splines.size() - 1].getXandY(1.0);
          traj[i].x = coords[0];
          traj[i].y = coords[1];
          found_spline = true;
        }
      }
    }

    return traj;
}

path generate_path(const trajectory& input, double wheelbase_width) {
    trajectory output[2];
    output[0] = input;
    output[1] = input;
    trajectory left = output[0];
    trajectory right = output[1];

    for (size_t i = 0; i < input.size(); ++i) {
      segment current = input[i];
      double cos_angle = std::cos(current.heading);
      double sin_angle = std::sin(current.heading);

      segment s_left = left[i];
      s_left.x = current.x - wheelbase_width / 2 * sin_angle;
      s_left.y = current.y + wheelbase_width / 2 * cos_angle;
      if (i > 0) {
        // Get distance between current and last segment
        double dist = std::sqrt((s_left.x - left[i - 1].x)
                * (s_left.x - left[i - 1].x)
                + (s_left.y - left[i - 1].y)
                * (s_left.y - left[i - 1].y));
        s_left.pos = left[i - 1].pos + dist;
        s_left.vel = dist / s_left.dt;
        s_left.acc = (s_left.vel - left[i - 1].vel) / s_left.dt;
        s_left.jerk = (s_left.acc - left[i - 1].acc) / s_left.dt;
      }

      segment s_right = right[i];
      s_right.x = current.x + wheelbase_width / 2 * sin_angle;
      s_right.y = current.y - wheelbase_width / 2 * cos_angle;
      if (i > 0) {
        // Get distance between current and last segment
        double dist = std::sqrt((s_right.x - right[i - 1].x)
                * (s_right.x - right[i - 1].x)
                + (s_right.y - right[i - 1].y)
                * (s_right.y - right[i - 1].y));
        s_right.pos = right[i - 1].pos + dist;
        s_right.vel = dist / s_right.dt;
        s_right.acc = (s_right.vel - right[i - 1].vel) / s_right.dt;
        s_right.jerk = (s_right.acc - right[i - 1].acc) / s_right.dt;
      }
    }

    return path(output[0], output[1]);
}
