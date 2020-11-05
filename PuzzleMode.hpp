#pragma once

#include "Mode.hpp"

#include "Input.hpp"
#include "Player.hpp"
#include "PlatformTile.hpp"

#include <vector>

struct PuzzleMode : Mode
{
  PuzzleMode();
  virtual ~PuzzleMode();

  void add_player(glm::vec2 position, SDL_Keycode leftkey, SDL_Keycode rightkey, SDL_Keycode jumpkey);

  //functions called by main loop:
  virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
  virtual void update(float elapsed) override;
  virtual void draw(glm::uvec2 const &drawable_size) override;

  InputManager input_manager;

  //----- game state ------
  std::vector<Player *> players;
  std::vector < PlatformTile *> platforms;
};
