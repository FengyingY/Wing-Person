#include "PuzzleMode.hpp"

#include "Collisions.hpp"
#include "PackLevelData.hpp"
#include "data_path.hpp"
#include "read_write_chunk.hpp"

#include "StoryMode.hpp"
// #include "View.hpp"

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
	images.emplace_back("puzzle_sprites/End.png");

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

std::vector< Sprite* > red_idle, red_run, blue_idle, blue_run;
Sprite *red_jump, *red_fall, *blue_jump, *blue_fall;
Load< void > load_sprites(LoadTagEarly, []() -> void {
	red_jump = new Sprite(data_path("puzzle_sprites/red/jump.png"), "red_jump");
	red_fall = new Sprite(data_path("puzzle_sprites/red/fall.png"), "red_fall");
	blue_jump = new Sprite(data_path("puzzle_sprites/blue/jump.png"), "blue_jump");
	blue_fall = new Sprite(data_path("puzzle_sprites/blue/fall.png"), "blue_fall");

	bool look_for_red_idle = true, look_for_red_run = true, look_for_blue_idle = true, look_for_blue_run = true;
	for (int i = 0; i < 100; i++) {
		std::string tile_name = "tile0";
		if (i <= 9) tile_name = tile_name + std::to_string(0) + std::to_string(i);
		else tile_name = tile_name + std::to_string(i);
		
		if (look_for_red_idle) {
			Sprite *new_red_idle = new Sprite(data_path("puzzle_sprites/red/idle/" + tile_name + ".png"), "red_idle_" + tile_name);
			if (new_red_idle->data.size() > 0) red_idle.emplace_back(new_red_idle);
			else look_for_red_idle = false;
		}
		
		if (look_for_red_run) {
			Sprite *new_red_run = new Sprite(data_path("puzzle_sprites/red/run/" + tile_name + ".png"), "red_run_" + tile_name);
			if (new_red_run->data.size() > 0) red_run.emplace_back(new_red_run);
			else look_for_red_run = false;
		}

		if (look_for_blue_idle) {
			Sprite *new_blue_idle = new Sprite(data_path("puzzle_sprites/blue/idle/" + tile_name + ".png"), "blue_idle_" + tile_name);
			if (new_blue_idle->data.size() > 0) blue_idle.emplace_back(new_blue_idle);
			else look_for_blue_idle = false;
		}

		if (look_for_blue_run) {
			Sprite *new_blue_run = new Sprite(data_path("puzzle_sprites/blue/run/" + tile_name + ".png"), "blue_run_" + tile_name);
			if (new_blue_run->data.size() > 0) blue_run.emplace_back(new_blue_run);
			else look_for_blue_run = false;
		}

		if (!(look_for_red_idle || look_for_red_run || look_for_blue_idle || look_for_blue_run)) break;
	}
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
				}
				else if(level_data->at(y*25 + x) == 76){
					end = new PlatformTile(
						glm::vec2((x * tile_size) + (tile_size * 0.5f), ScreenHeight - (y * tile_size) - (tile_size * 0.5f)),
						glm::vec2(tile_size, tile_size),
						PlatformTile::Texture(sprites->at("puzzle_sprites/End.png")->size, sprites->at("puzzle_sprites/End.png")->data)
					);
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
	add_player(glm::vec2(200, 85), SDLK_a, SDLK_d, SDLK_w, red_idle, red_jump, red_fall, red_run);
	add_player(glm::vec2(600, 85), SDLK_LEFT, SDLK_RIGHT, SDLK_UP, blue_idle, blue_jump, blue_fall, blue_run);
}

PuzzleMode::~PuzzleMode() {}

void PuzzleMode::add_player(glm::vec2 position, SDL_Keycode leftkey, SDL_Keycode rightkey, SDL_Keycode jumpkey,
	std::vector< Sprite* > idle_sprites, Sprite* jump_sprite, Sprite* fall_sprite, std::vector< Sprite* > run_sprites) {
	
	Input* left = input_manager.register_key(leftkey);
	Input* right = input_manager.register_key(rightkey);
	Input* jump = input_manager.register_key(jumpkey);

	Player *player = new Player(position, left, right, jump, idle_sprites, jump_sprite, fall_sprite, run_sprites);
	players.emplace_back(player);
}

bool PuzzleMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
	return input_manager.handle_event(evt);
}

void PuzzleMode::update(float elapsed) {
	total_time += elapsed;

	if (is_timeup)
	{
		return;
	}
	
	// update puzzle time and countdown
	if (puzzle_time >= MaxPuzzleTime)
	{
		// puzzle failed. Perform negative action
		is_timeup = true;

		std::string branch_name = "Story16";
		Mode::set_current(std::make_shared<StoryMode>(branch_name));
		return;
	}
	else
	{
		puzzle_time += elapsed;
	}
	
	

	for (unsigned int i = 0; i < players.size(); i++) {

		// Calculate inputs and movement for each player
		players[i]->velocity.x = 0;
		players[i]->velocity.y = 0;

		size_t idle_sprites_size = players[i]->idle_sprites.size();
		size_t run_sprites_size = players[i]->run_sprites.size();
		if (!players[i]->falling)
			players[i]->curr_sprite = players[i]->idle_sprites[(int)(idle_sprites_size * total_time) % idle_sprites_size];

		if (players[i]->left->held()) {
			players[i]->velocity.x -= Player::movespeed * elapsed;
			players[i]->direction = -1.0f;
			if (!players[i]->falling)
				players[i]->curr_sprite = players[i]->run_sprites[(int)(run_sprites_size * total_time) % run_sprites_size];
		}

		if (players[i]->right->held()) {
			players[i]->velocity.x += Player::movespeed * elapsed;
			players[i]->direction = 1.0f;
			if (!players[i]->falling) {
				if (players[i]->left->held())
					players[i]->curr_sprite = players[i]->idle_sprites[(int)(idle_sprites_size * total_time) % idle_sprites_size];
				else
					players[i]->curr_sprite = players[i]->run_sprites[(int)(run_sprites_size * total_time) % run_sprites_size];
			}
		}

		// Process jumping
		if (players[i]->jump->held() && !players[i]->falling) {
			players[i]->jump_input = true;
			players[i]->curr_sprite = players[i]->jump_sprite;
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
			if (!players[i]->jump->held() || players[i]->input_jump_time >= Player::max_jump_time)
				players[i]->curr_sprite = players[i]->fall_sprite;
		} else {
			players[i]->falling = false;
			if (!(players[i]->left->held() || players[i]->right->held()) ||
				(players[i]->left->held() && players[i]->right->held()))
				players[i]->curr_sprite = players[i]->idle_sprites[(int)(idle_sprites_size * total_time) % idle_sprites_size];
			else if (players[i]->left->held()) {
				players[i]->direction = -1.0f;
				players[i]->curr_sprite = players[i]->run_sprites[(int)(run_sprites_size * total_time) % run_sprites_size];
			} else if (players[i]->right->held()) {
				players[i]->direction = 1.0f;
				players[i]->curr_sprite = players[i]->run_sprites[(int)(run_sprites_size * total_time) % run_sprites_size];
			}
		}

		//remove the other player to the list of things the current player can collide with
		platform_collision_shapes.pop_back();

		// collectibles collection check | Not using collision. Just checking for distance
		if (collectibles.size() > 0)
		{
			for (auto &&collectible : collectibles)
			{
				float sqr_dist = pow(collectible->position.x - players[i]->position.x, 2) + pow(collectible->position.y - players[i]->position.y, 2);
				if(sqr_dist < pow(collectible->size.x * 0.5f, 2)){
					// removing an element from a vector - https://stackoverflow.com/a/3385251
					collectibles.erase(std::remove(collectibles.begin(), collectibles.end(), collectible));
					break;
				}
			}
		}
		
		if (end != nullptr)
		{
			float sqr_dist = pow(end->position.x - players[i]->position.x, 2) + pow(end->position.y - players[i]->position.y, 2);
				if(sqr_dist < pow(end->size.x * 0.5f, 2)){
					std::string branch_name = "Story16";
					Mode::set_current(std::make_shared<StoryMode>(branch_name));
				}
		}
		

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

	end->draw(drawable_size);
	

  for (auto&& player : players) {
    
    player->draw(drawable_size);
  }
//   std::cout << std::endl;
	GL_ERRORS();
}