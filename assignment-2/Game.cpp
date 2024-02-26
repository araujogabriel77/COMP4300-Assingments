#include "Game.h"

#include <iostream>
#include <fstream>
#include <random>
#include <cmath>

const int PLAYER_SPEED = 5;

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

  std::cout << "Loading configuration...\n";
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
      std::cout << "Player configuration loaded!\n";
    }
    if (configType == "Enemy")
    {
      // struct EnemyConfig { int SR, CR, OR, OG, OB, OT, VMIN, VMAX, L, SI; float SMIN, SMAX; };
      fin >> m_enemyConfig.SR >> m_enemyConfig.CR >> m_enemyConfig.SMIN >> m_enemyConfig.SMAX >> m_enemyConfig.OR >> m_enemyConfig.OG >> m_enemyConfig.OB >> m_enemyConfig.OT >> m_enemyConfig.VMIN >> m_enemyConfig.VMAX >> m_enemyConfig.L >> m_enemyConfig.SI;
      std::cout << "Enemy configuration loaded!\n";
    }
    if (configType == "Bullet")
    {
      // struct BulletConfig { int SR, CR, FR, FG, FB, OR, OG, OB, OT, V, L; float S; };
      fin >> m_bulletConfig.SR >> m_bulletConfig.CR >> m_bulletConfig.S >> m_bulletConfig.FR >> m_bulletConfig.FG >> m_bulletConfig.FB >> m_bulletConfig.OR >> m_bulletConfig.OG >> m_bulletConfig.OB >> m_bulletConfig.OT >> m_bulletConfig.V >> m_bulletConfig.L;
      std::cout << "Bullet configuration loaded!\n";
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
    sRender();

    if (!m_paused)
    {
      sCollision();
      sMovement();
      sEnemySpawner();
      sLifespan();
      sScore();
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

void Game::sScore()
{
  auto scoreText = "Score: " + std::to_string(m_score);
  m_text = sf::Text(scoreText, m_font);
  m_text.setPosition(10, 10);
}

// respawn the player in the middle of the screen
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
  auto entity = m_entities.addEntity("enemy");

  float ex = rand() % m_window.getSize().x;
  float ey = rand() % m_window.getSize().y;
  float targetX = rand() % m_window.getSize().x;
  float targetY = rand() % m_window.getSize().y;
  int red = 50 + rand() % 255;
  int green = 50 + rand() % 255;
  int blue = 50 + rand() % 255;

  int vertices = 3 + rand() % 7;
  int enemySpeed = m_enemyConfig.VMIN + rand() % (m_enemyConfig.VMAX - m_enemyConfig.VMIN);
  
  Vec2 difference{targetX - ex, targetY - ey};
  difference.normalize();
  Vec2 velocity{enemySpeed * difference.x, enemySpeed * difference.y};

  entity->cTransform = std::make_shared<CTransform>(Vec2(ex, ey), velocity, 0.0f);
  entity->cShape = std::make_shared<CShape>(m_enemyConfig.SR, vertices, sf::Color(red, green, blue), sf::Color(255, 255, 255), m_enemyConfig.OT);
  entity->cCollision = std::make_shared<CCollision>(m_enemyConfig.CR);

  // record when most recent enemy was spawned
  m_lastEnemySpawnTime = m_currentFrame;
}

// spawns  te small enemies when a big one (input entity e) explodes
void Game::spawnSmallEnemies(std::shared_ptr<Entity> e)
{
	// TODO: spawn small enemies at the location of the input enemy e
  int vertices = e->cShape->circle.getPointCount();
	float speed = e->cTransform->velocity.dist(e->cTransform->pos);

  for(int i = 0; i < vertices; i++)
  {
    float angle = 360 / vertices * i;
    float angleInRadians = 3.14f * angle / 180;
    Vec2 velocity = Vec2(2*(float)cos(angleInRadians), 2*(float)sin(angleInRadians));

    auto entity = m_entities.addEntity("smallEnemy");

    entity->cTransform = std::make_shared<CTransform>(e->cTransform->pos, velocity, angle);

    entity->cShape = std::make_shared<CShape>(e->cShape->circle.getRadius() / 2, vertices, e->cShape->circle.getFillColor(), e->cShape->circle.getOutlineColor(), 4.0f);
    
    entity->cCollision = std::make_shared<CCollision>(e->cCollision->radius / 2);

    entity->cLifeSpan = std::make_shared<CLifespan>(m_enemyConfig.L);
  }
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
  bullet->cCollision = std::make_shared<CCollision>(m_bulletConfig.CR);
  bullet->cLifeSpan = std::make_shared<CLifespan>(m_bulletConfig.L);
}

void Game::spawnSpecialWeapon(std::shared_ptr<Entity> entity)
{
  for(int i = 0; i <= 360; i++) {
    spawnBullet(entity, Vec2(entity->cTransform->pos.x + 10 * cos(i), entity->cTransform->pos.y + 10 * sin(i)));
  }
  m_specialWeaponCharge = 0;
}

void Game::sMovement()
{
  // TODO: implement all entity movement in this function
  //			you should read the m_player->cInput component to determine if the player is moving

  m_player->cTransform->velocity = {0, 0};

  // implement player movement
  float playerRadius = m_player->cShape->circle.getRadius();
  if (m_player->cInput->up && (m_player->cTransform->pos.y - playerRadius) > PLAYER_SPEED)
  {
      m_player->cTransform->velocity.y = -PLAYER_SPEED;
  }
  if (m_player->cInput->down && (m_player->cTransform->pos.y + playerRadius) < m_window.getSize().y - PLAYER_SPEED)
  {
    m_player->cTransform->velocity.y = PLAYER_SPEED;
  }
  if (m_player->cInput->left && (m_player->cTransform->pos.x - playerRadius) > PLAYER_SPEED)
  {
    m_player->cTransform->velocity.x = -PLAYER_SPEED;
  }
  if (m_player->cInput->right && (m_player->cTransform->pos.x + playerRadius) < m_window.getSize().x - PLAYER_SPEED)
  {
    m_player->cTransform->velocity.x = PLAYER_SPEED;
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
  for(auto e : m_entities.getEntities())
  {
    if(e->cLifeSpan)
    {
      if(e->cLifeSpan->remaining > 0)
      {
        e->cLifeSpan->remaining--;
      }
      if(e->cLifeSpan->remaining > 0)
      {
        auto color = sf::Color(e->cShape->circle.getFillColor().r, e->cShape->circle.getFillColor().g, e->cShape->circle.getFillColor().b, 255 * e->cLifeSpan->remaining / m_enemyConfig.L);
        e->cShape->circle.setFillColor(color);
        e->cShape->circle.setOutlineColor(color);
      }
      if(e->cLifeSpan->remaining == 0)
      {
        e->destroy();
      }
    }
  }
}

void Game::sCollision()
{
  for (auto b : m_entities.getEntities("bullet"))
  {
    for (auto e : m_entities.getEntities("enemy"))
    {
      float dist;
      dist = b->cTransform->pos.dist(e->cTransform->pos);

      if (dist < e->cCollision->radius + b->cCollision->radius)
      {
        b->destroy();
        e->destroy();
        m_score ++;
        if(m_specialWeaponCharge < 100)
        {
          m_specialWeaponCharge ++;
        }
        spawnSmallEnemies(e);
      }
    }

    for (auto e : m_entities.getEntities("smallEnemy"))
    {
      float dist;
      dist = b->cTransform->pos.dist(e->cTransform->pos);

      if (dist < e->cCollision->radius + b->cCollision->radius)
      {
        b->destroy();
        e->destroy();
        m_score += 2;
        m_specialWeaponCharge += 2;
      }
    }

    if(b->cTransform->pos.x < 0 || b->cTransform->pos.x > m_window.getSize().x || b->cTransform->pos.y < 0 || b->cTransform->pos.y > m_window.getSize().y)
    {
      b->destroy();
    }
  }
}

void Game::sEnemySpawner()
{
  if (m_currentFrame - m_lastEnemySpawnTime > 90 && m_entities.getEntities("enemy").size() < 10)
  {
    spawnEnemy();
  }
}

void Game::sRender()
{
  m_window.clear();

  // set the position of the shape based on the entity's transform->pos
  m_player->cShape->circle.setPosition(m_player->cTransform->pos.x, m_player->cTransform->pos.y);

  // set the rotation of the shape based on the entity's transform->angle
  m_player->cTransform->angle += 1.0f;
  m_player->cShape->circle.setRotation(m_player->cTransform->angle);

  // draw the entity's sf::CircleShape
  m_window.draw(m_player->cShape->circle);
  m_window.draw(m_text);

  for (auto e : m_entities.getEntities())
  {
    e->cShape->circle.setRotation(e->cTransform->angle);
    float xPos, yPos;
    xPos = e->cTransform->pos.x + e->cTransform->velocity.x;
    yPos = e->cTransform->pos.y + e->cTransform->velocity.y;

    if(e->tag() == "enemy" || e->tag() == "smallEnemy")
    {
      if (xPos < 0 || xPos > m_window.getSize().x)
      {
        e->cTransform->velocity.x *= -1;
      }
      if (yPos < 0 || yPos > m_window.getSize().y)
      {
        e->cTransform->velocity.y *= -1;
      }
    }
    e->cShape->circle.setPosition(xPos, yPos);

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
        break;
      case sf::Keyboard::S:
        m_player->cInput->down = true;
        break;
      case sf::Keyboard::D:
        m_player->cInput->right = true;
        break;
      case sf::Keyboard::A:
        m_player->cInput->left = true;
        break;
      case sf::Keyboard::P:
        setPaused();
        break;
      case sf::Keyboard::Space:
        spawnBullet(m_player, Vec2(sf::Mouse::getPosition(m_window).x, sf::Mouse::getPosition(m_window).y));
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
        break;
      case sf::Keyboard::S:
        m_player->cInput->down = false;
        break;
      case sf::Keyboard::D:
        m_player->cInput->right = false;
        break;
      case sf::Keyboard::A:
        m_player->cInput->left = false;
        break;
      case sf::Keyboard::X:
        if(m_specialWeaponCharge >= 100) {
          spawnSpecialWeapon(m_player);
        }
        break;
      case sf::Keyboard::Space:
        spawnBullet(m_player, Vec2(sf::Mouse::getPosition(m_window).x, sf::Mouse::getPosition(m_window).y));
      default:
        break;
      }
    }

    if (event.type == sf::Event::MouseButtonPressed)
    {
      if (event.mouseButton.button == sf::Mouse::Left)
      {
        spawnBullet(m_player, Vec2(event.mouseButton.x, event.mouseButton.y));
      }
    }
  }
}