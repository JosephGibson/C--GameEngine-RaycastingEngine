#include "Common.h"
#include "GameState.h"
#include <map>
#include <memory>
#include <deque>

#include "EntityManager.h"

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
	bool							m_drawCollision = true;
	std::vector<int>				m_RoomsX = { -1, 0, 1};
	std::vector<int>				m_RoomsY = { -1, 0, 1};
	std::vector<sf::VertexArray>	m_Light_Lines;
	std::vector<sf::ConvexShape>	m_lightPoly;
	sf::RenderTexture				m_background;


	/** Helper Methods **/
	void init(const std::string & levelPath);
	void loadLevel(const std::string & filename);
	void update();
	void spawnPlayer();


	/** Systems **/
	void sMovement();
	void sAI();
	void sLifespan();
	void sUserInput();
	void sAnimation();
	void sCollision();
	void sRender();
	void sLight();

public:

	GameState_Play(GameEngine & game, const std::string & levelPath);

};