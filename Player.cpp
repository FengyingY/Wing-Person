#include "Player.hpp"

Player::Player(glm::vec2 position_, Input* left_, Input* right_, Input* jump_, std::vector< Sprite* > idle_sprites_, Sprite* jump_sprite_, Sprite* fall_sprite_, std::vector< Sprite* > run_sprites_) :
	position(position_), left(left_), right(right_), jump(jump_), idle_sprites(idle_sprites_), jump_sprite(jump_sprite_), fall_sprite(fall_sprite_), run_sprites(run_sprites_) {
	
	curr_sprite = idle_sprites[0];
	collision_box = Shapes::Rectangle(position, (float)Player::size.x, (float)Player::size.y, false);
}

Player::~Player() {
	
}

	glDeleteTextures(1, &png_tex);
	png_tex = 0;
}

void Player::draw(glm::uvec2 const &drawable_size) {
	
	curr_sprite->draw(position, drawable_size, 0.6f, direction);
}
