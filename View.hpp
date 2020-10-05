#pragma once

#include <string>
#include <vector>
#include <utility>

#include <glm/glm.hpp>
#include "GL.hpp"
#include <hb.h>
#include <hb-ft.h>
#include <freetype/freetype.h>
#include <SDL.h>

namespace view {

class ViewContext {
public:
	glm::uvec2 logical_size_;
	glm::uvec2 drawable_size_;
	float scale_factor_;
	bool is_initialized = false;

	static const ViewContext &get();
	static void set(const glm::uvec2 &logicalSize, const glm::uvec2 &drawableSize);
	static unsigned compute_physical_px(unsigned logical_px) {
		return (unsigned) lround(logical_px * get().scale_factor_ + 0.5f);
	}

private:
	ViewContext() = default;
	static ViewContext singleton_;
};

class TextLine {
public:
	/**
	 * Create a line of text
	 * @param content the string to be displayed
	 * @param cursor_x position in window, 0 to 1280. 0 means left.
	 * @param cursor_y position in window, 0 to 720. 0 means top
	 * @param fg_color color. glm::uvec4(255,255,255,255) means white
	 * @param font_size font size in logical pixels (like CSS pixels)
	 */
	TextLine(std::string content, int cursor_x, int cursor_y, glm::uvec4 fg_color, unsigned font_size) :
		TextLine{std::move(content),
		         static_cast<float>((cursor_x - 1280 / 2) * (2.0f / 1280)),
		         static_cast<float>(-(cursor_y - 720 / 2) * (2.0f / 720)),
		         glm::vec4(fg_color) / 255.0f, font_size} {}

	/**
	 * Create a line of text -- the more openGL friendly version
	 * @param content the string to be displayed.
	 * @param cursor_x position in window. Range: [-1.0f,1.0f]. -1 means left.
	 * @param cursor_y position in window. Range: [-1.0f,1.0f]. -1 means bottom.
	 * @param fg_color color, range: [0.0f, 1.0f]. 1.0 means highest brightness
	 * @param font_size font size in logical pixels (like CSS pixels)
	 */
	TextLine(std::string content, float cursor_x, float cursor_y, glm::vec4 fg_color, unsigned font_size);

	/**
	 * copy constructor
	 */
	TextLine(const TextLine &that);

	/**
	 * TODO(xiaoqiao)
	 * copy assignment not implemented yet
	 */
	TextLine &operator=(const TextLine &) = delete;

	/**
	 * Move constructor and assignment is deleted -- because it'll be a hassle moving OpenGL resources
	 * @param that
	 */
	TextLine(TextLine &&that) = delete;
	TextLine& operator=(TextLine &&) = delete;
	TextLine() = delete;
	~TextLine();
	void update(float elapsed);
	void draw();

private:

	std::string content_;
	float cursor_x_;
	float cursor_y_;
	glm::vec4 fg_color_;
	unsigned font_size_; //< font size in "logical pixel"

	static constexpr char FONT_NAME[] = "cmunorm.ttf";
	FT_Library ft_library_ = nullptr;
	FT_Face face_ = nullptr;
	hb_buffer_t *hb_buffer_ = nullptr;
	hb_font_t *font_ = nullptr;
	unsigned int glyph_count_ = 0;
	hb_glyph_info_t *glyph_info_ = nullptr;
	hb_glyph_position_t *glyph_pos_ = nullptr;

	GLuint texture_{0}, sampler_{0};
	GLuint vbo_{0}, vao_{0};

	static glm::vec2 get_scale_physical() {
		const auto &ctx = ViewContext::get();
		return glm::vec2(2.0f) / glm::vec2(ctx.drawable_size_);
	}
};

class TextBox{
public:
	TextBox(std::vector<std::pair<glm::uvec3, std::string>> contents,
	        const glm::ivec2 &position,
	        unsigned int fontSize);
	void draw();
private:

	glm::ivec2 position_;
	unsigned font_size_;
	std::vector<std::pair<glm::uvec3, std::string>> contents_;
	std::vector<TextLine> lines_;
};

}