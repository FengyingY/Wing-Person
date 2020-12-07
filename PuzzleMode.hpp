#pragma once

#include "Mode.hpp"

#include "Input.hpp"
#include "Player.hpp"
#include "PlatformTile.hpp"
#include "Input.hpp"
#include "Player.hpp"

#include <vector>

struct PuzzleMode : Mode
{
	PuzzleMode(){};
	PuzzleMode(uint32_t level);
	virtual ~PuzzleMode();

	uint32_t parse_tiledata(uint32_t &tile_data);

	void add_player(glm::vec2 position, SDL_Keycode leftkey, SDL_Keycode rightkey, SDL_Keycode jumpkey,
		std::vector< Sprite* > idle_sprites, Sprite* jump_sprite, Sprite* fall_sprite, std::vector< Sprite* > run_sprites);

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	InputManager input_manager;

	//----- game state ------
	std::vector < Player *> players;
	
	std::vector < PlatformTile *> platforms;
	std::vector < Shapes::Rectangle > platform_collision_shapes;
	
	std::vector < PlatformTile *> collectibles;

	std::vector < PlatformTile *> objects;
	std::vector < Shapes::Rectangle > object_collision_shapes;

	std::vector < PlatformTile *> bg_tiles;

	PlatformTile *end = nullptr;
	float total_time = 0.0f;

	const float MaxPuzzleTime = 300.0f;	// in seconds
	float puzzle_time = 0.0f;
	bool is_timeup = false;
  
};
