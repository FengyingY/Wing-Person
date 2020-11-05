#include "PuzzleStage.hpp"

Player::Player(Shapes::Rectangle bounds,
    Input* left, Input* right, Input* jump) : bounding_box(bounds)
                                            , _left(left)
                                            , _right(right)
                                            , _jump(jump) {
}

Platform::Platform(Shapes::Rectangle rectangle) : bounding_box(rectangle) {}

PuzzleStage::PuzzleStage() {
  // TODO
  // default 2 players and a simple platform for now
  add_player(Shapes::Rectangle(glm::vec2(200, 200), 50, 80), SDLK_a, SDLK_d, SDLK_w);
  add_player(Shapes::Rectangle(glm::vec2(600, 200), 50, 80), SDLK_LEFT, SDLK_RIGHT, SDLK_UP);

  _platforms.emplace_back(Platform(Shapes::Rectangle(glm::vec2(400, 140), 700, 40)));
}

PuzzleStage::~PuzzleStage() {}

void PuzzleStage::add_player(Shapes::Rectangle bounds,
    SDL_Keycode leftkey, SDL_Keycode rightkey, SDL_Keycode jumpkey) {
  Input* left = _input_manager.register_key(leftkey);
  Input* right = _input_manager.register_key(rightkey);
  Input* jump = _input_manager.register_key(jumpkey);
  _players.emplace_back(Player(bounds, left, right, jump));
}

void PuzzleStage::tick(float elapsed) {
  // Calculate inputs and movement for each player
  for (auto& player : _players) {
    if(player._left->held()) {
      player._velocity.x -= Player::movespeed * elapsed;
    }

    if(player._right->held()) {
      player._velocity.x += Player::movespeed * elapsed;
    }

    if(player._jump->pressed() && !player._falling && !player._jump_input) {
       player._jump_input = true;
       if(player._input_jump_time < Player::max_jump_time) {
         player._input_jump_time += elapsed;
         if(player._input_jump_time >= Player::max_jump_time) {
           player._input_jump_time = Player::max_jump_time;
         }
       }
    }

    if(player._jump->released()) {
       player._jump_input = false;
       if(player._input_jump_time < Player::min_jump_time) {
         player._input_jump_time = Player::min_jump_time;
       }
    }

    // Process jumping
    if(player._cur_jump_time < player._input_jump_time) {
      player._cur_jump_time += elapsed;
      player._velocity.y += Player::jumpspeed * elapsed;

      if(player._cur_jump_time >= player._input_jump_time) {
        player._cur_jump_time = 0.0f;
        player._input_jump_time = 0.0f;
      }
    }

    // Add gravity
    if(player._falling) {
      player._velocity.y -= Player::gravityspeed * elapsed;
    }
  }
}
