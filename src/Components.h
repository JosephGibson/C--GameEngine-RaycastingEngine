#pragma once

#include <bitset>
#include <array>
#include "Animation.h"
#include "Assets.h"

class Component;
class Entity;

const size_t MaxComponents = 32;

class Component
{
public:
    virtual ~Component() {}
};

class CTransform : public Component
{
public:
    Vec2 pos        = { 0.0, 0.0 };
    Vec2 prevPos    = { 0.0, 0.0 };
    Vec2 scale      = { 1.0, 1.0 };
    Vec2 speed      = { 0.0, 0.0 };
    Vec2 facing      = { 1.0, 0.0 };
    float angle = 0;

    CTransform(const Vec2 & p = { 0, 0 })
        : pos(p), angle(0) {}
    CTransform(const Vec2 & p, const Vec2 & sp, const Vec2 & sc, float a)
        : pos(p), prevPos(p), speed(sp), scale(sc), angle(a) {}

};

class CLifeSpan : public Component
{
public:
    sf::Clock clock;
    int lifespan = 0;
    
    CLifeSpan(int l) : lifespan(l) {}
};

class CInput : public Component
{
public:
    bool up         = false;
    bool down       = false;
    bool left       = false;
    bool right      = false;
    bool shoot      = false;
    bool canShoot   = true;
    CInput() {}
};

class CBoundingBox : public Component
{
public:
    Vec2 size;
    Vec2 halfSize;
    bool blockMove = false;
    bool blockVision = false;
    CBoundingBox(const Vec2 & s, bool m, bool v)
        : size(s), blockMove(m), blockVision(v), halfSize(s.x / 2, s.y / 2) {}
};

class CAnimation : public Component
{
public:
    Animation animation;
    bool repeat;

    CAnimation(const Animation & animation, bool r)
        : animation(animation), repeat(r) {}
};

class CGravity : public Component
{
public:
    float gravity;
    int knockback;
    CGravity(float g, int kb) : gravity(g), knockback(kb) {}
};

class CState : public Component
{
public:
    std::string state = "";
    size_t frames = 0;
    bool grounded = true;
    CState(const std::string & s) : state(s) {}
};

class CFollowPlayer : public Component
{
public:
    Vec2 home = { 0, 0 };
    float speed = 0;
    CFollowPlayer(Vec2 p, float s)
        : home(p), speed(s) {}
    
};

class CLight : public Component
{
    public:
        float dist = 0;
        CLight(const float d): dist(d) {}
};

class CPatrol : public Component
{
public:
    std::vector<Vec2> positions;
    size_t currentPosition = 0;
    float speed = 0;
    CPatrol(std::vector<Vec2> & pos, float s) : positions(pos), speed(s) {}
};

class CHealth : public Component
{
public:
	int hp;
	CHealth(int health) : hp(health) {}
};

class CDamage : public Component 
{
public:
	int dmg;
	CDamage(int damage) : dmg(damage) {}
};

class CInventory : public Component
{
public:
	int numOfHealthKits = 0;
	int ammo = 0;
	bool meleeSelected = true; // if false, gun is the selected weapon;
	CInventory() {}
};

class CItem : public Component
{
public:
	int amount; // this will dictate how much ammo/medkits you get from pickup;
	bool isAmmo; // this will have to be changed should there be more then just ammo and health kits *** is true if it is ammo
	CItem(int numOf, bool type) : amount(numOf), isAmmo(type) {}
};

class CSteer : public Component
{
public:
	float scale = 0;
	float speed = 0;
	Vec2 home = Vec2 { 0.5, 0.5 };
	Vec2 vel = Vec2 { 0, 0 };
	CSteer(float spd, float scale, Vec2 home) : scale(scale), speed(spd), home(home) {};
};