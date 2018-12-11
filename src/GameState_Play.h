#include "Common.h"
#include "GameState.h"
#include <map>
#include <memory>
#include <deque>

#include "EntityManager.h"

#define _USE_MATH_DEFINES // fixs visual studio issues on windows
#include <cmath>

struct PlayerConfig
{
	float X, Y, CX, CY, SPEED, Gravity, JumpSpeed;
};

class GameState_Play : public GameState
{

protected:

	EntityManager					m_entityManager;
	std::shared_ptr<Entity> 		m_player;
	std::string						m_levelPath;
	PlayerConfig					m_playerConfig;
	bool							m_canShoot = true;
	sf::Clock 						m_shoot_timer;
	int								m_shoot_Max = 200;
	sf::RenderTexture				m_background;
	sf::VertexArray					m_lightPoly;
	sf::Sound						m_pickUpSound;
	sf::Sound						m_hitSound;
	sf::Sound						m_hurtSound;
	sf::Sound						m_deathSound;
	sf::Sound						m_playerAttackSound;
	sf::Sound						m_backgroundMusic;
	sf::Sound						m_playerSound;
	sf::Text						m_ammoCount, m_hpKitCount, m_weaponSelected , m_currentHP;

	/** Helper Methods **/
	void init(const std::string & levelPath);
	void loadLevel(const std::string & filename);
	void update();
	void spawnPlayer();
	void useHealthKit();
	void spawnBullet(std::shared_ptr<Entity> entity);
	void spawnMeele(std::shared_ptr<Entity> entity);

	/** Systems **/
	void sMovement();
	void sAI();
	void sLifespan();
	void sUserInput();
	void sAnimation();
	void sCollision();
	void sRender();
	void sLight();
	void sHealth();



public:

	GameState_Play(GameEngine & game, const std::string & levelPath);

};