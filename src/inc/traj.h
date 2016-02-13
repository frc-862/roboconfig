#pragma once

#include "json/json.h"
#include "trajectory.h"
#include "waypoint.h"
#include "segment.h"
#include "path.h"
#include "planner.h"
#include "spline.h"

Json::Value generate_path(const Json::Value& root);

