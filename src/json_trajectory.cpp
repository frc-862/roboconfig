#include "json/json.h"
#include "traj.h"

Json::Value generate_path(const Json::Value& root) {
  using namespace std;

  config conf;
  conf.dt = root.get("dt", 0.05).asDouble();
  conf.max_acc = root.get("max_acc", 6.0).asDouble();
  conf.max_jerk = root.get("max_jerk", 50.0).asDouble();
  conf.max_vel = root.get("max_vel", 7.0).asDouble();
 
  waypoint_sequence seq;
  for (auto wp : root["waypoints"]) {
    seq.append(waypoint(
          wp.get("x", 0.0).asDouble(),
          wp.get("y", 0.0).asDouble(),
          frc_math::degree2radian(wp.get("theta", 0.0).asDouble())));
  }

  trajectory t = generate_trajectory(conf, seq);
  path p = generate_path(t, 30);
  return p.to_json();
}
