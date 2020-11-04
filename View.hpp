#pragma once

#include <string>
#include <utility>
#include <vector>
#include <optional>
#include <functional>
#include <memory>
#include <map>

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

enum class FontFace {
	ComputerModernRegular,
	IBMPlexMono,
};

/**
 * GlyphTextureCache
 *
 * A Texture class that holds all the OpenGL textures for a FreeType glyph bitmap.
 *
 * This is a singleton class.
 */
class GlyphTextureCache {
public:
	struct GlyphTextureEntry {
	friend GlyphTextureCache;
		std::vector<uint8_t> bitmap;
		int bitmap_left;
		int bitmap_top;
		int width;
		int height;
		GLuint gl_texture_id;
		GlyphTextureEntry(const std::vector<uint8_t> &bitmap,
		                  int bitmapLeft,
		                  int bitmapTop,
		                  int width,
		                  int height,
		                  int refCnt);
		GlyphTextureEntry() = delete;
		GlyphTextureEntry(const GlyphTextureEntry &other) = delete;
		~GlyphTextureEntry();
		int ref_cnt;
	};
	static GlyphTextureCache *get_instance();
	void incTexRef(FontFace font_face, int font_size, hb_codepoint_t codepoint);
	const GlyphTextureEntry *getTextRef(FontFace font_face, int font_size, hb_codepoint_t codepoint);
	void decTexRef(FontFace font_face, int font_size, hb_codepoint_t codepoint);
	FT_Face get_free_type_face(FontFace font_face);
private:
	GlyphTextureCache();
	~GlyphTextureCache();

	static std::string get_font_filename(FontFace font_face);

	struct GlyphTextureKey {
		FontFace font_face;
		int font_size;
		hb_codepoint_t codepoint;
		GlyphTextureKey(FontFace fontFace, int fontSize, hb_codepoint_t codepoint);
		bool operator<(const GlyphTextureKey &rhs) const;
	};
	static GlyphTextureCache *singleton_;
	std::map<GlyphTextureKey, GlyphTextureEntry> map_;
	FT_Library ft_library_ = nullptr;
	std::map<FontFace, FT_Face> font_faces_;
};

class TextLine {
public:
	/**
	 * Constructor - create a TextLine object with an empty string, default position, font face, etc.
	 */
	TextLine();
	TextLine(const TextLine &other);
	TextLine &operator=(const TextLine &other);
	~TextLine();

	TextLine &set_text(std::string text);
	TextLine &set_font(FontFace font_face);
	TextLine &set_font_size(unsigned font_size);
	TextLine &set_position(int x, int y);
	TextLine &set_position(glm::ivec2 pos);
	TextLine &set_color(glm::u8vec4 color);
	TextLine &disable_animation();
	TextLine &set_animation(float speed, std::optional<std::function<void()>> callback);
	TextLine &set_visibility(bool value);

	/**
	 * redo_shape: this function should be called when new call to hb_shape() is needed.
	 *
	 * This function should be called when, for example:
	 * (1) text is changed
	 * (2) font size or screen dpi is changed
	 * (3) font is changed
	 *
	 */
	 void redo_shape();

	void draw();
	void update(float elapsed);


private:
	void do_render();
	void undo_render();

private:

	//  ---- data fields that directly relates to setters ---
	std::string text_;
	FontFace font_ = FontFace::ComputerModernRegular;
	unsigned font_size_ = 16; //< font size in "logical pixel"
	glm::ivec2 position_ = glm::ivec2(0, 0);
	glm::u8vec4 color_ = glm::u8vec4(255);
	std::optional<float> animation_speed_ = std::nullopt;
	std::optional<std::function<void()>> callback_ = std::nullopt;
	bool is_visible_ = true;

	// ---- internal state related to animation ----
	float total_time_elapsed_ = 0.0f;
	unsigned int visible_glyph_count_ = 0;

	// ---- internal states that can be discarded on a copy constructor
	// text_is_rendered_: a state variable
	// if set to true, this means (1) GlyphTextureCache holds the reference
	// of the latest text and (2) the hb_buffer_ related variable reflects the
	// content of latest text
	// if set to false, this means: (1) GlyphTextureCache doesn't hold
	// the glyph ref and (2) hb_buffer_ related fields are set to an empty
	// state
	bool text_is_rendered_ = false;

	// ---- internal state that can be reconstructed from other fields ----
	// ---- harf-buzz related fields ----
	// ---- only valid when text_is_rendered_ is true ---
	hb_buffer_t *hb_buffer_ = nullptr;
	unsigned int glyph_count_ = 0;
	hb_glyph_info_t *glyph_info_ = nullptr;
	hb_glyph_position_t *glyph_pos_ = nullptr;
	// ----- scale factor for HiDPI screens ----
	glm::vec2 scale_factor_;
	// ----- position for text in opengl friendly format
	glm::vec2 cursor_;

	// --- opengl resources ---
	GLuint sampler_{0};
	GLuint vbo_{0}, vao_{0};

	static glm::vec2 get_scale_physical() {
		const auto &ctx = ViewContext::get();
		return glm::vec2(2.0f) / glm::vec2(ctx.drawable_size_);
	}
};

class TextBox{
public:
	TextBox() = default;
	~TextBox() = default;

	TextBox &set_contents(std::vector<std::pair<glm::u8vec4, std::string>> contents);
	TextBox &set_position(glm::ivec2 pos);
	TextBox &set_line_space(int line_space);
	TextBox &set_font_size(unsigned font_size);
	TextBox &set_font_face(FontFace font_face);
	TextBox &disable_animation();
	TextBox &set_animation(float speed, std::optional<std::function<void()>> callback);
	TextBox &show();
	void update(float elapsed);
	void draw();
	int get_height() const { return static_cast<int>((font_size_ + line_space_) * contents_.size()); }

private:
	FontFace font_face_ = FontFace::ComputerModernRegular;
	int line_space_ = 4;
	glm::ivec2 position_ = glm::ivec2(0, 0);
	unsigned font_size_ = 16;
	std::vector<std::pair<glm::u8vec4, std::string>> contents_;
	std::vector<std::shared_ptr<TextLine>> lines_;
	std::optional<float> animation_speed_;
	std::optional<std::function<void()>> callback_;
};

class Dialog {
public:
	Dialog(std::vector<std::pair<glm::u8vec4, std::string>> prompts, std::vector<std::string> options);
	void draw();
	void update(float elapsed);

	void MoveUp();
	void MoveDown();

	std::optional<int> Enter();

	bool finished() const;


private:
	void SetOptionFocus(int new_index);

	std::vector<std::pair<glm::u8vec4, std::string>> prompt_;
	std::vector<std::string> options_;
	int option_focus_ = 0;

	bool options_shown_ = false;

	std::shared_ptr<TextBox> prompt_box_;
	std::vector<std::pair<std::shared_ptr<TextLine>, std::shared_ptr<TextLine>>> option_lines_;
	static constexpr unsigned PADDING_LEFT = 25;
	static constexpr unsigned PADDING_TOP = 435;
};

}