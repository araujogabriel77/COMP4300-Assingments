#include <string>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <memory>

int WIND0W_WIDTH;
int WIND0W_HEIGHT;
sf::Font font;
sf::Color fontColor;
int fontSize;

class Rectangle {
   void make_shape(sf::Vector2f size)
  {
    sf::RectangleShape temp(size);
    this->rectangle = temp;
    this->rectangle.setPosition(posX, posY);
    this->set_text_position();
  }
  public:
  sf::RectangleShape rectangle;
  sf::Text text;
  float x_speed, y_speed;
  int posX, posY;
  sf::Vector2f size;

    Rectangle(std::string text, sf::Vector2f size, int x_pos, int y_pos, float x_speed, float y_speed):
    text(text, font, fontSize),
    posX(x_pos),
    posY(y_pos),
    x_speed(x_speed),
    y_speed(y_speed),
    size(size)
    {
      make_shape(size);
    }

  void update_position()
  {
    sf::Vector2f pos = this->rectangle.getPosition();
    this->rectangle.setPosition(pos.x + this->x_speed, pos.y + this->y_speed);
    this->set_text_position();
  }

  void set_text_position()
  {
    sf::FloatRect textBounds = this->text.getLocalBounds();
    sf::Vector2f pos = this->rectangle.getPosition();
    
    float textPosX = pos.x + ((this->size.x / 2) - (textBounds.width / 2));
    float textPosY = pos.y + ((this->size.y / 2) - (textBounds.height / 2));
    this->text.setPosition(textPosX, textPosY);
  }

  sf::Vector2f get_position()
  {
    return this->rectangle.getPosition();
  }

  void set_color(int red, int green, int blue)
  {
    sf::Color color(red, green,blue);
    this->rectangle.setFillColor(color);
  }

  void inverse_x_speed()
  {
    this->x_speed = this->x_speed * -1;
  }

  void inverse_y_speed()
  {
    this->y_speed = this->y_speed * -1;
  }
};

class Circle {
  void make_shape(float radius)
  {
    sf::CircleShape temp(radius);
    this->circle = temp;
    
    this->circle.setPosition(posX, posY);
    this->set_text_position();
  }
  public:
  sf::CircleShape circle;
  sf::Text text;
  float x_speed, y_speed;
  int posX, posY;
  float radius;

    Circle(std::string text, float radius, int x_pos, int y_pos, float x_speed, float y_speed):
    text(text, font, fontSize),
    posX(x_pos),
    posY(y_pos),
    x_speed(x_speed),
    y_speed(y_speed),
    radius(radius)
    {
      make_shape(radius);
    }

    ~Circle(){}

  void update_position()
  {
    sf::Vector2f pos = this->circle.getPosition();
    this->circle.setPosition(pos.x + this->x_speed, pos.y + this->y_speed);
    this->set_text_position();
  }

  void set_text_position()
  {
    sf::FloatRect textBounds = this->text.getLocalBounds();
    sf::Vector2f pos = this->circle.getPosition();
    float textPosX = pos.x + (this->radius - (textBounds.width / 2));
    float textPosY = pos.y + (this->radius - (textBounds.height / 2));
    this->text.setPosition(textPosX, textPosY);
  }

  sf::Vector2f get_position()
  {
    return this->circle.getPosition();
  }

  void set_color(int red, int green, int blue)
  {
    sf::Color color(red, green,blue);
    this->circle.setFillColor(color);
  }

  void inverse_x_speed()
  {
    this->x_speed = this->x_speed * -1;
  }

  void inverse_y_speed()
  {
    this->y_speed = this->y_speed * -1;
  }
};


std::vector<std::shared_ptr<Circle>> circles;
std::vector<std::shared_ptr<Rectangle>> rectangles;

const float GRAVITY = 0.1f;
const float MAX_CIRCLE_RADIUS = 15.0f;

void readConfig(const std::string& filename) {
  std::ifstream fin(filename);
  std::string type;
  std::vector<std::string> circleArray;
  std::vector<std::string> rectangleArray;

  while(fin >> type) {
    if(type == "Window")
    {
      fin >> WIND0W_WIDTH >> WIND0W_HEIGHT;
      std::cout << WIND0W_WIDTH << "\n" << WIND0W_HEIGHT << "\n";
    }
    if(type == "Font")
    {
      std::string type, fontFile;
      int size, r, g, b;
      fin >> fontFile >> size >> r >> g >> b;
      
      if (!font.loadFromFile(fontFile))
      {
        std::cerr << "Could not load font\n";
        exit(-1);
      }

      fontColor = sf::Color(r, g ,b);
      fontSize = size;
      std::cout << "Font loaded!\n";
    }

    if(type == "Circle") {
      std::string shapeName;
      float posX, posY, speedX, speedY, radius;
      int r, g, b;
      fin >> shapeName >> posX >> posY >> speedX >> speedY >> r >> g >> b >> radius;

      std::shared_ptr<Circle> circle = std::make_shared<Circle>(shapeName, radius, posX, posY, speedX, speedY);
    
      circle->set_color(r, g, b);
      circles.push_back(circle);
    }

    if(type == "Rectangle") {
      std::string shapeName;
      float posX, posY, speedX, speedY, sizeW, sizeH;
      int r, g, b;
      fin >> shapeName >> posX >> posY >> speedX >> speedY >> r >> g >> b >> sizeW >> sizeH;
      sf::Vector2f rectangleSize = sf::Vector2f(sizeW, sizeH);
      std::shared_ptr<Rectangle> rectangle = std::make_shared<Rectangle>(shapeName, rectangleSize, posX, posY, speedX, speedY);
    
      rectangle->set_color(r, g, b);
      rectangles.push_back(rectangle);
    }
  }

  for (auto circle : circleArray) {
    std::cout << circle << "\n";
  }

  for (auto rectangle : rectangleArray) {
    std::cout << rectangle << "\n";
  }
}

int main () {
  readConfig("config.txt");
  sf::RenderWindow window(sf::VideoMode(WIND0W_WIDTH, WIND0W_HEIGHT), "SFML window");
  window.setFramerateLimit(60);
 
    // Start the game loop
    while (window.isOpen())
    {
        // Process events
        sf::Event event;
        while (window.pollEvent(event))
        {
            // Close window: exit
            if (event.type == sf::Event::Closed)
                window.close();
        }
 
        // Clear screen
        window.clear();
 
        for(auto& circle : circles) 
        {
          sf::Vector2f pos = circle->circle.getPosition();
          if(pos.x <= 0 || pos.x + (circle->radius * 2) >= WIND0W_WIDTH) {
            circle->inverse_x_speed();
          }
          if(pos.y <= 0 || pos.y + (circle->radius * 2) >= WIND0W_HEIGHT) {
            circle->inverse_y_speed();
          }

          if(circle->circle.getPosition().y <= 0) {
            circle->inverse_x_speed();
          }
          circle->update_position();
          window.draw(circle->circle);
          window.draw(circle->text);
        }

        for(auto& rectangle : rectangles) 
        {
          sf::Vector2f pos = rectangle->rectangle.getPosition();
          if(pos.x <= 0 || pos.x + rectangle->size.x >= WIND0W_WIDTH) {
            rectangle->inverse_x_speed();
          }
          if(pos.y <= 0 || pos.y + rectangle->size.y >= WIND0W_HEIGHT) {
            rectangle->inverse_y_speed();
          }

          rectangle->update_position();
          window.draw(rectangle->rectangle);
          window.draw(rectangle->text);
        }
 
        // Update the window
        window.display();
    }

    return EXIT_SUCCESS;
}