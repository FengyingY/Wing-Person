#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "gl_errors.hpp"
#include "ColorTextureProgram.hpp"
#include "Sprite.hpp"

struct PlatformTile
{
	PlatformTile();
	PlatformTile(glm::vec2 const &pos_, glm::vec2 const &size_);
	~PlatformTile();

	void setup_opengl();

	// world position
	glm::vec2 position;

	// size [x,y]
	glm::vec2 size;

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

	// TODO: change this to suit sprites loaded from an atlas
	// loaded texture references
	struct Texture {
		Texture(glm::uvec2 const &size_, std::vector< glm::u8vec4 > const &data_) :
			size(size_), data(data_) {}
		glm::uvec2 size;
		std::vector< glm::u8vec4 > data;
	};
};
