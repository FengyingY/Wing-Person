#pragma once

#include "Shapes.hpp"
#include "Input.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "gl_errors.hpp"
#include "ColorTextureProgram.hpp"

struct Player {

    Shapes::Rectangle bounding_box;

    Player(Shapes::Rectangle bounds, Input* left, Input* right, Input* jump);

    glm::vec2 _velocity;

    Input* _left;
    Input* _right;
    Input* _jump;

    // Constants
    static constexpr float movespeed = 10.0f;
    static constexpr float jumpspeed = 20.0f;
    static constexpr float gravityspeed = 10.0f;

    static constexpr float min_jump_time = 0.3f;
    static constexpr float max_jump_time = 0.6f;

    float _input_jump_time = 0.0f;
    float _cur_jump_time = 0.0f;

    bool _jump_input = false;
    bool _falling = false;

	// ---- Rendering ---
	//functions called by main loop:
	void draw(glm::uvec2 const &drawable_size);

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
};