#include "Common.h"
#include "GameState.h"
#include <map>
#include <memory>
#include <deque>

#include "EntityManager.h"

struct PlayerConfig 
{ 
    float X, Y, CX, CY, SPEED;
};

class GameState_Play : public GameState
{

protected:

    EntityManager            m_entityManager;
    std::shared_ptr<Entity>  m_player;
    std::string              m_levelPath;
    PlayerConfig             m_playerConfig;
    bool                     m_drawTextures = true;
    bool                     m_drawCollision = false;
    bool                     m_follow = false;
	std::vector<int>		 m_RoomsX = {-1, 0, 1};
	std::vector<int>		 m_RoomsY = {-1, 0, 1};



    sf::Text                    m_saveText;
    void init(const std::string & levelPath);
    void loadLevel(const std::string & filename);
    void update();
    void spawnPlayer();
    void sMovement();
    void sAI();
    void sLifespan();
    void sUserInput();
    void sAnimation();
    void sCollision();
    void sRender();



public:

    GameState_Play(GameEngine & game, const std::string & levelPath);

};