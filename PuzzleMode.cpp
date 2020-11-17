#include "PuzzleMode.hpp"

#include "Collisions.hpp"
#include "PackLevelData.hpp"
#include "read_write_chunk.hpp"

#include <fstream>

const float ScreenWidth = 800.0f;
const float ScreenHeight = 600.0f;

Load< std::vector<uint32_t> > level_data(LoadTagDefault, []() -> std::vector<uint32_t> * {
	std::vector<uint32_t> *ret = new std::vector< uint32_t >();

	std::ifstream dat_file(data_path("level-data.dat"), std::ios::binary);
	read_chunk(dat_file, "lvdt", ret);

	return ret;
});

Load <PlatformTile::Texture> tile_set(LoadTagEarly, []() -> PlatformTile::Texture * {
	PlatformTile::Texture *ret = new PlatformTile::Texture();

	try
	{
		load_png(data_path("puzzle_sprites/platform.png"), &ret->size, &ret->data, LowerLeftOrigin);
	}
	catch(const std::exception& e)
	{
		std::cerr << "Load png of platform failed. " << e.what() << '\n';
	}

	return ret;
});

PuzzleMode::PuzzleMode() {
	// Read level data and create platforms
	{
		float tile_size = 32.0f;
		PlatformTile *platform;
		for (size_t y = 0; y < 18; y++)	// TODO: Read Tile map layer data and set height & width accordingly
		{
			for (size_t x = 0; x < 25; x++)
			{
				if (level_data->at(y*25 + x) == 0){
					continue;
				}
				// TODO : Assign texture to selected tile
				platform = new PlatformTile(glm::vec2((x * tile_size) + (tile_size * 0.5f), ScreenHeight - (y * tile_size) - (tile_size * 0.5f)), glm::vec2(tile_size, tile_size), PlatformTile::Texture(tile_set->size, tile_set->data));
				platforms.emplace_back(platform);
				platform_collision_shapes.emplace_back(platform->collision_shape);
			}
		}
	}

	// #HACK : spawn 2 default players
	add_player(glm::vec2(200, 75), SDLK_a, SDLK_d, SDLK_w, glm::u8vec4(0x65, 0xc9, 0xee, 0xff));
	add_player(glm::vec2(600, 75), SDLK_LEFT, SDLK_RIGHT, SDLK_UP, glm::u8vec4(0xf3, 0x0c, 0x23, 0xff));
}

PuzzleMode::~PuzzleMode() {}

void PuzzleMode::add_player(glm::vec2 position, SDL_Keycode leftkey, SDL_Keycode rightkey, SDL_Keycode jumpkey, glm::u8vec4 color) {
	Input* left = input_manager.register_key(leftkey);
	Input* right = input_manager.register_key(rightkey);
	Input* jump = input_manager.register_key(jumpkey);

	Player *player = new Player(position, left, right, jump, color);
	players.emplace_back(player);
}

bool PuzzleMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
	return input_manager.handle_event(evt);
}

void PuzzleMode::update(float elapsed) {
	for (int i = 0; i < players.size(); i++) {

		// Calculate inputs and movement for each player
		players[i]->velocity.x = 0;
		players[i]->velocity.y = 0;

		if (players[i]->left->held()) {
			players[i]->velocity.x -= Player::movespeed * elapsed;
		}

		if (players[i]->right->held()) {
			players[i]->velocity.x += Player::movespeed * elapsed;
		}

		// Process jumping
		if (players[i]->jump->held() && !players[i]->falling) {
			players[i]->jump_input = true;
			if (players[i]->input_jump_time < Player::max_jump_time) {
				players[i]->input_jump_time += elapsed;
				if (players[i]->input_jump_time >= Player::max_jump_time) {
					players[i]->input_jump_time = Player::max_jump_time;
				}
			}
		}

		if (players[i]->jump->just_released()) {
			players[i]->jump_input = false;
			if (players[i]->input_jump_time < Player::min_jump_time) {
				players[i]->input_jump_time = Player::min_jump_time;
			}
		}

		if (players[i]->cur_jump_time < players[i]->input_jump_time) {
			players[i]->cur_jump_time += elapsed;
			players[i]->velocity.y += Player::jumpspeed * elapsed;

			if (players[i]->cur_jump_time >= players[i]->input_jump_time) {
				players[i]->jump_clear = true;
			}
		}

		if (!players[i]->jump->held() && players[i]->jump_clear) {
			players[i]->cur_jump_time = 0.0f;
			players[i]->input_jump_time = 0.0f;
			players[i]->jump_clear = false;
		}

		//quick fix to deal with collision box center not updating properly; not at all optimized code
		int other_player_index = (i + 1) % players.size();
		glm::vec2 center = players[other_player_index]->collision_box.center;
		float width = players[other_player_index]->collision_box.width;
		float height = players[other_player_index]->collision_box.height;

		//add the other player to the list of things the current player can collide with
		platform_collision_shapes.emplace_back(Shapes::Rectangle::Rectangle(center, width, height, false));

		//check for collisions before moving due to input:
		if (!Collisions::player_rectangles_collision(players[i]->collision_box, players[i]->position + players[i]->velocity, platform_collision_shapes)) {
			players[i]->position += players[i]->velocity;
			players[i]->collision_box.center += players[i]->velocity;
		}

		//check for collisions before moving due to gravity:
		glm::vec2 gravity = glm::vec2(0, -Player::gravityspeed * elapsed);
		if (!Collisions::player_rectangles_collision(players[i]->collision_box, players[i]->position + gravity, platform_collision_shapes)) {
			players[i]->position += gravity;
			players[i]->collision_box.center += gravity;
			players[i]->on_ground = false;
		} else players[i]->on_ground = true;

		//remove the other player to the list of things the current player can collide with
		platform_collision_shapes.pop_back();

		/* Player-player collision
		for (auto t1 = players.begin(); t1 != players.end(); t1++) {
		  for (auto t2 = t1 + 1; t2 != players.end(); t2++) {
		  Shapes::Rectangle rect1 = Shapes::Rectangle((*t1)->position, Player::size.x, Player::size.y, false);
			Shapes::Rectangle rect2 = Shapes::Rectangle((*t2)->position, Player::size.x, Player::size.y, false);
			(*t1)->position += Collisions::rectangle_rectangle_collision(rect1, rect2, 2);
		  }
		}*/

		/* Player-platform collision
		for (auto& player : players) {
		  // Check for collision with platforms
		  Shapes::Rectangle player_rect = Shapes::Rectangle(player->position,
			  Player::size.x, Player::size.y, false);

		  for (auto& platform : platforms) {
			Shapes::Rectangle platform_rect = Shapes::Rectangle(platform->position,
			  platform->size.x, platform->size.y, true);

			if(Collisions::rectangle_rectangle_collision(player_rect, platform_rect)) {
			  player->position += Collisions::rectangle_rectangle_collision(player_rect,
				  platform_rect, 2);
			  break;
			}
		  }
		}*/

		/* Gravity
		for (auto& player : players) {
		  glm::vec2 gravity = glm::vec2(0, -Player::gravityspeed * elapsed);

		  player->falling = true;

		  // Check for collision with platforms
		  Shapes::Rectangle player_rect = Shapes::Rectangle(player->position + gravity,
			  Player::size.x, Player::size.y, false);

		  for (auto& platform : platforms) {
			Shapes::Rectangle platform_rect = Shapes::Rectangle(platform->position,
			  platform->size.x, platform->size.y, true);

			if(Collisions::rectangle_rectangle_collision(player_rect, platform_rect)) {
			  glm::vec2 movement = Collisions::rectangle_rectangle_collision(player_rect,
				  platform_rect, 1);

			  //FIXME bug in collisions with nonzero x value
			  movement.x = 0;

			  player->position += gravity + movement;
			  player->falling = false;
			  break;
			}
		  }

		  if(player->falling) {
			player->position += gravity;
		  }
		}*/

		input_manager.tick();
	}
}

void PuzzleMode::draw(glm::uvec2 const &drawable_size) {
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	GL_ERRORS();

	for (auto &&platform : platforms)
	{
		platform->draw(drawable_size);
	}

  for (auto&& player : players) {
    
    player->draw(drawable_size);
  }
//   std::cout << std::endl;
	GL_ERRORS();
}
