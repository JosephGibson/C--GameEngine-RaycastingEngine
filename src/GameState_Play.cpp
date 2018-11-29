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
 * [GameState_Play::init description]
 * @param levelPath [description]
 */
void GameState_Play::init(const std::string & levelPath)
{

	loadLevel(levelPath);
	gridBuilder();
}

/**
 * [GameState_Play::loadLevel description]
 * @param filename [description]
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
			file >> m_playerConfig.X >> m_playerConfig.Y >> m_playerConfig.CX >> m_playerConfig.CY >> m_playerConfig.SPEED;
		}

		/*Tile found, load.*/
		else if (token == "Tile")
		{
			std::string name;
			int RX, RY, TX, TY, BM, BV;
			file >> name >> RX >> RY >> TX >> TY >> BM >> BV;
			auto e = m_entityManager.addEntity(token);
			e->addComponent<CTransform>()->pos = Vec2(RX * m_game.window().getDefaultView().getSize().x + (TX * 64) + 32, RY * m_game.window().getDefaultView().getSize().y + (TY * 64) + 32);
			e->addComponent<CAnimation>(m_game.getAssets().getAnimation(name), true);
			e->addComponent<CBoundingBox>(e->getComponent<CAnimation>()->animation.getSize(), BM, BV);
			e->addComponent<CDraggable>(true);
		}

		/*NPC found, load.*/
		else if (token == "NPC")
		{
			std::string name, aiName;
			int RX, RY, BM, BV, speed;
			int TY, TX;
			file >> name >> RX >> RY >> TX >> TY >> BM >> BV >> aiName;

			auto e = m_entityManager.addEntity(token);
			e->addComponent<CTransform>()->pos = Vec2(RX * m_game.window().getDefaultView().getSize().x + (TX * 64) + 32, RY * m_game.window().getDefaultView().getSize().y + (TY * 64) + 32);
			e->addComponent<CAnimation>(m_game.getAssets().getAnimation(name), true);
			e->addComponent<CBoundingBox>(e->getComponent<CAnimation>()->animation.getSize(), BM, BV);
			e->addComponent<CDraggable>(true);

			if ( aiName == "Follow")
			{
				file >> speed;
				e->addComponent<CFollowPlayer>(Vec2(0, 0), speed);
				e->getComponent<CFollowPlayer>()->home = Vec2(RX * m_game.window().getDefaultView().getSize().x + (TX * 64) + 32, RY * m_game.window().getDefaultView().getSize().y + (TY * 64)  + 32);
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
					pos.push_back(Vec2(RX * m_game.window().getDefaultView().getSize().x + (x * 64) + 32, RY * m_game.window().getDefaultView().getSize().y + (y * 64) + 32));
					i++;
				}
				e->addComponent<CPatrol>(pos, speed);
			}

		}
	}

	spawnPlayer();

}

void GameState_Play::spawnPlayer()
{
	m_player = m_entityManager.addEntity("player");
	m_player->addComponent<CTransform>(Vec2(m_playerConfig.X, m_playerConfig.Y));
	m_player->getComponent<CTransform>()->facing = Vec2(0, 1);
	m_player->addComponent<CAnimation>(m_game.getAssets().getAnimation("Black"), true);
	m_player->addComponent<CInput>();
	m_player->addComponent<CBoundingBox>(Vec2(m_playerConfig.CX, m_playerConfig.CY), true, true);;
}


/**
 * [GameState_Play::update description]
 */
void GameState_Play::update()
{
	m_entityManager.update();
	if (!m_paused)
	{
		//	sAI();
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
 * [GameState_Play::sMovement description]
 */
void GameState_Play::sMovement()
{

	/* Do NPC speed asignments for current cycle */
	for (auto &  npc : m_entityManager.getEntities("NPC"))
	{
		npc->getComponent<CTransform>()->prevPos = npc->getComponent<CTransform>()->pos;
		npc->getComponent<CTransform>()->pos += npc->getComponent<CTransform>()->speed;
	}


	/* Save players speed for easy access. */
	Vec2 speed = Vec2(0, 0);
	auto pInput = m_player->getComponent<CInput>();

	if (pInput->up)
	{
		speed.y = -3.5;
	}
	else if (pInput->down)
	{
		speed.y = 3.5;
	}

	if (pInput->right)
	{
		speed.x = 3.5;
	}
	else if (pInput->left)
	{
		speed.x = -3.5;
	}
	/* Set m_players PrevPos = Pos, Set Pos += Speed */
	m_player->getComponent<CTransform>()->speed = speed;
	m_player->getComponent<CTransform>()->prevPos = m_player->getComponent<CTransform>()->pos;
	m_player->getComponent<CTransform>()->pos += m_player->getComponent<CTransform>()->speed;

}

/**
 * [GameState_Play::sAI description]
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
				npc->getComponent<CTransform>()->speed = Norm;
			}
			/* Check if the NPCS is close that a movement would push them too far. */
			else if (npc->getComponent<CTransform>()->pos.dist(npc->getComponent<CFollowPlayer>()->home) >= npc->getComponent<CFollowPlayer>()->speed * 1.15)
			{
				Vec2 Norm = (npc->getComponent<CFollowPlayer>()->home - npc->getComponent<CTransform>()->pos).norm();
				Norm *= npc->getComponent<CFollowPlayer>()->speed;
				npc->getComponent<CTransform>()->speed = Norm;
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
				npc->getComponent<CTransform>()->speed = Norm;
			}
			else
			{
				npc->getComponent<CPatrol>()->currentPosition = nextPosNum;
			}

		}
	}
}

/**
 * [GameState_Play::sLifespan description]
 */
void GameState_Play::sLifespan()
{
	/* Check all life span clocks to see if ElapsedTime >= Life, Del the entity if so.*/
	for (auto e : m_entityManager.getEntities())
	{
		if (e->hasComponent<CLifeSpan>() && e->getComponent<CLifeSpan>()->clock.getElapsedTime().asMilliseconds() >= e->getComponent<CLifeSpan>()->lifespan)
		{
			/*Once the sword is dead, reset canShoot, check for both animations. */
			if (e->getComponent<CAnimation>()->animation.getName() == "SwordUp" || e->getComponent<CAnimation>()->animation.getName() == "SwordRight")
			{
				m_player->getComponent<CInput>()->canShoot = true;
			}

			e->destroy();
		}
	}

}

/**
 * [GameState_Play::sCollision description]
 */

void GameState_Play::sCollision()
{
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
					}
				}

			}

			/* Do NPC tile col, this could be wrapped in anoter function since its the same as player col.*/
			for (auto & npc : m_entityManager.getEntities("NPC"))
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
						}

					}


				}

			}
		}
	}
}


/**
 * [GameState_Play::gridBuilder description]
 */
void GameState_Play::gridBuilder()
{
	int w = m_game.window().getDefaultView().getSize().x;
	int h = m_game.window().getDefaultView().getSize().y;

	for (auto room : m_RoomsX)
	{
		int index = 64;
		while (index < w)
		{
			sf::VertexArray line(sf::LinesStrip, 2);
			line[0].color = sf::Color::Black;
			line[1].color = sf::Color::Black;
			int x = room * w + index;
			line[0].position = sf::Vector2f(x, m_RoomsY.front() * h);
			line[1].position = sf::Vector2f(x, m_RoomsY.back() * h + h);
			m_grid.push_back(line);
			index += 64;

		}

	}
	for (auto room : m_RoomsY)
	{
		int index = 0;
		while (index < h)
		{
			sf::VertexArray line(sf::LinesStrip, 2);
			line[0].color = sf::Color::Black;
			line[1].color = sf::Color::Black;
			int y = room * h + index;
			line[0].position = sf::Vector2f(m_RoomsX.front() * w, y);
			line[1].position = sf::Vector2f(m_RoomsX.back() * w + w, y);
			m_grid.push_back(line);
			index += 64;
		}
	}


	int index = m_RoomsX.front();

	while (index <= static_cast<int>(m_RoomsX.size()))
	{
		sf::VertexArray quad(sf::Quads, 4);
		quad[0].color = sf::Color::Red;
		quad[1].color = sf::Color::Red;
		quad[2].color = sf::Color::Red;
		quad[3].color = sf::Color::Red;
		quad[0].position = sf::Vector2f(index * w + 2, h * (m_RoomsY.back() + 1));
		quad[2].position = sf::Vector2f(index * w - 2, h *  m_RoomsY.front());
		quad[1].position = sf::Vector2f(index * w - 2, h * (m_RoomsY.back() + 1));
		quad[3].position = sf::Vector2f(index * w + 2, h * m_RoomsY.front());
		m_grid.push_back(quad);
		index++;
	}

	index = m_RoomsY.front();

	while (index <= static_cast<int>(m_RoomsY.size()))
	{
		sf::VertexArray quad(sf::Quads, 4);
		quad[0].color = sf::Color::Red;
		quad[1].color = sf::Color::Red;
		quad[2].color = sf::Color::Red;
		quad[3].color = sf::Color::Red;
		quad[0].position = sf::Vector2f(w * (m_RoomsX.back() + 1), index * h - 2);
		quad[2].position = sf::Vector2f(w *  m_RoomsX.front(), index * h + 2);
		quad[1].position = sf::Vector2f(w * (m_RoomsX.back() + 1), index * h + 2);
		quad[3].position = sf::Vector2f(w * m_RoomsX.front(), index * h - 2);
		m_grid.push_back(quad);
		index++;
	}
}


/**
 * [GameState_Play::sAnimation description]
 */
void GameState_Play::sAnimation()
{


	// Implement updating and setting of all animations here
	// std::string playerState = m_player->getComponent<CState>()->state;

	//  Retreive the players Wnimation name.
	// std::string animationName = m_player->getComponent<CAnimation>()->animation.getName();


	// check all entities for their animation, if ended, delete entity
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

	if (m_follow)
	{

		view.setCenter(m_player->getComponent<CTransform>()->pos.x, m_player->getComponent<CTransform>()->pos.y);
		m_game.window().setView(view);
	}

	/* Else adjust the players location based off the room they are in.*/
	else
	{
		auto window_size = m_game.window().getDefaultView().getSize();
		Vec2 player_pos = m_player->getComponent<CTransform>()->pos;
		float x = window_size.x * floor(player_pos.x / window_size.x);
		float y = window_size.y * floor(player_pos.y / window_size.y);
		view.move(x, y);
		view.setCenter(view.getCenter());
		m_game.window().setView(view);
	}

	/* draw all Entity textures / animations */
	if (m_drawTextures)
	{
		for (auto e : m_entityManager.getEntities())
		{
			auto transform = e->getComponent<CTransform>();

			if (e->hasComponent<CAnimation>())
			{
				auto animation = e->getComponent<CAnimation>()->animation;
				animation.getSprite().setRotation(transform->angle);
				animation.getSprite().setPosition(transform->pos.x, transform->pos.y);
				animation.getSprite().setScale(transform->scale.x, transform->scale.y);
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
				rect.setPosition(transform->pos.x, transform->pos.y);
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
				lines[0].position.y = e->getComponent<CTransform>()->pos.y;
				lines[0].color = sf::Color::Black;
				lines[1].position.x = m_player->getComponent<CTransform>()->pos.x;
				lines[1].position.y = m_player->getComponent<CTransform>()->pos.y;
				lines[1].color = sf::Color::Black;
				m_game.window().draw(lines);
				dot.setPosition(e->getComponent<CFollowPlayer>()->home.x, e->getComponent<CFollowPlayer>()->home.y);
				m_game.window().draw(dot);
			}
		}


	}
	m_game.window().display();
}
