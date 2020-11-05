#include "PuzzleMode.hpp"

const float ScreenWidth = 800.0f;
const float ScreenHeight = 600.0f;

PuzzleMode::PuzzleMode() {
	// TODO : Read level data and create platforms

	// #TEMP : Creating a set of test platforms
	// Keeping window size as constant 800x600
	{
		// Floor
		PlatformTile *platform = new PlatformTile(glm::vec2(ScreenWidth * 0.5f, 5.0f), glm::vec2(ScreenWidth, 10.0f));
		level_platforms.emplace_back(platform);

		// Left wall
		platform = new PlatformTile(glm::vec2(5.0f, ScreenHeight * 0.5f), glm::vec2(10.0f, ScreenHeight));
		level_platforms.emplace_back(platform);

		// Right wall
		platform = new PlatformTile(glm::vec2(ScreenWidth - 5.0f, ScreenHeight * 0.5f), glm::vec2(10.0f, ScreenHeight));
		level_platforms.emplace_back(platform);

		// partition
		platform = new PlatformTile(glm::vec2(ScreenWidth * 0.5f, ScreenHeight * 0.5f), glm::vec2(10.0f, ScreenHeight));
		level_platforms.emplace_back(platform);

		// lower-mid
		platform = new PlatformTile(glm::vec2(ScreenWidth * 0.5f, 125.0f), glm::vec2(100.0f, 10.0f));
		level_platforms.emplace_back(platform);

		// mid-left
		platform = new PlatformTile(glm::vec2(10.0f + 50.0f, 250.0f), glm::vec2(100.0f, 10.0f));
		level_platforms.emplace_back(platform);

		// mid-right
		platform = new PlatformTile(glm::vec2(ScreenWidth - 50.0f - 10.0f, 250.0f), glm::vec2(100.0f, 10.0f));
		level_platforms.emplace_back(platform);

		// upper-mid
		platform = new PlatformTile(glm::vec2(ScreenWidth * 0.5f, 375.0f), glm::vec2(100.0f, 10.0f));
		level_platforms.emplace_back(platform);
	}
}

PuzzleMode::~PuzzleMode() {

}

bool PuzzleMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
	
	return _input_manager.handle_event(evt);
}

void PuzzleMode::update(float elapsed) {
	/*
	// Calculate inputs and movement for each player
	for (auto&& player : _players) {
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
			player._velocity.y -= 9.8f * elapsed;
		}
	}*/
}

void PuzzleMode::draw(glm::uvec2 const &drawable_size) {
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	for (auto &&platform : level_platforms)
	{
		platform->draw(drawable_size);
	}
	GL_ERRORS();
}

// --- Added from Stage.cpp ---
void PuzzleMode::add_player(Shapes::Rectangle bounds, SDL_Keycode leftkey, SDL_Keycode rightkey, SDL_Keycode jumpkey)
{
	Input* left = _input_manager.register_key(leftkey);
	Input* right = _input_manager.register_key(rightkey);
	Input* jump = _input_manager.register_key(jumpkey);
	_players.emplace_back(Player(bounds, left, right, jump));
}