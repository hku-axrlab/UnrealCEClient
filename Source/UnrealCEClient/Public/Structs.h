#pragma once

#include "nlohmann/json.hpp"
#include "MemberVariable.h"

struct Transform
{
    float x, y, z;
    float qx, qy, qz, qw; // rotation as quaternion
    float sx, sy, sz;     // scale
};

struct ObjectData
{
    std::string id;
    std::string name;
    std::string home;
    std::string tag;
    Transform transform;
    std::vector<FMemberVariable> variables;
};