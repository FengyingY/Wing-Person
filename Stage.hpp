#pragma once

#include "Shapes.hpp"
#include "Input.hpp"
#include "Player.hpp"
#include "Platform.hpp"

#include <vector>

//TODO move over to shapes
struct Shape {
  int shape;
  union {
    Shape::Circle circle;
    Shape::Rectangle rectangle;
    Shape::Triangle triangle;
  } active_shape;
};

class Platform {
  friend class PuzzleStage;

  public:
    Platform(Shapes::Rectangle rectangle);

    Shape bounding_box;
};

class Player {
  friend class PuzzleStage;

  public:
    Shapes::Rectangle bounding_box;

  private:
    Player(Shapes::Rectangle bounds, Input* left, Input* right, Input* jump);

    glm::vec2 _velocity;

    Input* _left;
    Input* _right;
    Input* _jump;

    // Constants
    static constexpr float movespeed = 10.0f;
    static constexpr float jumpspeed = 20.0f;
    static constexpr float gravityspeed = 10.0f;

    static constexpr float min_jump_time = 0.3f;
    static constexpr float max_jump_time = 0.6f;

    float _input_jump_time = 0.0f;
    float _cur_jump_time = 0.0f;

    bool _jump_input = false;
    bool _falling = false;
};

class PuzzleStage {
  public:
    PuzzleStage();
    ~PuzzleStage();

    void add_player(Shapes::Rectangle bounds, SDL_Keycode leftkey, SDL_Keycode rightkey, SDL_Keycode jumpkey);

    void tick(float elapsed);

  private:
    InputManager _input_manager;

    std::vector<Player> _players;
    std::vector<Platform> _platforms;
};
