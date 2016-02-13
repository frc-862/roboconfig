#pragma once

#include "path.h"

enum Strategy {
  Automatic, Step, Trapezoidal, SCurves,
};

enum IntegrationMethod {
  RectangularIntegration, TrapezoidalIntegration
};

struct config {
  config() { 
    dt = 0.05;
    max_vel = 8;
    max_acc = 60;
    max_jerk = 45;
  }

  double dt;
  double max_vel;
  double max_acc;
  double max_jerk;
};

trajectory generate_trajectory(const config& config,
          Strategy strategy,
          double start_vel,
          double start_heading,
          double goal_pos,
          double goal_vel,
          double goal_heading);

trajectory generate_trajectory(const config& config, const waypoint_sequence& );
path generate_path(const trajectory& t, double wheelbase_width); 

