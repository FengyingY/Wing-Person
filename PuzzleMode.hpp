#pragma once
#include "Mode.hpp"
#include "PlatformTile.hpp"
#include "Input.hpp"
#include "Player.hpp"

#include <vector>

struct PuzzleMode : Mode
{
	PuzzleMode();
	virtual ~PuzzleMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state ------
	InputManager _input_manager;
	std::vector < PlatformTile *> level_platforms;
	std::vector < Player > _players;

	// ---- methods ----
	void add_player(Shapes::Rectangle bounds, SDL_Keycode leftkey, SDL_Keycode rightkey, SDL_Keycode jumpkey);
};
