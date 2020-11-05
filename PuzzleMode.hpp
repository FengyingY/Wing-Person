#pragma once
#include "Mode.hpp"
#include "PlatformTile.hpp"

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
	std::vector < PlatformTile *> level_platforms;

	PlatformTile *tile;

};
