#include "GameState_Play.h"
#include "Common.h"
#include "Physics.h"
#include "Assets.h"
#include "GameEngine.h"
#include "Components.h"
#include <math.h> // Needed for G++ compiler, not required on visual studio.

GameState_Play::GameState_Play(GameEngine & game, const std::string & levelPath)
	: GameState(game)
	, m_levelPath(levelPath)
{
	init(m_levelPath);
}

/**
 * @brief      Intializes the game state
 *
 * @param[in]  levelPath  The level path
 */
void GameState_Play::init(const std::string & levelPath)
{

	loadLevel(levelPath);
}


/**
 * @brief      Loads a level.
 *
 * @param[in]  filename  The filename
 */
void GameState_Play::loadLevel(const std::string & filename)
{
	m_entityManager = EntityManager();

	std::string token;
	std::ifstream file(filename);

	/*The function to handle reading in files.*/
	while (file.good())
	{
		/*Set the line key => Token*/

		file >> token;

		/*Player found, load.*/
		if (token == "Player")
		{
			float x, y;
			file >> x >> y >> m_playerConfig.CX >> m_playerConfig.CY >> m_playerConfig.SPEED >> m_playerConfig.Gravity >> m_playerConfig.JumpSpeed;
			m_playerConfig.X = x * 64 + 32;
			m_playerConfig.Y = y * 64 + 32;
		}

		/*Tile found, load.*/
		else if (token == "Tile")
		{
			std::string name;
			float TX, TY;
			int BM, BV;
			file >> name >> TX >> TY >> BM >> BV;
			auto e = m_entityManager.addEntity(token);
			e->addComponent<CTransform>()->pos = Vec2(TX * 64 + 32, TY * 64 + 32);
			e->addComponent<CAnimation>(m_game.getAssets().getAnimation(name), true);
			e->addComponent<CBoundingBox>(e->getComponent<CAnimation>()->animation.getSize(), BM, BV);
		}

		/*NPC found, load.*/
		else if (token == "NPC")
		{

			std::string name, aiName;			int BM, BV;
			float TY, TX, grav, speed;
			file >> name >> TX >> TY >> BM >> BV >> grav >> aiName;
			auto e = m_entityManager.addEntity(token);
			e->addComponent<CAnimation>(m_game.getAssets().getAnimation(name), true);
			e->addComponent<CTransform>()->pos = Vec2((TX * 64) + e->getComponent<CAnimation>()->animation.getSize().x / 2, TY * 64 + e->getComponent<CAnimation>()->animation.getSize().y / 2);
			e->addComponent<CBoundingBox>(e->getComponent<CAnimation>()->animation.getSize(), BM, BV);
			e->addComponent<CState>("STATE_NAME_HERE");
			e->addComponent<CGravity>(grav);

			if ( aiName == "Follow")
			{
				file >> speed;
				e->addComponent<CFollowPlayer>(Vec2(0, 0), speed);
				e->getComponent<CFollowPlayer>()->home = Vec2((TX * 64) + e->getComponent<CAnimation>()->animation.getSize().x / 2, TY * 64 + e->getComponent<CAnimation>()->animation.getSize().y / 2);
			}

			else if ( aiName == "Patrol")
			{
				int points, x, y;
				file >> speed >> points;
				std::vector<Vec2> pos;
				int i = 0;
				while (i < points)
				{
					file >> x >> y;
					pos.push_back(Vec2((TX * 64) + e->getComponent<CAnimation>()->animation.getSize().x / 2, TY * 64 + e->getComponent<CAnimation>()->animation.getSize().y / 2));
					i++;
				}
				e->addComponent<CPatrol>(pos, speed);
			}
		}
	}

	spawnPlayer();

}

/**
 * @brief      { A function to spawn in a player based of config }
 */
void GameState_Play::spawnPlayer()
{
	m_player = m_entityManager.addEntity("player");
	m_player->addComponent<CTransform>(Vec2(m_playerConfig.X, m_playerConfig.Y));
	m_player->getComponent<CTransform>()->facing = Vec2(0, 1);
	m_player->addComponent<CAnimation>(m_game.getAssets().getAnimation("player_stand"), true);
	m_player->addComponent<CInput>();
	m_player->addComponent<CBoundingBox>(Vec2(m_playerConfig.CX, m_playerConfig.CY), true, true);;
	m_player->addComponent<CGravity>(m_playerConfig.Gravity);
	m_player->addComponent<CState>("stand");
}


/**
 * @brief      { The main loop of the engine }
 */
void GameState_Play::update()
{
	m_entityManager.update();
	if (!m_paused)
	{
		sAI();
		sMovement();
		sLifespan();
		sCollision();
		sAnimation();
		sUserInput();
		sRender();
	}
	else
	{
		sAnimation();
		sUserInput();
		sRender();
	}
}


/**
 * @brief      { The Sytem to handle movement and state switching. }
 */

void GameState_Play::sMovement()
{

	/* Do NPC speed asignments for current cycle */
	for (auto &  npc : m_entityManager.getEntities("NPC"))
	{
		//	std::cout << npc->getComponent<CTransform>()->pos.x << " " << npc->getComponent<CTransform>()->pos.y << std::endl;
		if (!npc->getComponent<CState>()->grounded)
		{
			npc->getComponent<CTransform>()->speed.y -= npc->getComponent<CGravity>()->gravity;
		}
		else
		{
			npc->getComponent<CTransform>()->speed.y = 0;
		}
		npc->getComponent<CTransform>()->prevPos = npc->getComponent<CTransform>()->pos;
		npc->getComponent<CTransform>()->pos += npc->getComponent<CTransform>()->speed;
	}

	/* Save players speed for easy access. */
	Vec2 speed = m_player->getComponent<CTransform>()->speed;
	auto pInput = m_player->getComponent<CInput>();
	std::string state = "stand";


	if (pInput->right)
	{
		speed.x = 3.5;
		m_player->getComponent<CTransform>()->scale.x = 1;
		state = "run";
	}
	else if (pInput->left)
	{
		speed.x = -3.5;
		m_player->getComponent<CTransform>()->scale.x = -1;
		state = "run";
	}
	else
	{
		speed.x = 0;
	}


	if (!m_player->getComponent<CState>()->grounded)
	{
		speed.y -= m_player->getComponent<CGravity>()->gravity;
		state = "jump";
	}
	else if (pInput->up)
	{
		speed.y = m_playerConfig.JumpSpeed;
		state = "jump";
	}
	else
	{
		speed.y = 0;
	}

	/* Set m_players PrevPos = Pos, Set Pos += Speed */
	m_player->getComponent<CTransform>()->speed = speed;
	m_player->getComponent<CTransform>()->prevPos = m_player->getComponent<CTransform>()->pos;
	m_player->getComponent<CTransform>()->pos += m_player->getComponent<CTransform>()->speed;
	m_player->getComponent<CState>()-> state = state;
}


/**
 * @brief      { The AI system. }
 */

void GameState_Play::sAI()
{
	/* Check every NPC for AI udates. */
	for (auto & npc : m_entityManager.getEntities("NPC"))
	{
		if (npc->hasComponent<CFollowPlayer>())
		{
			/* Use bool can see to keep track if any block breaks LOS */
			bool can_see = true;

			/* Check for tile line intersetions. */
			for (auto & tile : m_entityManager.getEntities("Tile"))
			{
				/* Check tile for appropriate components */
				if (tile->hasComponent<CBoundingBox>() && tile->getComponent<CBoundingBox>()->blockVision)
				{
					if (Physics::EntityIntersect(npc->getComponent<CTransform>()->pos, m_player->getComponent<CTransform>()->pos, tile))
					{
						can_see = false;
					}
				}
			}
			/* If vision, chase the player. */
			if (can_see)
			{
				Vec2 Norm = (m_player->getComponent<CTransform>()->pos - npc->getComponent<CTransform>()->pos).norm();
				Norm *= npc->getComponent<CFollowPlayer>()->speed;
				if (npc->getComponent<CState>()->grounded || npc->getComponent<CGravity>()->gravity == 0)
				{
					npc->getComponent<CTransform>()->speed = Norm;
				}
				else
				{
					npc->getComponent<CTransform>()->speed.x = Norm.x;
				}
				if (Norm.x < 0)
				{
					npc->getComponent<CTransform>()->scale.x = 1;
				}
				else
				{
					npc->getComponent<CTransform>()->scale.x = -1;
				}
			}
			/* Check if the NPCS is close that a movement would push them too far. */
			else if (npc->getComponent<CTransform>()->pos.dist(npc->getComponent<CFollowPlayer>()->home) >= npc->getComponent<CFollowPlayer>()->speed * 1.15)
			{
				Vec2 Norm = (npc->getComponent<CFollowPlayer>()->home - npc->getComponent<CTransform>()->pos).norm();
				Norm *= npc->getComponent<CFollowPlayer>()->speed;
				if (npc->getComponent<CState>()->grounded || npc->getComponent<CGravity>()->gravity == 0)
				{
					npc->getComponent<CTransform>()->speed = Norm;
				}
				else
				{
					npc->getComponent<CTransform>()->speed.x = Norm.x;
				}
			}
			/* Else, the player is now home. */
			else
			{
				npc->getComponent<CTransform>()->pos = npc->getComponent<CFollowPlayer>()->home;
			}

		}

		/*Check for patrol updates */
		else if (npc->hasComponent<CPatrol>())
		{
			size_t nextPosNum = 0;
			size_t posNum = npc->getComponent<CPatrol>()->currentPosition;

			if (posNum + 1 < npc->getComponent<CPatrol>()->positions.size())
			{
				nextPosNum = posNum + 1;
			}
			else
			{
				nextPosNum = 0;
			}

			/* Get the next postion and the current postion. */
			Vec2 currentPos = npc->getComponent<CPatrol>()->positions[posNum];
			Vec2 nextPos = npc->getComponent<CPatrol>()->positions[nextPosNum];

			if (npc->getComponent<CTransform>()->pos.dist(nextPos) >= 5)
			{
				Vec2 Norm = (nextPos - currentPos).norm();
				Norm *= npc->getComponent<CPatrol>()->speed;
				if (npc->getComponent<CState>()->grounded || npc->getComponent<CGravity>()->gravity == 0)
				{
					npc->getComponent<CTransform>()->speed = Norm;
				}
				else
				{
					npc->getComponent<CTransform>()->speed.x = Norm.x;
				}
			}
			else
			{
				npc->getComponent<CPatrol>()->currentPosition = nextPosNum;
			}

		}
	}
}


/**
 * @brief      { The Life Span Sytem, handles destroying entities. }
 */

void GameState_Play::sLifespan()
{
	/* Check all life span clocks to see if ElapsedTime >= Life, Del the entity if so.*/
	for (auto e : m_entityManager.getEntities())
	{
		if (e->hasComponent<CLifeSpan>() && e->getComponent<CLifeSpan>()->clock.getElapsedTime().asMilliseconds() >= e->getComponent<CLifeSpan>()->lifespan)
		{
			e->destroy();
		}
	}

}


/**
 * @brief      { The collision system. }
 */

void GameState_Play::sCollision()
{

	bool player_grounded = false;

	/* Check every tile for cols with player.*/
	for (auto & tile : m_entityManager.getEntities("Tile"))
	{
		if (tile->hasComponent<CBoundingBox>() && tile->getComponent<CBoundingBox>()->blockMove)
		{

			/*Player tile collision control block: */
			Vec2 overLap = Physics::GetOverlap(m_player, tile);

			if (overLap.x >= 0 && overLap.y >= 0)
			{
				/* The previous overlap to resolve col.*/
				Vec2 prevOverLap =  Physics::GetPreviousOverlap(m_player, tile);

				/* Check for X collison, resolve first.*/
				if (prevOverLap.y > 0)
				{
					if (m_player->getComponent<CTransform>()->prevPos.x < tile->getComponent<CTransform>()->pos.x)
					{
						m_player->getComponent<CTransform>()->pos.x -= overLap.x;
					}
					else
					{
						m_player->getComponent<CTransform>()->pos.x += overLap.x;
					}
				}

				/* Check for Y Collisions */
				else if (prevOverLap.x > 0)
				{
					/* Check bottom of the tile for Col: */
					if (m_player->getComponent<CTransform>()->prevPos.y < tile->getComponent<CTransform>()->pos.y)
					{
						m_player->getComponent<CTransform>()->pos.y -= overLap.y;
					}

					/* Check top for Col. Accepts: prevOverLap.x = 0 */
					else
					{
						m_player->getComponent<CTransform>()->pos.y += overLap.y;
						player_grounded = true;
					}
				}

			}

			/* Do NPC tile col, this could be wrapped in anoter function since its the same as player col.*/
		}
	}


	/** Check for NPC cols**/
	for (auto & npc : m_entityManager.getEntities("NPC"))
	{
		bool NPC_Grounded = false;
		for (auto & tile : m_entityManager.getEntities("Tile"))
		{
			Vec2 overLap = Physics::GetOverlap(npc, tile);
			if (overLap.x >= 0 && overLap.y >= 0)
			{

				/* The previous overlap to resolve col.*/
				Vec2 prevOverLap = Physics::GetPreviousOverlap(npc, tile);

				if (prevOverLap.y > 0)
				{
					if (npc->getComponent<CTransform>()->prevPos.x < tile->getComponent<CTransform>()->pos.x)
					{
						npc->getComponent<CTransform>()->pos.x -= overLap.x;
					}
					else
					{
						npc->getComponent<CTransform>()->pos.x += overLap.x;
					}
				}
				else if (prevOverLap.x > 0)
				{
					/* Check bottom of the tile for Col: */
					if (npc->getComponent<CTransform>()->prevPos.y < tile->getComponent<CTransform>()->pos.y)
					{
						npc->getComponent<CTransform>()->pos.y -= overLap.y;
					}

					/* Check top for Col. Accepts: prevOverLap.x = 0 */
					else
					{
						npc->getComponent<CTransform>()->pos.y += overLap.y;
						NPC_Grounded = true;
					}

				}
			}

		}

		if (NPC_Grounded)
		{
			npc->getComponent<CState>()->grounded = true;
		}
		else
		{
			npc->getComponent<CState>()->grounded = false;
		}


		/** Check foor NPC player cols**/

		Vec2 overLap = Physics::GetOverlap(m_player, npc);

		if (overLap.x >= 0 && overLap.y >= 0)
		{
			/* The previous overlap to resolve col.*/
			Vec2 prevOverLap =  Physics::GetPreviousOverlap(m_player, npc);

			/* Check for X collison, resolve first.*/
			if (prevOverLap.y > 0)
			{
				if (m_player->getComponent<CTransform>()->prevPos.x < npc->getComponent<CTransform>()->pos.x)
				{
					m_player->getComponent<CTransform>()->pos.x -= overLap.x;
				}
				else
				{
					m_player->getComponent<CTransform>()->pos.x += overLap.x;
				}
			}

			/* Check for Y Collisions */
			else if (prevOverLap.x > 0)
			{
				/* Check bottom of the tile for Col: */
				if (m_player->getComponent<CTransform>()->prevPos.y < npc->getComponent<CTransform>()->pos.y)
				{
					m_player->getComponent<CTransform>()->pos.y -= overLap.y;
				}

				/* Check top for Col. Accepts: prevOverLap.x = 0 */
				else
				{
					m_player->getComponent<CTransform>()->pos.y += overLap.y;
					player_grounded = true;
				}
			}

		}
	}

	if (player_grounded)
	{
		m_player->getComponent<CState>()->grounded = true;
	}
	else
	{
		m_player->getComponent<CState>()->grounded = false;
	}
}



/**
 * [GameState_Play::sAnimation description]
 */
void GameState_Play::sAnimation()
{

	std::string playerState = m_player->getComponent<CState>()->state;

	/* Retreive the players Animation name.*/
	std::string animationName = m_player->getComponent<CAnimation>()->animation.getName();

	if (playerState == "stand"  && animationName != "player_stand")
	{
		m_player->addComponent<CAnimation>(m_game.getAssets().getAnimation("player_stand"), true);
	}
	else if (playerState == "jump"  && animationName != "player_jump")
	{
		m_player->addComponent<CAnimation>(m_game.getAssets().getAnimation("player_jump"), true);
	}
	else if (playerState == "run" && animationName != "player_run")
	{
		m_player->addComponent<CAnimation>(m_game.getAssets().getAnimation("player_run"), true);
	}




	for (auto e : m_entityManager.getEntities())
	{
		if (e->getComponent<CAnimation>()->animation.hasEnded() && !e->getComponent<CAnimation>()->repeat) 	{ e->destroy(); }
		else { e->getComponent<CAnimation>()->animation.update(); }
	}

}

/**
 * [GameState_Play::sUserInput description]
 */
void GameState_Play::sUserInput()
{
	auto pInput = m_player->getComponent<CInput>();


	sf::Event event;
	while (m_game.window().pollEvent(event))
	{
		if (event.type == sf::Event::Closed)
		{
			m_game.quit();
		}
		// this event is triggered when a key is pressed
		if (event.type == sf::Event::KeyPressed)

		{
			switch (event.key.code)
			{
			case sf::Keyboard::Escape:  { m_game.popState(); break; }
			case sf::Keyboard::W:       { pInput->up = true; break; }
			case sf::Keyboard::A:       { pInput->left = true; break; }
			case sf::Keyboard::S:       { pInput->down = true; break; }
			case sf::Keyboard::D:       { pInput->right = true; break; }
			case sf::Keyboard::Z:       { init(m_levelPath); break; }
			case sf::Keyboard::R:       { m_drawTextures = !m_drawTextures; break; }
			case sf::Keyboard::F:       { m_drawCollision = !m_drawCollision; break; }
			case sf::Keyboard::Y:       { m_follow = !m_follow; break; }
			case sf::Keyboard::P:       { setPaused(!m_paused);  break; }
			default : {break;}
			}
		}

		else if (event.type == sf::Event::KeyReleased)
		{
			switch (event.key.code)
			{
			case sf::Keyboard::W:       { pInput->up = false; break; }
			case sf::Keyboard::A:       { pInput->left = false; break; }
			case sf::Keyboard::S:       { pInput->down = false; break; }
			case sf::Keyboard::D:       { pInput->right = false; break; }
			default : {break;}
			}
		}
	}
}

/**
 * [GameState_Play::sRender description]
 */
void GameState_Play::sRender()
{
	m_game.window().clear(sf::Color(255, 192, 122));

	sf::View view(m_game.window().getDefaultView());


	/* Set camera to follow player */
	view.setCenter(m_player->getComponent<CTransform>()->pos.x, m_game.window().getDefaultView().getSize().y - m_player->getComponent<CTransform>()->pos.y);
	m_game.window().setView(view);

	/* draw all Entity textures / animations */
	if (m_drawTextures)
	{
		for (auto e : m_entityManager.getEntities())
		{
			auto transform = e->getComponent<CTransform>();

			{
				auto animation = e->getComponent<CAnimation>()->animation;
				animation.getSprite().setRotation(transform->angle);
				animation.getSprite().setPosition(transform->pos.x, m_game.window().getDefaultView().getSize().y -  transform->pos.y);
				animation.getSprite().setScale(transform->scale.x,  transform->scale.y);
				m_game.window().draw(animation.getSprite());
			}
		}
	}


	// draw all Entity collision bounding boxes with a rectangleshape
	if (m_drawCollision)
	{
		sf::CircleShape dot(4);
		dot.setFillColor(sf::Color::Black);
		for (auto e : m_entityManager.getEntities())
		{
			if (e->hasComponent<CBoundingBox>())
			{
				auto box = e->getComponent<CBoundingBox>();
				auto transform = e->getComponent<CTransform>();
				sf::RectangleShape rect;
				rect.setSize(sf::Vector2f(box->size.x - 1, box->size.y - 1));
				rect.setOrigin(sf::Vector2f(box->halfSize.x, box->halfSize.y));
				rect.setPosition(transform->pos.x, m_game.window().getDefaultView().getSize().y - transform->pos.y);
				rect.setFillColor(sf::Color(0, 0, 0, 0));

				if (box->blockMove && box->blockVision) { rect.setOutlineColor(sf::Color::Black); }
				if (box->blockMove && !box->blockVision) { rect.setOutlineColor(sf::Color::Blue); }
				if (!box->blockMove && box->blockVision) { rect.setOutlineColor(sf::Color::Red); }
				if (!box->blockMove && !box->blockVision) { rect.setOutlineColor(sf::Color::White); }
				rect.setOutlineThickness(1);
				m_game.window().draw(rect);
			}

			if (e->hasComponent<CPatrol>())
			{
				auto & patrol = e->getComponent<CPatrol>()->positions;
				for (size_t p = 0; p < patrol.size(); p++)
				{
					dot.setPosition(patrol[p].x, patrol[p].y);
					m_game.window().draw(dot);
				}
			}

			if (e->hasComponent<CFollowPlayer>())
			{
				sf::VertexArray lines(sf::LinesStrip, 2);
				lines[0].position.x = e->getComponent<CTransform>()->pos.x;
				lines[0].position.y = m_game.window().getDefaultView().getSize().y - e->getComponent<CTransform>()->pos.y;
				lines[0].color = sf::Color::Black;
				lines[1].position.x = m_player->getComponent<CTransform>()->pos.x;
				lines[1].position.y = m_game.window().getDefaultView().getSize().y - m_player->getComponent<CTransform>()->pos.y;
				lines[1].color = sf::Color::Black;
				m_game.window().draw(lines);
				dot.setPosition(e->getComponent<CFollowPlayer>()->home.x, e->getComponent<CFollowPlayer>()->home.y);
				m_game.window().draw(dot);
			}
		}


	}


	m_game.window().display();
}
