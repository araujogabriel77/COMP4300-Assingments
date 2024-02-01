#include "Game.h"

#include <iostream>
#include <fstream>
#include <random>
#include <cmath>

#define PI 3.14

Game::Game(const std::string& config)
{
	init(config);
}

void Game::init(const std::string& path)
{
	// TODO: read in config file here
	// use the premade PlayerConfig, EnemyConfig, BulletConfig variables
	std::ifstream fin(path);
  std::string configType;
  int window_width, window_height, frameLimit, isFullScreen;

  while (fin >> configType)
  {
    if (configType == "Window")
    {
      fin >> window_width >> window_height >> frameLimit >> isFullScreen;
    }

    if (configType == "Font")
    {
      std::string type, fontFile;
      int size, r, g, b;
      fin >> fontFile >> size >> r >> g >> b;

      if (!m_font.loadFromFile(fontFile))
      {
        std::cerr << "Could not load font\n";
        exit(-1);
      }
      m_text.setCharacterSize(size);
      std::cout << "Font loaded!\n";
    }

    if (configType == "Player")
    {
      // struct PlayerConfig { int SR, CR, FR, FG, FB, OR, OG, OB, OT, V; float S; };
      fin >> m_playerConfig.SR >> m_playerConfig.CR >> m_playerConfig.S >> m_playerConfig.FR >> m_playerConfig.FG >> m_playerConfig.FB >> m_playerConfig.OR >> m_playerConfig.OG >> m_playerConfig.OB >> m_playerConfig.OT >> m_playerConfig.V;
    }
    if (configType == "Enemy")
    {
      // struct EnemyConfig { int SR, CR, OR, OG, OB, OT, VMIN, VMAX, L, SI; float SMIN, SMAX; };
      fin >> m_enemyConfig.SR >> m_enemyConfig.CR >> m_enemyConfig.SMIN >> m_enemyConfig.SMAX >> m_enemyConfig.OR >> m_enemyConfig.OG >> m_enemyConfig.OB >> m_enemyConfig.OT >> m_enemyConfig.VMIN >> m_enemyConfig.VMAX >> m_enemyConfig.L >> m_enemyConfig.SI;
    }
    if (configType == "Bullet")
    {
      // struct BulletConfig { int SR, CR, FR, FG, FB, OR, OG, OB, OT, V, L; float S; };
      fin >> m_bulletConfig.SR >> m_bulletConfig.CR >> m_bulletConfig.S >> m_bulletConfig.FR >> m_bulletConfig.FG >> m_bulletConfig.FB >> m_bulletConfig.OR >> m_bulletConfig.OG >> m_bulletConfig.OB >> m_bulletConfig.OT >> m_bulletConfig.V >> m_bulletConfig.L;
    }
  }

  // set up default window parameters
  m_window.create(sf::VideoMode(window_width, window_height), "Assignment 2");
  m_window.setFramerateLimit(frameLimit);
  m_window.setVerticalSyncEnabled(true);

  spawnPlayer();
}

void Game::run()
{
  // TODO: add pause functionality in here
  //  some systems should function while paused (rendering)
  //  some systems shouldn't (movement / input)

  while (m_running)
  {
    m_entities.update();

    sUserInput();
    sCollision();
    sRender();

    if (!m_paused)
    {
      sMovement();
      sEnemySpawner();
      m_currentFrame++;
    }

    // increment the current frame
    // may need to be moved when pause implemented
  }
}

void Game::setPaused()
{
  m_paused = !m_paused;
}

// respawin the player in the middle of the screen
void Game::spawnPlayer()
{
	// TODO: Finish adding all properties of the player with the correct values from the config

	// we create every entity by calling EntityManger.addEntity(tag)
	// This returns a std::shared_ptr<Entity>, so we use 'auto' to save typing
	auto entity = m_entities.addEntity("player");

	// Give this entity a Transform so it spawns at (200, 200) with velocity (1,1) and angle 0
	entity->cTransform = std::make_shared<CTransform>(Vec2(200.0f, 200.0f), Vec2(1.0f, 1.0f), 0.0f);

	// The entity's shape will have radius 32, 8 sides, dark grey fill, and red outline of thickness 4
  entity->cShape = std::make_shared<CShape>(m_playerConfig.SR, m_playerConfig.V, sf::Color(m_playerConfig.FR, m_playerConfig.FG, m_playerConfig.FB), sf::Color(m_playerConfig.OR, m_playerConfig.OG, m_playerConfig.OB), m_playerConfig.OT);

  // Add an input component to the plaer so that we can use inputs
  entity->cInput = std::make_shared<CInput>();

  // Since we want this Entity to be our player, set our Game's player variable to be this Entioty
  // This goes slightly against the EntityManager paradim, but we use the player so much it's worth it
  m_player = entity;
}

// spawn an enemy at a random position
void Game::spawnEnemy()
{
	// TODO: make sure the enemy is spawned properly with te m_enemyConfig variables
	//		 the enemy must be spawned completely within the bounds of the window

  // exemplo
  auto entity = m_entities.addEntity("Enemy");

  float ex = rand() % m_window.getSize().x;
  float ey = rand() % m_window.getSize().y;

  entity->cTransform = std::make_shared<CTransform>(Vec2(ex, ey), Vec2(0.0f, 0.0f), 0.0f);
  entity->cShape = std::make_shared<CShape>(16.f, 3, sf::Color(0, 0, 255), sf::Color(255, 255, 255), 4.0f);

  // record when most recent enemy was spawned
  m_lastEnemySpawnTime = m_currentFrame;
}

// spawns  te small enemies when a big one (input entity e) explodes
void Game::spawnSmallEnemies(std::shared_ptr<Entity> e)
{
	// TODO: spawn small enemies at the location of the input enemy e

	// when we create the smaller enemy, we have to read the values of the original enemy
	// - spawn a number of small enemies equal to the verticles of the original enemy
	// - set each small enemy to the same color as the original, half the size
	// - small enemies are worth double points of the original enemy
}

// spawns a bullet from a given entity to a target location
void Game::spawnBullet(std::shared_ptr<Entity> entity, const Vec2& target)
{
  auto bullet = m_entities.addEntity("bullet");

  // The entity's shape will have radius 32, 8 sides, dark grey fill, and red outline of thickness 4
  bullet->cShape = std::make_shared<CShape>(
      m_bulletConfig.SR,
      m_bulletConfig.V,
      sf::Color(m_bulletConfig.FR, m_bulletConfig.FG, m_bulletConfig.FB),
      sf::Color(m_bulletConfig.OR, m_bulletConfig.OG, m_bulletConfig.OB),
      m_playerConfig.OT);

  Vec2 difference{target.x - entity->cTransform->pos.x, target.y - entity->cTransform->pos.y};

  difference.normalize();

  Vec2 velocity{m_bulletConfig.S * difference.x, m_bulletConfig.S * difference.y};

  bullet->cTransform = std::make_shared<CTransform>(entity->cTransform->pos, velocity, 0);
}

void Game::spawnSpecialWeapon(std::shared_ptr<Entity> entity)
{
  // TODO: implement your own special weapon
}

void Game::sMovement()
{
  // TODO: implement all entity movement in this function
  //			you should read the m_player->cInput component to determine if the player is moving

  m_player->cTransform->velocity = {0, 0};

  // implement player movement
  if (m_player->cInput->up)
  {
    m_player->cTransform->velocity.y = -5;
  }
  if (m_player->cInput->down)
  {
    m_player->cTransform->velocity.y = 5;
  }
  if (m_player->cInput->left)
  {
    m_player->cTransform->velocity.x = -5;
  }
  if (m_player->cInput->right)
  {
    m_player->cTransform->velocity.x = 5;
  }
  for (auto e : m_entities.getEntities())
  {
    if (e->tag() == "player")
    {
      m_player->cTransform->pos += m_player->cTransform->velocity;
      e->cTransform->angle += 2.0f;
      e->cShape->circle.setRotation(e->cTransform->angle);
    }
    else if (e->cTransform)
    {
      e->cTransform->pos += e->cTransform->velocity;
      e->cTransform->angle += 2.0f;
      e->cShape->circle.setRotation(e->cTransform->angle);
    }
  }
}

void Game::sLifespan()
{
  // TODO: implement all lifespan functionality
  //
  // for all entityes
  //		if entity has no lifespan component, skip it
  //		if entity has > 0 remaining lifespan, subtract 1
  //		if it has lifespan and is alive
  //			scale its alpha channel properly
  //		if it has lifespan and its time is up
  //			destroy the entity
}

void Game::sCollision()
{
  // TODO: implement all proper collision between entities
  //		 be sure to use the collision radius, NOT the shape radius

  for (auto b : m_entities.getEntities("bullet"))
  {
    for (auto e : m_entities.getEntities("enemy"))
    {
      if (
          b->cTransform->pos.x + b->cCollision->radius == e->cTransform->pos.x - e->cCollision->radius && b->cTransform->pos.y + b->cCollision->radius == e->cTransform->pos.y - e->cCollision->radius || b->cTransform->pos.x - b->cCollision->radius == e->cTransform->pos.x + e->cCollision->radius && b->cTransform->pos.y - b->cCollision->radius == e->cTransform->pos.y + e->cCollision->radius)
      {
        std::cout << "collision!" << std::endl;
      }
    }
  }
}

void Game::sEnemySpawner()
{
  // TODO: code wich implements enemy spawning should go there
  //
  //		(use m_currentFrame - m_lastEnemtSpawnTime) to determine
  //		how long it has been since the last enemy spawned
  if (m_currentFrame - m_lastEnemySpawnTime > 90)
  {
    spawnEnemy();
  }
}

void Game::sRender()
{
  // TODO: change the code below to draw ALL of the entities
  //		sample drawing of the player Entity that we have created
  m_window.clear();

  // set the position of the shape based on the entity's transform->pos
  m_player->cShape->circle.setPosition(m_player->cTransform->pos.x, m_player->cTransform->pos.y);

  // set the rotation of the shape based on the entity's transform->angle
  m_player->cTransform->angle += 1.0f;
  m_player->cShape->circle.setRotation(m_player->cTransform->angle);

  // draw the entity's sf::CirclShape
  m_window.draw(m_player->cShape->circle);

  for (auto e : m_entities.getEntities())
  {
    // if (e->tag() != "player")
    // {
    //   // e->cShape->circle.setPosition(e->cTransform->pos.x, e->cTransform->pos.y);
    //   // e->cTransform->angle += 1.0f;

    // }
    e->cShape->circle.setRotation(e->cTransform->angle);
    float xPos, yPos;
    xPos = e->cTransform->pos.x + e->cTransform->velocity.x;
    yPos = e->cTransform->pos.y + e->cTransform->velocity.y;
    e->cShape->circle.setPosition(xPos, yPos);

    std::cout << "angle :" << e->cTransform->angle << " velocity x: " << e->cTransform->velocity.x << " velocity y: " << e->cTransform->velocity.y << std::endl;
    m_window.draw(e->cShape->circle);
  }

  for (auto e : m_entities.getEntities())
  {
    e->cShape->circle.setPosition(e->cTransform->pos.x, e->cTransform->pos.y);

    e->cTransform->angle += 1.0f;
    e->cShape->circle.setRotation(e->cTransform->angle);

    m_window.draw(e->cShape->circle);
  }

  m_window.display();
}

void Game::sUserInput()
{
	// TODO: handle user input here
	//		note that you should only be setting the player's input component variables here
	//		you should not implement the player's movement logic here
	//		the movement system will read the variables you set in this function

	sf::Event event;
	while (m_window.pollEvent(event))
	{
		// this event triggers when te window is closed
		if (event.type == sf::Event::Closed)
		{
			m_running = false;
		}

		// this event is triggered when a key is pressed
		if (event.type == sf::Event::KeyPressed)
		{
			switch (event.key.code)
			{
			case sf::Keyboard::W:
				m_player->cInput->up = true;
				std::cout << "W key Pressed\n";
        break;
      case sf::Keyboard::S:
        m_player->cInput->down = true;
        std::cout << "S key Pressed\n";
        break;
      case sf::Keyboard::D:
        m_player->cInput->right = true;
        std::cout << "D key Pressed\n";
        break;
      case sf::Keyboard::A:
        m_player->cInput->left = true;
        std::cout << "A key Pressed\n";
        break;
      case sf::Keyboard::P:
        setPaused();
        std::cout << "P key Pressed\n";
        break;
      default:
        break;
      }
    }

    // this event is triggered when a key is released
    if (event.type == sf::Event::KeyReleased)
    {
      switch (event.key.code)
      {
      case sf::Keyboard::W:
        m_player->cInput->up = false;
        std::cout << "W key Released\n";
        break;
      case sf::Keyboard::S:
        m_player->cInput->down = false;
        std::cout << "S key Released\n";
        break;
      case sf::Keyboard::D:
        m_player->cInput->right = false;
        std::cout << "D key Released\n";
        break;
      case sf::Keyboard::A:
        m_player->cInput->left = false;
        std::cout << "A key Released\n";
        break;
      default:
        break;
      }
    }

    if (event.type == sf::Event::MouseButtonPressed)
    {
      if (event.mouseButton.button == sf::Mouse::Left)
      {
        std::cout << "Left mouse button clicked at (" << event.mouseButton.x << "," << event.mouseButton.y << ")\n";
        spawnBullet(m_player, Vec2(event.mouseButton.x, event.mouseButton.y));
      }
    }
  }
}