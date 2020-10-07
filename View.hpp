#pragma once

#include <string>
#include <utility>
#include <vector>
#include <utility>
#include <optional>
#include <functional>
#include <iostream>
#include <memory>

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
	 * @param animation_speed if not null, appear glyphs one by one like an animation
	 *                               number means "how many new letters per second"
	 */
	TextLine(std::string content,
	         int cursor_x,
	         int cursor_y,
	         glm::uvec4 fg_color,
	         unsigned font_size,
	         std::optional<float> animation_speed,
	         bool visibility,
	         std::string font_face = "cmunorm.ttf") :
		TextLine{std::move(content),
		         static_cast<float>(cursor_x * (2.0f / ViewContext::get().logical_size_.x) - 1.0f),
		         static_cast<float>(-(cursor_y * (2.0f / ViewContext::get().logical_size_.y) - 1.0f)),
		         glm::vec4(fg_color) / 255.0f, font_size,
		         animation_speed,
		         visibility,
		         font_face} {}

	/**
	 * Create a line of text -- the more openGL friendly version
	 * @param content the string to be displayed.
	 * @param cursor_x position in window. Range: [-1.0f,1.0f]. -1 means left.
	 * @param cursor_y position in window. Range: [-1.0f,1.0f]. -1 means bottom.
	 * @param fg_color color, range: [0.0f, 1.0f]. 1.0 means highest brightness
	 * @param font_size font size in logical pixels (like CSS pixels)
	 * @param animation_speed if not null, appear glyphs one by one like an animation
	 *                               number means "how many new letters per second"
	 * @param visibility whether display the content or not
	 */
	TextLine(std::string content,
	         float cursor_x,
	         float cursor_y,
	         glm::vec4 fg_color,
	         unsigned font_size,
	         std::optional<float> animation_speed,
	         bool visibility,
	         std::string font_face = "cmunorm.ttf");

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
	 */
	TextLine(TextLine &&that) = delete;
	TextLine& operator=(TextLine &&) = delete;
	TextLine() = delete;
	~TextLine();

	/**
	 * Add a callback when the textLine is fully displayed
	 * @param cb the callback function
	 */
	TextLine& setAnimationCallback(std::optional<std::function<void()>> cb) {
		callback_ = cb;
		return *this;
	}

	TextLine& setVisibility(bool visibility) {
		visibility_ = visibility;
		return *this;
	}

	TextLine &setText(std::string content, std::optional<float> animation_speed);

	void update(float elapsed);
	void draw();

private:

	bool visibility_ = true;
	std::string content_;
	float cursor_x_;
	float cursor_y_;
	glm::vec4 fg_color_;
	unsigned font_size_; //< font size in "logical pixel"
	std::optional<float> animation_speed_;
	float total_time_elapsed_ = 0.0f;
	unsigned int visible_glyph_count_ = 0;
	/// callback_ called once when appear_by_letter_speed_.has_value() and all the glyphs are shown.
	std::optional<std::function<void()>> callback_ = std::nullopt;

	std::string font_face_;

	glm::vec2 scale_factor_;

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
	TextBox(std::vector<std::pair<glm::uvec4, std::string>> contents,
	        const glm::ivec2 &position,
	        unsigned int fontSize,
	        std::optional<float> animation_speed);
	void update(float elapsed);
	void draw();
	void set_contents(std::vector<std::pair<glm::uvec4, std::string>> contents, std::optional<float> animation_speed);
	int get_height() const { return static_cast<int>(font_size_ * contents_.size()); }
	void set_callback(std::optional<std::function<void()>> cb) {
		callback_ = cb;
		if (lines_.empty()) {
			return;
		}
		lines_.at(lines_.size() - 1)->setAnimationCallback(cb);
	}
private:
	// called when all letters displayed
	// precondition: animation_speed_ is not null
	std::optional<std::function<void()>> callback_;
	glm::ivec2 position_;
	unsigned font_size_;
	std::vector<std::pair<glm::uvec4, std::string>> contents_;
	std::vector<std::shared_ptr<TextLine>> lines_;
	std::optional<float> animation_speed_;
};

class Dialog {
public:
	Dialog(std::vector<std::pair<glm::uvec4, std::string>> prompts, std::vector<std::string> options);
	void draw() {
		prompt_box_->draw();
		for (auto&[choice, text] : option_lines_) {
			choice->draw();
			text->draw();
		}
	}
	void update(float elapsed) {
		prompt_box_->update(elapsed);
	}

	void MoveUp() {
		if (options_shown_ && !options_.empty()) {
			SetOptionFocus(std::max<int>(option_focus_ - 1, 0));
		}
	}
	void MoveDown() {
		if (options_shown_ && !options_.empty()) {
			SetOptionFocus(std::min<int>(option_focus_ + 1, (int)options_.size() - 1));
		}
	}

	std::optional<int> Enter() {
		if (options_shown_ && !options_.empty()) {
			return std::make_optional(option_focus_);
		} else {
			return std::nullopt;
		}
	}

	bool finished() {
		return options_shown_;
	}


private:
	void SetOptionFocus(int new_index) {
		if (option_focus_ != new_index) {
			option_lines_.at(option_focus_).first->setText("[ ]", std::nullopt);
			option_lines_.at(new_index).first->setText("[x]", std::nullopt);
			option_focus_ = new_index;
		}
	}

	std::vector<std::pair<glm::uvec4, std::string>> prompt_;
	std::vector<std::string> options_;
	int option_focus_ = 0;

	bool options_shown_ = false;

	std::shared_ptr<TextBox> prompt_box_;
	std::vector<std::pair<std::shared_ptr<TextLine>, std::shared_ptr<TextLine>>> option_lines_;
	static constexpr unsigned PADDING_LEFT = 16;
	static constexpr unsigned PADDING_TOP = 16;
};

}