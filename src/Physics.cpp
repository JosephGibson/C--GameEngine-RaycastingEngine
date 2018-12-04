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


bool Physics::EntityIntersect2(const Vec2 & a, const Vec2 & b, std::shared_ptr<Entity> intersect_tile)
{
	Vec2 intersect_orgin = intersect_tile->getComponent<CTransform>()->pos;
	Vec2 intersect_bb = intersect_tile->getComponent<CBoundingBox>()->halfSize;
	intersect_orgin.y = 768 - intersect_orgin.y;


	
	Vec2 v1 = Vec2(intersect_orgin.x - intersect_bb.x, intersect_orgin.y - intersect_bb.y);
	Vec2 v2 = Vec2(intersect_orgin.x + intersect_bb.x, intersect_orgin.y - intersect_bb.y);
	Vec2 v3 = Vec2(intersect_orgin.x + intersect_bb.x, intersect_orgin.y + intersect_bb.y);
	Vec2 v4 = Vec2(intersect_orgin.x - intersect_bb.x, intersect_orgin.y + intersect_bb.y);


	if(Physics::LineIntersect(a, b, v1, v2))
	{
		std::cout << "w1" << std::endl;
		return true;
	}
	else if(Physics::LineIntersect(a, b, v2, v3))
	{
		std::cout << "w2" << std::endl;
		return true;
	}
	else if(Physics::LineIntersect(a, b, v3, v4))
	{
		std::cout << "w3" << std::endl;
		return true;
	}
	else if(Physics::LineIntersect(a, b, v4, v1))
	{
		std::cout << "w4" << std::endl;
		return true;
	}
	

	return false;
}



std::vector<bool> Physics::LightCaster(const Vec2 & cast_origin,  std::shared_ptr<Entity> end_tile, std::shared_ptr<Entity> intersect_tile)
{
	std::vector<bool> points;


	Vec2 end_origin = end_tile->getComponent<CTransform>()->pos;
	end_origin.y = 768 - end_origin.y;

	Vec2 end_bb = end_tile->getComponent<CBoundingBox>()->halfSize;

	/* Get all verts from given tile, assume rectangle*/
	/* The order is TopLeft, TopRight, BottomRigh, BottomLet. (Clockwise) */
	std::vector<Vec2> end_vertices;
	end_vertices.push_back(Vec2(end_origin.x - end_bb.x, end_origin.y - end_bb.y));
	end_vertices.push_back(Vec2(end_origin.x + end_bb.x, end_origin.y - end_bb.y));
	end_vertices.push_back(Vec2(end_origin.x + end_bb.x, end_origin.y + end_bb.y));
	end_vertices.push_back(Vec2(end_origin.x - end_bb.x, end_origin.y + end_bb.y));

	for (Vec2 vert : end_vertices)
	{
		points.push_back(EntityIntersect2(cast_origin, vert, intersect_tile));
		//points.push_back(true);
	}


	return points;
}

