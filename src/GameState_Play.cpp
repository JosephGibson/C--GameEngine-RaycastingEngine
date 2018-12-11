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
	m_lightPoly = sf::VertexArray(sf::TrianglesFan);
	m_background.create(1344, 768);
	m_backgroundMusic.setBuffer(m_game.getAssets().getSound("music_1"));
	m_backgroundMusic.setLoop(true);
	m_backgroundMusic.setVolume(70);
	m_backgroundMusic.play();
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

		/*Goal found, load.*/
		else if (token == "Goal")
		{
			float TX, TY;
			file >> TX >> TY;
			auto e = m_entityManager.addEntity(token);
			e->addComponent<CTransform>()->pos = Vec2(TX * 64 + 32, TY * 64 + 32);
			e->addComponent<CAnimation>(m_game.getAssets().getAnimation("goal"), true);
			e->addComponent<CBoundingBox>(e->getComponent<CAnimation>()->animation.getSize(), 0, 0);
		}

		/*Move Tile found, load.*/
		else if (token == "Move_Tile")
		{
			std::string name;
			float TX1, TY1, TX2, TY2, speed;
			int BM, BV;
			file >> name >> TX1 >> TY1 >> TX2 >> TY2 >> speed >> BM >> BV;
			auto e = m_entityManager.addEntity("Tile");
			e->addComponent<CTransform>()->pos = Vec2(TX1 * 64 + 32, TY1 * 64 + 32);
			e->addComponent<CAnimation>(m_game.getAssets().getAnimation(name), true);
			e->addComponent<CBoundingBox>(e->getComponent<CAnimation>()->animation.getSize(), BM, BV);
			e->addComponent<CMoveTile>(e->getComponent<CTransform>()->pos,  Vec2(TX2 * 64 + 32, TY2 * 64 + 32), speed);
		}

		else if (token == "Item") // items should have all the same data as a Tile as well as int for the amount and a bool for the item type (true for ammo, false for medkits)
		{
			std::string name , type;
			float TX, TY;
			int  amount;
			bool isAmmo;

			file >> name >> TX >> TY  >> amount;
			auto e = m_entityManager.addEntity(token);
			e->addComponent<CTransform>()->pos = Vec2(TX * 64 + 32, TY * 64 + 32);
			e->addComponent<CAnimation>(m_game.getAssets().getAnimation(name), true);
			e->addComponent<CBoundingBox>(e->getComponent<CAnimation>()->animation.getSize(), 1, 0);
			if (name == "ammo_pack")
			{
				isAmmo = true;
			}
			else
			{
				isAmmo = false;
			}
			e->addComponent<CItem>(amount, isAmmo);
		}


		/*NPC found, load.*/
		else if (token == "NPC")
		{

			std::string name, aiName;
			int BM, BV, HP, KnockBack;
			float TY, TX, grav, speed, BoundingX, BoundingY;
			file >> name >> TX >> TY >> BM >> BV >> grav >> BoundingX >> BoundingY >> HP >> KnockBack >> aiName;
			auto e = m_entityManager.addEntity(token);
			e->addComponent<CAnimation>(m_game.getAssets().getAnimation(name), true);
			e->addComponent<CTransform>()->pos = Vec2((TX * 64) + e->getComponent<CAnimation>()->animation.getSize().x / 2, TY * 64 + e->getComponent<CAnimation>()->animation.getSize().y / 2);
			e->addComponent<CBoundingBox>(Vec2(e->getComponent<CAnimation>()->animation.getSize().x * BoundingX,  e->getComponent<CAnimation>()->animation.getSize().y * BoundingY), BM, BV);
			e->addComponent<CState>("STATE_NAME_HERE");
			e->addComponent<CGravity>(grav, KnockBack);
			e->addComponent<CHealth>(HP);
			e->addComponent<CDamage>(10);  /// health and damage values could be added as in config text , default values are used for now

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
			else if ( aiName == "Steer")
			{
				float speed, scale;
				file >> speed >> scale;
				e->addComponent<CSteer>(speed, scale, e->getComponent<CTransform>()->pos);
			}
		}
	}

	spawnPlayer();

	m_ammoCount.setFont(m_game.getAssets().getFont("titleFont"));
	m_ammoCount.setCharacterSize(32);
	m_ammoCount.setFillColor(sf::Color::White);

	m_hpKitCount.setFont(m_game.getAssets().getFont("titleFont"));
	m_hpKitCount.setCharacterSize(32);
	m_hpKitCount.setFillColor(sf::Color::White);

	m_weaponSelected.setFont(m_game.getAssets().getFont("titleFont"));
	m_weaponSelected.setCharacterSize(32);
	m_weaponSelected.setFillColor(sf::Color::White);

	m_currentHP.setFont(m_game.getAssets().getFont("titleFont"));
	m_currentHP.setCharacterSize(32);
	m_currentHP.setFillColor(sf::Color::White);

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
	m_player->addComponent<CGravity>(m_playerConfig.Gravity, 0);
	m_player->addComponent<CState>("stand");
	m_player->addComponent<CLight>(350);
	m_player->addComponent<CHealth>(100);
	m_player->addComponent<CInventory>();
	m_player->getComponent<CTransform>()->facing.x = 1;
}


/**
*
* @brief		{function to consumes health kit and heal hp}
*
*/

void GameState_Play::useHealthKit()
{
	if (m_player->getComponent<CInventory>()->numOfHealthKits > 0)
	{
		m_pickUpSound.setBuffer(m_game.getAssets().getSound("health_pack"));
		m_pickUpSound.play();
		m_player->getComponent<CInventory>()->numOfHealthKits -= 1;

		if (m_player->getComponent<CHealth>()->hp < 100)
		{
			m_player->getComponent<CHealth>()->hp += 25;

			if (m_player->getComponent<CHealth>()->hp > 100) // currently default max health is 200, should be changed to correspond to a set max health value in player config
			{
				m_player->getComponent<CHealth>()->hp = 100;
			}
		}

	}
}


/**
 * @brief      { Spawn a bullet  }
 *
 * @param[in]  entity  The entity in which to spawn a bullet.
 */

void GameState_Play::spawnBullet(std::shared_ptr<Entity> entity) // add check for ammo and ammo depletion
{
	if (m_player->getComponent<CInventory>()->ammo)
	{
		m_playerAttackSound.setBuffer(m_game.getAssets().getSound("gun_shoot"));
		m_playerAttackSound.play();
		m_shoot_timer.restart();

		auto bullet = m_entityManager.addEntity("bullet");

		if (m_player->getComponent<CTransform>()->facing.x == 1)
		{
			bullet->addComponent<CTransform>(Vec2(m_player->getComponent<CTransform>()->pos.x + 15, m_player->getComponent<CTransform>()->pos.y));
			bullet->getComponent<CTransform>()->speed = Vec2(m_playerConfig.SPEED * 2.5, 0);
		}
		else
		{
			bullet->addComponent<CTransform>(Vec2(m_player->getComponent<CTransform>()->pos.x - 15, m_player->getComponent<CTransform>()->pos.y));
			bullet->getComponent<CTransform>()->speed = Vec2(-m_playerConfig.SPEED * 2.5, 0);
		}

		bullet->addComponent<CAnimation>(m_game.getAssets().getAnimation("bullet"), true);
		bullet->addComponent<CBoundingBox>(Vec2(m_game.getAssets().getAnimation("bullet").getSize().x, m_game.getAssets().getAnimation("bullet").getSize().y), false, false);
		bullet->addComponent<CLifeSpan>(2000);
		bullet->addComponent<CDamage>(100);
		m_player->getComponent<CInventory>()->ammo -= 1;

		m_canShoot = false;
	}
	else
	{
		m_shoot_timer.restart();
		m_playerAttackSound.setBuffer(m_game.getAssets().getSound("gun_click"));
		m_playerAttackSound.play();
		m_canShoot = false;
	}
}


/**
 * @brief      { Spawn a pipe at a given entity }
 *
 * @param[in]  entity  The sword a given entity.
 */

void GameState_Play::spawnMeele(std::shared_ptr<Entity> entity)
{
	m_playerAttackSound.setBuffer(m_game.getAssets().getSound("pipe_swing"));
	m_playerAttackSound.play();
	m_shoot_timer.restart();
	auto meele = m_entityManager.addEntity("meele");
	meele->addComponent<CTransform>();
	meele->addComponent<CAnimation>(m_game.getAssets().getAnimation("pipe"), true);
	meele->addComponent<CBoundingBox>(Vec2(m_game.getAssets().getAnimation("pipe").getSize().x * 1.35, m_game.getAssets().getAnimation("pipe").getSize().y * 2), false, false);
	meele->addComponent<CLifeSpan>(250);
	meele->addComponent<CDamage>(25);
	m_canShoot = false;
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
		sHealth();
		sAnimation();
		sLight();
		sUserInput();
		sRender();
	}
	else
	{
		sAnimation();
		sLight();
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
		if (!npc->getComponent<CState>()->grounded)
		{
			npc->getComponent<CTransform>()->speed.y -= npc->getComponent<CGravity>()->gravity;
		}
		else
		{
			npc->getComponent<CTransform>()->speed.y = 0;
		}
		if (!npc->hasComponent<CSteer>())
		{
			npc->getComponent<CTransform>()->prevPos = npc->getComponent<CTransform>()->pos;
			npc->getComponent<CTransform>()->pos += npc->getComponent<CTransform>()->speed;
		}
	}

	/** Move the moving tiles **/
	for (auto &  tile : m_entityManager.getEntities("Tile"))
	{
		if (tile->hasComponent<CMoveTile>())
		{
			Vec2 target = Vec2(0, 0);

			/** Check the tile for which point to move to. **/
			if (tile->getComponent<CMoveTile>()->point == 0)
			{
				target = tile->getComponent<CMoveTile>()->pos2;
			}
			else
			{
				target = tile->getComponent<CMoveTile>()->pos1;
			}

			/** Check for distance and speed. **/
			if (tile->getComponent<CTransform>()->pos.dist(target) >= 5)
			{
				Vec2 Norm = (target - tile->getComponent<CTransform>()->pos).norm();
				Norm *= tile->getComponent<CMoveTile>()->speed;
				tile->getComponent<CTransform>()->pos += Norm;
			}
			else
			{
				if (tile->getComponent<CMoveTile>()->point == 0)
				{
					tile->getComponent<CMoveTile>()->point = 1;
				}
				else
				{
					tile->getComponent<CMoveTile>()->point = 0;
				}
			}
		}
	}

	/* Save players speed for easy access. */
	Vec2 speed = m_player->getComponent<CTransform>()->speed;
	auto pInput = m_player->getComponent<CInput>();
	std::string state = "stand";

	/** Left/ Right control block. **/
	if (pInput->right)
	{
		speed.x = m_playerConfig.SPEED;
		m_player->getComponent<CTransform>()->scale.x = 1;
		m_player->getComponent<CTransform>()->facing.x = 1;
		state = "run";

		/** check for foot steps **/
		if (m_playerSound.getStatus() != sf::Sound::Status::Playing && m_player->getComponent<CState>()->grounded)
		{
			m_playerSound.setBuffer(m_game.getAssets().getSound("player_step"));
			m_playerSound.play();
		}
	}
	else if (pInput->left)
	{
		speed.x = -m_playerConfig.SPEED;
		m_player->getComponent<CTransform>()->scale.x = -1;
		m_player->getComponent<CTransform>()->facing.x = -1;
		state = "run";

		/** check for foot steps **/
		if (m_playerSound.getStatus() != sf::Sound::Status::Playing && m_player->getComponent<CState>()->grounded)
		{
			m_playerSound.setBuffer(m_game.getAssets().getSound("player_step"));
			m_playerSound.play();
		}

	}
	else
	{
		speed.x = 0;
	}

	/** Jumping control bloclk.  **/
	if (!m_player->getComponent<CState>()->grounded)
	{
		speed.y -= m_player->getComponent<CGravity>()->gravity;
		state = "jump";
	}
	else if (pInput->up)
	{
		speed.y = m_playerConfig.JumpSpeed;
		m_playerSound.setBuffer(m_game.getAssets().getSound("player_jump"));
		m_playerSound.play();
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



	for (auto & bullet : m_entityManager.getEntities("bullet"))
	{
		bullet->getComponent<CTransform>()->prevPos =  bullet->getComponent<CTransform>()->pos;
		bullet->getComponent<CTransform>()->pos += bullet->getComponent<CTransform>()->speed;
	}

	for (auto meele : m_entityManager.getEntities("meele"))
	{
		if (m_player->getComponent<CTransform>()->facing.x == 1)
		{
			meele->getComponent<CTransform>()->pos = Vec2(m_player->getComponent<CTransform>()->pos.x + 42, m_player->getComponent<CTransform>()->pos.y);
			meele->getComponent<CTransform>()->scale.x = 1;
		}
		else
		{
			meele->getComponent<CTransform>()->pos = Vec2(m_player->getComponent<CTransform>()->pos.x - 42, m_player->getComponent<CTransform>()->pos.y);
			meele->getComponent<CTransform>()->scale.x = -1;
		}

	}
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
				if (m_hurtSound.getStatus() != sf::Sound::Status::Playing)
				{
					std::string animationName = npc->getComponent<CAnimation>()->animation.getName();
					m_hurtSound.setBuffer(m_game.getAssets().getSound(animationName));
					m_hurtSound.play();

				}

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
				//	m_hurtSound.stop();
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

		else if (npc->hasComponent<CSteer>())
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
			if (can_see)
			{

				if (m_hurtSound.getStatus() != sf::Sound::Status::Playing)
				{
					std::string animationName = npc->getComponent<CAnimation>()->animation.getName();
					m_hurtSound.setBuffer(m_game.getAssets().getSound(animationName));
					m_hurtSound.play();

				}

				if (npc->getComponent<CSteer>()->vel.x != 0 && npc->getComponent<CSteer>()->vel.y != 0 )
				{
					Vec2 desired = m_player->getComponent<CTransform>()->pos - npc->getComponent<CTransform>()->pos;
					desired = desired.norm() * npc->getComponent<CSteer>()->speed;
					Vec2 steering = (desired - npc->getComponent<CSteer>()->vel) * npc->getComponent<CSteer>()->scale ;


					npc->getComponent<CSteer>()->vel = npc->getComponent<CSteer>()->vel + steering;
					npc->getComponent<CTransform>()->prevPos = npc->getComponent<CTransform>()->pos;
					npc->getComponent<CTransform>()->pos += npc->getComponent<CSteer>()->vel;

					if (npc->getComponent<CTransform>()->pos.x < m_player->getComponent<CTransform>()->pos.x)
					{
						npc->getComponent<CTransform>()->scale.x = -1;
					}
					else
					{
						npc->getComponent<CTransform>()->scale.x = 1;
					}
				}
				else
				{
					Vec2 vel = (m_player->getComponent<CTransform>()->pos - npc->getComponent<CTransform>()->pos).norm();
					vel *= npc->getComponent<CSteer>()->speed;
					npc->getComponent<CSteer>()->vel = vel;
				}
			}
			/* Check if the NPCS is close that a movement would push them too far. */
			else if (npc->getComponent<CTransform>()->pos.dist(npc->getComponent<CSteer>()->home) >= npc->getComponent<CSteer>()->speed * 1.15)
			{
				Vec2 Norm = (npc->getComponent<CSteer>()->home - npc->getComponent<CTransform>()->pos).norm();
				Norm *= npc->getComponent<CSteer>()->speed;
				if (npc->getComponent<CState>()->grounded || npc->getComponent<CGravity>()->gravity == 0)
				{
					npc->getComponent<CTransform>()->pos += Norm;
				}
				else
				{
					npc->getComponent<CTransform>()->speed.x = Norm.x;
				}
			}
			/* Else, the player is now home. */
			else
			{
				npc->getComponent<CTransform>()->pos = npc->getComponent<CSteer>()->home;
			}
		}
	}
}


/**
 * @brief      { The Life Span Sytem, handles destroying entities. }
 */

void GameState_Play::sLifespan()
{

	if (!m_canShoot && m_shoot_timer.getElapsedTime().asMilliseconds() > m_shoot_Max)
	{
		m_canShoot = true;
	}
	/* Check all life span clocks to see if ElapsedTime >= Life, Del the entity if so.*/
	for (auto e : m_entityManager.getEntities())
	{
		if (e->hasComponent<CLifeSpan>() && e->getComponent<CLifeSpan>()->clock.getElapsedTime().asMilliseconds() >= e->getComponent<CLifeSpan>()->lifespan)
		{
			e->destroy();
		}
	}

}


/*
* @ brief { Handles Npc and player death when health reaches 0}
*/

void GameState_Play::sHealth()
{
	if (m_player->getComponent<CHealth>()->hp <= 0 || m_player->getComponent<CTransform>()->pos.y < -1000)
	{
		m_game.popState();
	}

	for (auto & npc : m_entityManager.getEntities("NPC"))
	{
		if (npc->hasComponent<CHealth>())
		{
			if (npc->getComponent<CHealth>()->hp <= 0)
			{
				npc->destroy();
				auto death = m_entityManager.addEntity("effect");
				death->addComponent<CTransform>();
				death->getComponent<CTransform>()->pos = npc->getComponent<CTransform>()->pos;
				death->addComponent<CAnimation>(m_game.getAssets().getAnimation("death"), false);
				m_deathSound.setBuffer(m_game.getAssets().getSound("death"));
				m_deathSound.play();
				m_hurtSound.stop();
			}
		}
	}

}


/**
 * @brief      { The collision system. }
 */

void GameState_Play::sCollision()
{

	bool player_grounded = false;

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

				if (prevOverLap.x > 0)
				{
					/* Check bottom of the tile for Col: */
					if (npc->getComponent<CTransform>()->prevPos.y < tile->getComponent<CTransform>()->pos.y)
					{
						npc->getComponent<CTransform>()->speed.y = 0;
						npc->getComponent<CTransform>()->pos.y -= overLap.y;
					}

					/* Check top for Col. Accepts: prevOverLap.x = 0 */
					else
					{
						npc->getComponent<CTransform>()->pos.y += overLap.y;
						NPC_Grounded = true;
					}

				}
				else if (prevOverLap.y > 5)
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
			}

		}

		/** At the end of update cycle NPC is grounded thus set state. **/
		if (NPC_Grounded)
		{
			npc->getComponent<CState>()->grounded = true;
		}
		else
		{
			npc->getComponent<CState>()->grounded = false;
		}

		/** check for NPC bullet cols **/

		for (auto & bullet : m_entityManager.getEntities("bullet"))
		{
			Vec2 overLap = Physics::GetOverlap(npc, bullet);

			if (overLap.x >= 0 && overLap.y >= 0)
			{

				Vec2 prevOverLap = Physics::GetPreviousOverlap(npc, bullet);
				if (npc->getComponent<CTransform>()->prevPos.x < bullet->getComponent<CTransform>()->pos.x)
				{
					npc->getComponent<CTransform>()->pos.x -= 15;
				}
				else
				{
					npc->getComponent<CTransform>()->pos.x +=  15;
				}

				if (npc->hasComponent<CSteer>())
				{
					npc->getComponent<CSteer>()->vel = Vec2(0.1, 0.1);
				}

				m_hitSound.setBuffer(m_game.getAssets().getSound("gun_hit"));
				m_hitSound.play();

				npc->getComponent<CHealth>()->hp -= bullet->getComponent<CDamage>()->dmg;
				bullet->destroy();
				auto damge = m_entityManager.addEntity("effect");
				damge->addComponent<CTransform>();
				damge->getComponent<CTransform>()->pos = npc->getComponent<CTransform>()->pos;
				damge->addComponent<CAnimation>(m_game.getAssets().getAnimation("death"), false);
			}
		}

		/** check for Meele cols **/
		for (auto & meele : m_entityManager.getEntities("meele"))
		{
			if (meele->hasComponent<CBoundingBox>())
			{
				Vec2 overLap = Physics::GetOverlap(npc, meele);

				if (overLap.x >= 0 && overLap.y >= 0)
				{

					Vec2 prevOverLap = Physics::GetPreviousOverlap(npc, meele);
					if (npc->getComponent<CTransform>()->prevPos.x < meele->getComponent<CTransform>()->pos.x)
					{
						npc->getComponent<CTransform>()->pos.x -= 25;
					}
					else
					{
						npc->getComponent<CTransform>()->pos.x +=  25;
					}

					if (npc->hasComponent<CSteer>())
					{
						npc->getComponent<CSteer>()->vel = Vec2(0.1, 0.1);
					}

					m_hitSound.setBuffer(m_game.getAssets().getSound("pipe_hit"));
					m_hitSound.play();
					auto damge = m_entityManager.addEntity("effect");
					damge->addComponent<CTransform>();
					damge->getComponent<CTransform>()->pos = npc->getComponent<CTransform>()->pos;
					damge->addComponent<CAnimation>(m_game.getAssets().getAnimation("blood"), false);
					npc->getComponent<CHealth>()->hp -= meele->getComponent<CDamage>()->dmg;;
					meele->addComponent<CDamage>(0);
					meele->removeComponent<CBoundingBox>();
				}
			}
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
					m_player->getComponent<CTransform>()->pos.x -= overLap.x + npc->getComponent<CGravity>()->knockback;
					npc->getComponent<CTransform>()->pos.x += overLap.x;
					if (npc->hasComponent<CSteer>())
					{
						npc->getComponent<CSteer>()->vel = Vec2(0.1, 0.1);
					}
				}
				else
				{
					m_player->getComponent<CTransform>()->pos.x += overLap.x +  npc->getComponent<CGravity>()->knockback;
					npc->getComponent<CTransform>()->pos.x -= overLap.x;

					if (npc->hasComponent<CSteer>())
					{
						npc->getComponent<CSteer>()->vel = Vec2(0.1, 0.1);
					}
				}
			}

			/* Check for Y Collisions */
			else if (prevOverLap.x > 0)
			{
				/* Check bottom of the tile for Col: */
				if (m_player->getComponent<CTransform>()->prevPos.y < npc->getComponent<CTransform>()->pos.y)
				{
					m_player->getComponent<CTransform>()->speed.y = 0;
					m_player->getComponent<CTransform>()->pos.y -= overLap.y + 5;
				}

				/* Check top for Col. Accepts: prevOverLap.x = 0 */
				else
				{
					m_player->getComponent<CTransform>()->pos.y += overLap.y + 25;
				}
			}
			m_player->getComponent<CHealth>()->hp -= npc->getComponent<CDamage>()->dmg;
			m_playerSound.setBuffer(m_game.getAssets().getSound("player_hurt"));
			m_playerSound.play();

		}

	}

	/** check for player/item collision **/
	for (auto & item : m_entityManager.getEntities("Item"))
	{
		Vec2 overLap = Physics::GetOverlap(m_player, item);
		if (overLap.x >= 0 && overLap.y >= 0)
		{
			if (item->getComponent<CItem>()->isAmmo)
			{
				m_player->getComponent<CInventory>()->ammo += item->getComponent<CItem>()->amount;
			}
			else
			{
				m_player->getComponent<CInventory>()->numOfHealthKits += item->getComponent<CItem>()->amount;
			}
			m_pickUpSound.setBuffer(m_game.getAssets().getSound("pick_up"));
			m_pickUpSound.play();
			item->destroy();
		}
	}


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

				/* Check for Y Collisions */

				if (prevOverLap.x > 0)
				{
					/* Check bottom of the tile for Col: */
					if (m_player->getComponent<CTransform>()->prevPos.y < tile->getComponent<CTransform>()->pos.y)
					{
						m_player->getComponent<CTransform>()->speed.y = 0;
						m_player->getComponent<CTransform>()->pos.y -= overLap.y;
					}

					/* Check top for Col. Accepts: prevOverLap.x = 0 */
					else
					{
						player_grounded = true;
						if (tile->hasComponent<CMoveTile>())
						{
							m_player->getComponent<CTransform>()->pos.y = tile->getComponent<CTransform>()->pos.y + (64 - tile->getComponent<CMoveTile>()->speed);
						}
						else
						{
							m_player->getComponent<CTransform>()->pos.y += overLap.y;
						}
					}
				}
				/* Check for X collison, resolve first.*/
				else if (prevOverLap.y > 0)
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
			}
			for (auto & bullet : m_entityManager.getEntities("bullet"))
			{
				overLap = Physics::GetOverlap(bullet, tile);
				if (overLap.x >= 0 && overLap.y >= 0)
				{
					bullet->destroy();
				}
			}


		}
	}

	/** Finalize the players state for this cycle **/
	if (player_grounded)
	{
		m_player->getComponent<CState>()->grounded = true;
	}
	else
	{
		m_player->getComponent<CState>()->grounded = false;
	}

	/** Finally check for Goal -> player cols. End level if goal reached. **/
	for (auto & goal : m_entityManager.getEntities("Goal"))
	{
		Vec2 overLap = Physics::GetOverlap(m_player, goal);
		if (overLap.x >= 0 && overLap.y >= 0)
		{
			m_game.popState();
		}
	}

}



/**
 * @brief      { The animation system }
 */

void GameState_Play::sAnimation()
{

	std::string playerState = m_player->getComponent<CState>()->state;

	/* Retreive the players Animation name.*/
	std::string animationName = m_player->getComponent<CAnimation>()->animation.getName();

	if (m_canShoot)
	{
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
	}
	else if (!m_player->getComponent<CInventory>()->meleeSelected)
	{
		m_player->addComponent<CAnimation>(m_game.getAssets().getAnimation("player_shoot"), true);
	}
	else if (m_player->getComponent<CInventory>()->meleeSelected)
	{
		m_player->addComponent<CAnimation>(m_game.getAssets().getAnimation("player_whack"), true);
	}

	/** Update NPC animations **/
	for (auto e : m_entityManager.getEntities())
	{
		if (e->getComponent<CAnimation>()->animation.hasEnded() && !e->getComponent<CAnimation>()->repeat) 	{ e->destroy(); }
		else { e->getComponent<CAnimation>()->animation.update(); }
	}

}


/**
 * @brief      { The lighting system, uses both a 360 degree raycast and vertex aware raycasts. Create a triangle fan to render }
 */

void GameState_Play::sLight()
{
	m_Light_Lines.clear();
	std::vector<Vec2> intersetions;


	/*Cast a ray every 5 degree from player, remove if intersect. */
	Vec2 playPos = m_player->getComponent<CTransform>()->pos;
	Vec2 pPos = m_player->getComponent<CTransform>()->pos;
	pPos.y = m_game.window().getDefaultView().getSize().y  - pPos.y;
	float dist = m_player->getComponent<CLight>()->dist;

	
	for (int angle = 0; angle < 360; angle += 18)
	{

		bool no_intersect = true;

		for (auto & tile : m_entityManager.getEntities("Tile"))
		{
			if (tile->getComponent<CTransform>()->pos.dist(playPos) <  dist)
			{
				if (Physics::LightEntityIntersect(pPos, Vec2(pPos.x + std::cos((angle * 3.145) / 180) * dist, pPos.y + std::sin((angle * 3.145) / 180) * dist), tile))
				{
					no_intersect = false;
					break;
				}
			}
		}

		if (no_intersect)
		{
			Vec2 k = Vec2(std::cos((angle * 3.145) / 180) * dist, std::sin((angle * 3.145) / 180) * dist);
			intersetions.push_back(k);
		}

	}
	

	/*	For every vert in a tile cast a point to the player. */
	/*	Assume not intersection, correct if not.             */
	for (auto & end_tile : m_entityManager.getEntities("Tile"))
	{
		if (end_tile->getComponent<CTransform>()->pos.dist(playPos) <  dist)
		{
			std::vector<bool> points(4);
			points[0] = true;
			points[1] = true;
			points[2] = true;
			points[3] = true;


			Vec2 end_origin = end_tile->getComponent<CTransform>()->pos;
			end_origin.y = m_game.window().getDefaultView().getSize().y - end_origin.y;
			Vec2 end_bb = end_tile->getComponent<CBoundingBox>()->halfSize;


			Vec2 end_v1 = Vec2(end_origin.x - end_bb.x, end_origin.y - end_bb.y);
			Vec2 end_v2 = Vec2(end_origin.x + end_bb.x, end_origin.y - end_bb.y);
			Vec2 end_v3 = Vec2(end_origin.x + end_bb.x, end_origin.y + end_bb.y);
			Vec2 end_v4 = Vec2(end_origin.x - end_bb.x, end_origin.y + end_bb.y);

			for (int i = 0; i < 4; i++)
			{
				Vec2 vert;
				switch (i)
				{
				case 0:
					vert = end_v1;
					break;
				case 1:
					vert = end_v2;
					break;
				case 2:
					vert = end_v3;
					break;
				case 3:
					vert = end_v4;
					break;
				}

				for (auto & intersect_tile : m_entityManager.getEntities("Tile"))
				{
					if (intersect_tile->getComponent<CTransform>()->pos.dist(playPos) <  m_player->getComponent<CLight>()->dist)
					{
						Vec2 intersect_origin = intersect_tile->getComponent<CTransform>()->pos;
						intersect_origin.y = m_game.window().getDefaultView().getSize().y - intersect_origin.y;
						Vec2 intersect_bb = intersect_tile->getComponent<CBoundingBox>()->halfSize;

						Vec2 int_v1 = Vec2(intersect_origin.x - intersect_bb.x, intersect_origin.y - intersect_bb.y);
						Vec2 int_v2 = Vec2(intersect_origin.x + intersect_bb.x, intersect_origin.y - intersect_bb.y);
						Vec2 int_v3 = Vec2(intersect_origin.x + intersect_bb.x, intersect_origin.y + intersect_bb.y);
						Vec2 int_v4 = Vec2(intersect_origin.x - intersect_bb.x, intersect_origin.y + intersect_bb.y);

						if (Physics::LineIntersect(pPos, vert, int_v1, int_v2))
						{
							points[i] = false;
							break;
						}
						else if (Physics::LineIntersect(pPos, vert, int_v2, int_v3))
						{
							points[i] = false;
							break;
						}
						else if (Physics::LineIntersect(pPos, vert, int_v3, int_v4))
						{
							points[i] = false;
							break;
						}
						else if (Physics::LineIntersect(pPos, vert, int_v4, int_v1))
						{
							points[i] = false;
							break;
						}
					}
				}
			}
			/*	For every vert in a tile cast a point to the player. */

			for (int j = 0; j < 4; j++)
			{
				if (points[j])
				{
					Vec2 vert;
					switch (j)
					{
					case 0:
						vert = end_v1;
						break;
					case 1:
						vert = end_v2;
						break;
					case 2:
						vert = end_v3;
						break;
					case 3:
						vert = end_v4;
						break;
					}
					if (pPos.dist(vert) <= dist )
					{
						intersetions.push_back(vert - pPos);
					}
				}
			}

		}
	}


	/** Sort all points clockwise for triangle fan. **/
	std::sort(intersetions.begin(), intersetions.end());
	sf::VertexArray TriangleFan(sf::TriangleFan, intersetions.size() + 2);

	TriangleFan[0].position = sf::Vector2f(pPos.x, pPos.y);
	TriangleFan[0].color = sf::Color(255, 255, 210, 255);

	for (int i = 1; i < intersetions.size(); i += 1)
	{
		float point_to_player = 1 - pPos.dist(Vec2(intersetions[i].x + pPos.x, intersetions[i].y + pPos.y)) / m_player->getComponent<CLight>()->dist;
		TriangleFan[i].position = sf::Vector2f(intersetions[i].x + pPos.x, intersetions[i].y + pPos.y);
		TriangleFan[i].color = sf::Color(255 * point_to_player, 255 * point_to_player, 210 * point_to_player, 255 * point_to_player);
	}

	/** Connect the final points in TF **/

	float point_to_player = 1 - pPos.dist(Vec2(intersetions[0].x + pPos.x, intersetions[0].y + pPos.y)) / m_player->getComponent<CLight>()->dist;
	TriangleFan[intersetions.size()].position = sf::Vector2f(intersetions[0].x + pPos.x, intersetions[0].y + pPos.y);
	TriangleFan[intersetions.size()].color = sf::Color(255 * point_to_player, 255 * point_to_player, 195 * point_to_player, 255 * point_to_player);
	point_to_player = 1 - pPos.dist(Vec2(intersetions[1].x + pPos.x, intersetions[1].y + pPos.y)) / m_player->getComponent<CLight>()->dist;
	TriangleFan[intersetions.size() + 1].position = sf::Vector2f(intersetions[1].x + pPos.x, intersetions[1].y + pPos.y);
	TriangleFan[intersetions.size() + 1].color = sf::Color(255 * point_to_player, 255 * point_to_player, 195 * point_to_player, 255 * point_to_player);
	m_lightPoly.clear();
	m_lightPoly = TriangleFan;



}


/**
 * @brief      { The user input system.}
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
			case sf::Keyboard::F:       { m_drawCollision = !m_drawCollision; break; }
			case sf::Keyboard::P:       { setPaused(!m_paused);  break; }
			case sf::Keyboard::E:		{ useHealthKit(); break; }
			case sf::Keyboard::Q:		{ m_player->getComponent<CInventory>()->meleeSelected = !m_player->getComponent<CInventory>()->meleeSelected; break; }
			case sf::Keyboard::Space:
			{
				if (!m_player->getComponent<CInventory>()->meleeSelected && m_canShoot)     { spawnBullet(m_player); break;}
				else if (m_player->getComponent<CInventory>()->meleeSelected && m_canShoot) { spawnMeele(m_player); break;}
			}
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
	m_game.window().clear(sf::Color(185, 175, 175));
	m_background.clear(sf::Color(23, 23, 23));
	sf::View view(m_game.window().getDefaultView());

	/* use center of view to position hp and item counts/selection */
	auto camPos = m_player->getComponent<CTransform>()->pos;
	auto window = m_game.window().getDefaultView().getSize();

	/* Set camera to follow player */
	view.setCenter(m_player->getComponent<CTransform>()->pos.x, m_game.window().getDefaultView().getSize().y - m_player->getComponent<CTransform>()->pos.y);

	m_game.window().setView(view);
	//m_background.setView(view);

	std::string conString = "HP: " + std::to_string(m_player->getComponent<CHealth>()->hp);//"Current HP: " + m_player->getComponent<CHealth>()->hp;
	m_currentHP.setPosition(camPos.x - (window.x / 2.1) , (window.y / 2) - camPos.y );
	m_currentHP.setString(conString);

	m_ammoCount.setPosition(camPos.x - (window.x / 2.4), (window.y / 2) - camPos.y);
	conString = "Ammo: " + std::to_string(m_player->getComponent<CInventory>()->ammo);
	m_ammoCount.setString(conString);

	m_hpKitCount.setPosition(camPos.x - (window.x / 2.9), (window.y / 2) - camPos.y);
	conString = "Med kits: " + std::to_string(m_player->getComponent<CInventory>()->numOfHealthKits);
	m_hpKitCount.setString(conString);

	m_weaponSelected.setPosition(camPos.x - (window.x / 4), (window.y / 2) - camPos.y);
	if (m_player->getComponent<CInventory>()->meleeSelected)
	{
		conString = "Weapon Selected: Pipe" ;
	}
	else
	{
		conString = "Weapon Selected: Gun";
	}
	m_weaponSelected.setString(conString);

	/* draw all Entity textures / animations */

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



	// draw all Entity collision bounding boxes with a rectangleshape
	if (m_drawCollision)
	{
		for (auto l : m_Light_Lines)
		{
			m_game.window().draw(l);
		}

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
				dot.setPosition(e->getComponent<CFollowPlayer>()->home.x, m_game.window().getDefaultView().getSize().y - e->getComponent<CFollowPlayer>()->home.y);
				m_game.window().draw(dot);
			}
		}


	}

	m_background.setView(view);
	m_background.draw(m_lightPoly);


	m_background.display();

	/** Use are render texture to  Blend triangle fan creating a mask for the light.**/
	const sf::Texture& texture = m_background.getTexture();
	sf::Sprite sprite(texture);
	sprite.setPosition(m_player->getComponent<CTransform>()->pos.x - m_game.window().getSize().x / 2, m_game.window().getSize().y / 2 - m_player->getComponent<CTransform>()->pos.y);


	m_game.window().draw(sprite, sf::BlendMultiply);

	m_game.window().draw(m_currentHP);
	m_game.window().draw(m_ammoCount);
	m_game.window().draw(m_hpKitCount);
	m_game.window().draw(m_weaponSelected);

	m_game.window().display();
}
