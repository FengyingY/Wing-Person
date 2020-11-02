#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "gl_errors.hpp"
#include "data_path.hpp"
#include "load_save_png.hpp"
#include "ColorTextureProgram.hpp"

struct Sprite
{
    Sprite(std::string file_name);

    ~Sprite();

    void draw(glm::vec2 center_pos, glm::vec2 drawable_size, float scale);
    
    std::string name;
    glm::uvec2 size;
	std::vector< glm::u8vec4 > data;

    
    // from game 0 base code
	
	//draw functions will work on vectors of vertices, defined as follows:
	struct Vertex {
		Vertex(glm::vec3 const &Position_, glm::u8vec4 const &Color_, glm::vec2 const &TexCoord_) :
			Position(Position_), Color(Color_), TexCoord(TexCoord_) { }
		glm::vec3 Position;
		glm::u8vec4 Color;
		glm::vec2 TexCoord;
	};
	static_assert(sizeof(Vertex) == 4*3 + 1*4 + 4*2, "PongMode::Vertex should be packed");
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

};
