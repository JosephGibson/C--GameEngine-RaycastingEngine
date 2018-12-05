#include "Physics.h"
#include "Components.h"

Vec2 Physics::GetOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b)
{
	Vec2 aBB = a->getComponent<CBoundingBox>()->halfSize;
	Vec2 aPos = a->getComponent<CTransform>()->pos;
	Vec2 bBB = b->getComponent<CBoundingBox>()->halfSize;
	Vec2 bPos = b->getComponent<CTransform>()->pos;
	return Vec2((aBB.x + bBB.x) - std::abs(bPos.x - aPos.x), (aBB.y + bBB.y) - std::abs(bPos.y - aPos.y));
}

Vec2 Physics::GetPreviousOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b)
{
	Vec2 aBB = a->getComponent<CBoundingBox>()->halfSize;
	Vec2 aPos = a->getComponent<CTransform>()->prevPos;
	Vec2 bBB = b->getComponent<CBoundingBox>()->halfSize;
	Vec2 bPos = b->getComponent<CTransform>()->pos;
	return Vec2((aBB.x + bBB.x) - std::abs(bPos.x - aPos.x), (aBB.y + bBB.y) - std::abs(bPos.y - aPos.y));
}

bool Physics::LineIntersect(const Vec2 & a, const Vec2 & b, const Vec2 & c, const Vec2 & d)
{
	if (b == c || b == d)
	{
		return false;
	}

	Vec2 r = b - a;
	Vec2 s = d - c;
	float rxs = r * s;
	Vec2 cma = c - a;
	float t = (cma * s) / rxs;
	float u = (cma * r) / rxs;
	return (t >= 0 && t <= 1 && u >= 0 && u <= 1);
}

bool Physics::EntityIntersect(const Vec2 & a, const Vec2 & b, std::shared_ptr<Entity> e)
{

	Vec2 p = e->getComponent<CTransform>()->pos;
	Vec2 bb = e->getComponent<CBoundingBox>()->halfSize;


	if (Physics::LineIntersect(a, b, Vec2(p.x - bb.x, p.y + bb.y), Vec2(p.x + bb.x, p.y + bb.y)))
	{return true;}

	else if (Physics::LineIntersect(a, b, Vec2(p.x + bb.x, p.y + bb.y), Vec2(p.x + bb.x, p.y - bb.y)))
	{return true;}

	else if (Physics::LineIntersect(a, b, Vec2(p.x + bb.x, p.y - bb.y), Vec2(p.x - bb.x, p.y - bb.y)))
	{return true;}

	else if (Physics::LineIntersect(a, b, Vec2(p.x - bb.x, p.y - bb.y), Vec2(p.x - bb.x, p.y + bb.y)))
	{return true;}

	else
	{return false;}

}

bool Physics::LightEntityIntersect(const Vec2 & a, const Vec2 & b, std::shared_ptr<Entity> e)
{

	Vec2 p = e->getComponent<CTransform>()->pos;
	p.y = 768 - p.y;
	Vec2 bb = e->getComponent<CBoundingBox>()->halfSize;


	if (Physics::LineIntersect(a, b, Vec2(p.x - bb.x, p.y + bb.y), Vec2(p.x + bb.x, p.y + bb.y)))
	{return true;}

	else if (Physics::LineIntersect(a, b, Vec2(p.x + bb.x, p.y + bb.y), Vec2(p.x + bb.x, p.y - bb.y)))
	{return true;}

	else if (Physics::LineIntersect(a, b, Vec2(p.x + bb.x, p.y - bb.y), Vec2(p.x - bb.x, p.y - bb.y)))
	{return true;}

	else if (Physics::LineIntersect(a, b, Vec2(p.x - bb.x, p.y - bb.y), Vec2(p.x - bb.x, p.y + bb.y)))
	{return true;}

	else
	{return false;}

}


