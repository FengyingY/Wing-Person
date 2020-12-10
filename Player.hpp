#pragma once

#include "Input.hpp"
#include "ColorTextureProgram.hpp"
#include "Shapes.hpp"
#include "Sprite.hpp"
#include "PlatformTile.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <limits>

#include "gl_errors.hpp"

  // Taken from https://stackoverflow.com/questions/8622256/in-c11-is-sqrt-defined-as-constexpr
  namespace Detail
  {
    double constexpr sqrtNewtonRaphson(double x, double curr, double prev)
    {
      return curr == prev
        ? curr
        : sqrtNewtonRaphson(x, 0.5 * (curr + x / curr), curr);
    }
  }

  /*
   * Constexpr version of the square root
   * Return value:
   *   - For a finite and non-negative value of "x", returns an approximation for the square root of "x"
   *   - Otherwise, returns NaN
   */
  double constexpr custom_sqrt(double x)
  {
    return x >= 0 && x < std::numeric_limits<double>::infinity()
      ? Detail::sqrtNewtonRaphson(x, x, 0)
      : std::numeric_limits<double>::quiet_NaN();
  }

struct Player {
  Player(glm::vec2 position, Input* left, Input* right, Input* jump, std::vector< Sprite* > idle_sprites, Sprite* jump_sprite, Sprite* fall_sprite, std::vector< Sprite* > run_sprites);
  ~Player();

	//functions called by main loop:
	void draw(glm::uvec2 const &drawable_size);

  // State
  glm::vec2 position;
  glm::vec2 velocity;
  float direction = 1.0f;

  Input* left;
  Input* right;
  Input* jump;

  // Constants
  static constexpr glm::vec2 size = glm::vec2(30.0f, 30.0f);

  static constexpr float rel_movespeed = 4.0f;
  static constexpr float movespeed = rel_movespeed * PlatformTile::default_size.x;


  // Jumping/gravity constants relative to the default height of a tile
  static constexpr float rel_fall_acceleration = -15.0f;
  static constexpr float rel_terminal_velocity = -30.0f;
  static constexpr float rel_max_jump_height = 3.2f;

  static constexpr float min_jump_time = 0.15f;
  static constexpr float max_jump_time = 0.2f;

  // Calculated jumping/gravity constants
  static constexpr float fall_acceleration = rel_fall_acceleration * PlatformTile::default_size.y;
  static constexpr float terminal_velocity = rel_terminal_velocity * PlatformTile::default_size.y;
  static constexpr float max_jump_height = rel_max_jump_height * PlatformTile::default_size.y;


  // max jump height = jumpspeed * max_jump_time + (jumpspeed * jumpspeed) / (2 * fall_acceleration)
  // i.e. jumpspeed = (time + sqrt(time * time + time * max_height / (2 * accel))) * accel
  static constexpr float jumpspeed = (- max_jump_time - (float)custom_sqrt(max_jump_time * max_jump_time - (2 * max_jump_height / fall_acceleration))) * fall_acceleration;


  float input_jump_time = 0.0f;
  float cur_jump_time = 0.0f;

  bool jump_input = false;
  bool jump_clear = false;
  bool falling = false;
  bool landed = true;

  //Drawing
	//Shader program that draws transformed, vertices tinted with vertex colors:
	ColorTextureProgram color_texture_program;

	//Buffer used to hold vertex data during drawing:
	GLuint vertex_buffer = 0;

	//Buffer for elements
	GLuint element_buffer = 0;

	//Vertex Array Object that maps buffer locations to color_texture_program attribute locations:
	GLuint vertex_buffer_for_color_texture_program = 0;

	//Solid white texture:
	GLuint png_tex = 0;

	// from game 0 base code
	//draw functions will work on vectors of vertices, defined as follows:
	struct Vertex {
		Vertex(glm::vec3 const &Position_, glm::u8vec4 const &Color_, glm::vec2 const &TexCoord_) :
			Position(Position_), Color(Color_), TexCoord(TexCoord_) { }
		glm::vec3 Position;
		glm::u8vec4 Color;
		glm::vec2 TexCoord;
	};
	static_assert(sizeof(Vertex) == 4*3 + 1*4 + 4*2, "PlatformTile::Vertex should be packed");

	// TODO: change this to suit sprites loaded from an atlas
	// loaded texture references
	struct Texture {
		Texture(glm::uvec2 const &size_, std::vector< glm::u8vec4 > const &data_) :
			size(size_), data(data_) {}
		glm::uvec2 size;
		std::vector< glm::u8vec4 > data;
	};

	//Collisions:
	Shapes::Rectangle collision_box;

	//to tell players apart
	// glm::u8vec4 color;
	Sprite *curr_sprite;
	std::vector< Sprite* > idle_sprites;
	Sprite *jump_sprite;
	Sprite *fall_sprite;
	std::vector< Sprite* > run_sprites;
};
