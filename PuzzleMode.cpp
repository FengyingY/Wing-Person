#include "PuzzleMode.hpp"

#include "Collisions.hpp"
#include "PackLevelData.hpp"
#include "data_path.hpp"
#include "read_write_chunk.hpp"

#include <fstream>
#include <map>

const float ScreenWidth = 800.0f;
const float ScreenHeight = 600.0f;

Load< std::vector<uint32_t> > level_data(LoadTagDefault, []() -> std::vector<uint32_t> * {
	std::vector<uint32_t> *ret = new std::vector< uint32_t >();

	std::ifstream dat_file(data_path("level-data.dat"), std::ios::binary);
	read_chunk(dat_file, "lvdt", ret);

	return ret;
});

Load < std::map<std::string, PlatformTile::Texture*> > sprites(LoadTagEarly, []() -> std::map<std::string, PlatformTile::Texture *> * {
	std::map<std::string, PlatformTile::Texture*> *ret = new std::map<std::string, PlatformTile::Texture *>();

	std::vector< std::string > images;
	images.emplace_back("puzzle_sprites/platform.png");
	images.emplace_back("puzzle_sprites/collectible.png");

	try
	{
		PlatformTile::Texture *newTex = nullptr;
		for (auto &&img : images)
		{
			newTex = new PlatformTile::Texture();
			load_png(data_path(img), &newTex->size, &newTex->data, LowerLeftOrigin);
			ret->emplace(img, newTex);
		}
	}
	catch(const std::exception& e)
	{
		std::cerr << "PuzzleMode::Sprites load failed. " << e.what() << '\n';
	}
	
	
	return ret;
});

Sprite *pink, *blue;
Load< void > load_sprites(LoadTagEarly, []() -> void {
	pink = new Sprite(data_path("puzzle_sprites/pinkman.png"), "pink_man");
	blue = new Sprite(data_path("puzzle_sprites/virtualguy.png"), "virtual_guy");
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

				if (level_data->at(y*25 + x) == 36){	// HACK: collectible
					platform = new PlatformTile(
						glm::vec2((x * tile_size) + (tile_size * 0.5f), ScreenHeight - (y * tile_size) - (tile_size * 0.5f)),
						glm::vec2(tile_size, tile_size),
						PlatformTile::Texture(sprites->at("puzzle_sprites/collectible.png")->size, sprites->at("puzzle_sprites/collectible.png")->data)
					);
					collectibles.emplace_back(platform);
					collectible_colliders.emplace_back(platform->collision_shape);
				}
				else
				{
					// TODO : Assign proper texture to tile based on tilemap data
					platform = new PlatformTile(
						glm::vec2((x * tile_size) + (tile_size * 0.5f),	ScreenHeight - (y * tile_size) - (tile_size * 0.5f)),
						glm::vec2(tile_size, tile_size),
						PlatformTile::Texture(sprites->at("puzzle_sprites/platform.png")->size, sprites->at("puzzle_sprites/platform.png")->data)
					);
					platforms.emplace_back(platform);
					platform_collision_shapes.emplace_back(platform->collision_shape);
				}
			}
		}
	}

	// #HACK : spawn 2 default players
	add_player(glm::vec2(200, 75), SDLK_a, SDLK_d, SDLK_w, pink);
	add_player(glm::vec2(600, 75), SDLK_LEFT, SDLK_RIGHT, SDLK_UP, blue);
}

PuzzleMode::~PuzzleMode() {}

void PuzzleMode::add_player(glm::vec2 position, SDL_Keycode leftkey, SDL_Keycode rightkey, SDL_Keycode jumpkey, Sprite* sprite) {
	Input* left = input_manager.register_key(leftkey);
	Input* right = input_manager.register_key(rightkey);
	Input* jump = input_manager.register_key(jumpkey);

	Player *player = new Player(position, left, right, jump, sprite);
	players.emplace_back(player);
}

bool PuzzleMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
	return input_manager.handle_event(evt);
}

void PuzzleMode::update(float elapsed) {
	for (unsigned int i = 0; i < players.size(); i++) {

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
			players[i]->falling = true;
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
			players[i]->falling = true;
		}

		//quick fix to deal with collision box center not updating properly; not at all optimized code
		int other_player_index = (i + 1) % players.size();
		glm::vec2 center = players[other_player_index]->collision_box.center;
		float width = players[other_player_index]->collision_box.width;
		float height = players[other_player_index]->collision_box.height;

		//add the other player to the list of things the current player can collide with
		platform_collision_shapes.emplace_back(Shapes::Rectangle(center, width, height, false));

		//check for collisions before moving due to input:
		if (!Collisions::player_rectangles_collision(players[i]->collision_box, players[i]->position + players[i]->velocity, platform_collision_shapes)) {
			players[i]->position += players[i]->velocity;
			players[i]->collision_box.center += players[i]->velocity;
		}

		//check for collisions before moving due to gravity:
		glm::vec2 gravity = glm::vec2(0, -players[i]->gravityspeed * elapsed);
		if (!Collisions::player_rectangles_collision(players[i]->collision_box, players[i]->position + gravity, platform_collision_shapes)) {
			players[i]->position += gravity;
			players[i]->collision_box.center += gravity;
		} else players[i]->falling = false;

		//remove the other player to the list of things the current player can collide with
		platform_collision_shapes.pop_back();

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

	for (auto &&collectible : collectibles)
	{
		collectible->draw(drawable_size);
	}
	

  for (auto&& player : players) {
    
    player->draw(drawable_size);
  }
//   std::cout << std::endl;
	GL_ERRORS();
}
