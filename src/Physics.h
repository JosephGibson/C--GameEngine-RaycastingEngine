#pragma once

#include "Common.h"
#include "Entity.h"


namespace Physics
{
    Vec2 GetOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b);
    Vec2 GetPreviousOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b);
    bool LineIntersect(const Vec2 & a, const Vec2 & b, const Vec2 & c, const Vec2 & d);
    bool EntityIntersect(const Vec2 & a, const Vec2 & b, std::shared_ptr<Entity> e);
    bool EntityIntersect2(const Vec2 & a, const Vec2 & b, std::shared_ptr<Entity> e);
    std::vector<bool> LightCaster(const Vec2 & cast_origin, std::shared_ptr<Entity> end_tile, std::shared_ptr<Entity> intersect_tile);
}