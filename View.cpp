#include <string>
#include <iostream>
#include <utility>

#include "GL.hpp"
#include "gl_errors.hpp"

#include "View.hpp"
#include "Load.hpp"
#include "data_path.hpp"

namespace view {

struct RenderTextureProgram {
	// constructor and destructor: these are heavy weight functions
	// creating and freeing OpenGL resources
	RenderTextureProgram() {
		GLuint vs{0}, fs{0};

		const char *VERTEX_SHADER = ""
		                            "#version 410 core\n"
		                            "in vec4 in_Position;\n"
		                            "out vec2 texCoords;\n"
		                            "void main(void) {\n"
		                            "    gl_Position = vec4(in_Position.xy, 0, 1);\n"
		                            "    texCoords = in_Position.zw;\n"
		                            "}\n";

		const char *FRAGMENT_SHADER = ""
		                              "#version 410 core\n"
		                              "precision highp float;\n"
		                              "uniform sampler2D tex;\n"
		                              "uniform vec4 color;\n"
		                              "in vec2 texCoords;\n"
		                              "out vec4 fragColor;\n"
		                              "void main(void) {\n"
		                              "    fragColor = vec4(1, 1, 1, texture(tex, texCoords).r) * color;\n"
		                              "}\n";


		// Initialize shader
		vs = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vs, 1, &VERTEX_SHADER, 0);
		glCompileShader(vs);

		fs = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fs, 1, &FRAGMENT_SHADER, 0);
		glCompileShader(fs);

		program_ = glCreateProgram();
		glAttachShader(program_, vs);
		glAttachShader(program_, fs);
		glLinkProgram(program_);


		// Get shader uniforms
		glUseProgram(program_);
		glBindAttribLocation(program_, 0, "in_Position");
		tex_uniform_ = glGetUniformLocation(program_, "tex");
		color_uniform_ = glGetUniformLocation(program_, "color");
	}
	~RenderTextureProgram() {}
	GLuint program_ = 0;
	GLuint tex_uniform_ = 0;
	GLuint color_uniform_ = 0;
};

static Load<RenderTextureProgram> program(LoadTagEarly);

ViewContext ViewContext::singleton_{};

const ViewContext &ViewContext::get() {
	if (!singleton_.is_initialized) {
		std::cerr << "Accessing ViewContext singleton before initialization" << std::endl;
		std::abort();
	}
	return singleton_;
}
void ViewContext::set(const glm::uvec2 &logicalSize, const glm::uvec2 &drawableSize) {
	singleton_.logical_size_ = logicalSize;
	singleton_.drawable_size_ = drawableSize;
	singleton_.scale_factor_ = static_cast<float>(drawableSize.x) / logicalSize.x;
	singleton_.is_initialized = true;
}

TextLine::TextLine(std::string content,
                   float cursor_x,
                   float cursor_y,
                   glm::vec4 fg_color,
                   unsigned font_size,
                   std::optional<float> animation_speed,
                   bool visibility,
                   std::string font_face)
	: visibility_{visibility},
	  content_(std::move(content)),
	  cursor_x_{cursor_x},
	  cursor_y_{cursor_y},
	  fg_color_{fg_color},
	  font_size_{font_size},
	  animation_speed_{animation_speed},
	  font_face_{font_face}{

	// appear_by_letter_speed must be either empty or a positive float
	if (animation_speed.has_value() && animation_speed.value() <= 0.0f) {
		throw std::invalid_argument("appear_by_letter_speed must be either empty or a positive float");
	}
	scale_factor_ = get_scale_physical();

	// initialize opengl resources
	glGenBuffers(1, &vbo_);
	glGenVertexArrays(1, &vao_);
	glGenTextures(1, &texture_);
	glGenSamplers(1, &sampler_);
	glSamplerParameteri(sampler_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(sampler_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(sampler_, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glSamplerParameteri(sampler_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Set some initialize GL state
	glEnable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	FT_Error error = FT_Init_FreeType(&ft_library_);
	if (error != 0) { throw std::runtime_error("Error in initializing FreeType library"); }
	const std::string font_path = data_path(font_face_);
	error = FT_New_Face(ft_library_, font_path.c_str(), 0, &face_);
	if (error != 0) { throw std::runtime_error("Error initializing font face"); }
	error = FT_Set_Pixel_Sizes(face_, 0, ViewContext::compute_physical_px(font_size_));
	if (error != 0) { throw std::runtime_error("Error setting char size"); }
	font_ = hb_ft_font_create(face_, nullptr);
	assert(font_ != nullptr);
	hb_buffer_ = hb_buffer_create();
	if (hb_buffer_ == nullptr) { throw std::runtime_error("Error in creating harfbuzz buffer"); }
	setText(content_, animation_speed_);
}

TextLine &TextLine::setText(std::string content, std::optional<float> animation_speed) {
	this->animation_speed_ = animation_speed;
	this->content_ = std::move(content);
	hb_buffer_clear_contents(hb_buffer_);
	hb_buffer_add_utf8(hb_buffer_, content_.c_str(), -1, 0, -1);
	hb_buffer_set_direction(hb_buffer_, HB_DIRECTION_LTR);
	hb_buffer_set_script(hb_buffer_, HB_SCRIPT_LATIN);
	hb_buffer_set_language(hb_buffer_, hb_language_from_string("en", -1));

	hb_shape(font_, hb_buffer_, nullptr, 0);

	glyph_info_ = hb_buffer_get_glyph_infos(hb_buffer_, &glyph_count_);
	glyph_pos_ = hb_buffer_get_glyph_positions(hb_buffer_, &glyph_count_);

	if (animation_speed_.has_value()) {
		visible_glyph_count_ = 0;
	} else {
		visible_glyph_count_ = glyph_count_;
	}
	return *this;
}

TextLine::TextLine(const TextLine &that) : TextLine(that.content_,
                                                    that.cursor_x_,
                                                    that.cursor_y_,
                                                    that.fg_color_,
                                                    that.font_size_,
                                                    that.animation_speed_,
                                                    that.visibility_,
                                                    that.font_face_) {
	total_time_elapsed_ = that.total_time_elapsed_;
	visible_glyph_count_ = that.visible_glyph_count_;
	callback_ = that.callback_;
}

TextLine::~TextLine() {
	// TODO(xiaoqiao): release other resources: glyph_info_, glyph_pos_ ?
	hb_buffer_destroy(hb_buffer_);
	hb_buffer_ = nullptr;
	hb_font_destroy(font_);
	font_ = nullptr;
	FT_Done_Face(face_);
	face_ = nullptr;
	FT_Done_FreeType(ft_library_);
	ft_library_ = nullptr;

	glDeleteBuffers(1, &vbo_);
	glDeleteSamplers(1, &sampler_);
	glDeleteVertexArrays(1, &vao_);
	glDeleteTextures(1, &texture_);
}

void TextLine::update(float elapsed) {
	if (visibility_ && animation_speed_.has_value() && visible_glyph_count_ < glyph_count_) {
		// show "appear letters one by one" animation
		total_time_elapsed_ += elapsed;
		visible_glyph_count_ =
			std::min(static_cast<unsigned>(total_time_elapsed_ * animation_speed_.value()), glyph_count_);
		if (visible_glyph_count_ == glyph_count_ && callback_.has_value()) {
			(*callback_)();
		}
	}
}

void TextLine::draw() {
	if (!visibility_) { return; }
	// Bind Stuff
	GL_ERRORS();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_);
	glBindSampler(0, sampler_);
	glBindVertexArray(vao_);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_);
	glUseProgram(program->program_);
	glUniform4f(program->color_uniform_, fg_color_.x, fg_color_.y, fg_color_.z, fg_color_.w);
	glUniform1i(program->tex_uniform_, 0);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	GL_ERRORS();

	const FT_GlyphSlot glyph = face_->glyph;

	float cursor_x = cursor_x_, cursor_y = cursor_y_ - font_size_ * 2.0f / ViewContext::get().logical_size_.y;

	assert(visible_glyph_count_ <= glyph_count_);
	for (size_t i = 0; i < visible_glyph_count_; ++i) {
		hb_codepoint_t glyphid = glyph_info_[i].codepoint;
		float x_offset = glyph_pos_[i].x_offset / 64.0f;
		float y_offset = glyph_pos_[i].y_offset / 64.0f;
		float x_advance = glyph_pos_[i].x_advance / 64.0f;
		float y_advance = glyph_pos_[i].y_advance / 64.0f;


		if(FT_Load_Glyph(face_, glyphid, FT_LOAD_DEFAULT) != 0) {
			throw std::runtime_error("Error loading glyph");
			std::cout << "Error rendering glyph" << std::endl;
			continue;
		}

		if (FT_Render_Glyph(glyph, FT_RENDER_MODE_NORMAL) != 0) {
			throw std::runtime_error("Error rendering glyph");
			std::cout << "Error rendering glyph" << std::endl;
			continue;
		}

		GL_ERRORS();
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8,
		             glyph->bitmap.width, glyph->bitmap.rows,
		             0, GL_RED, GL_UNSIGNED_BYTE, glyph->bitmap.buffer);
		GL_ERRORS();

		const float vx = cursor_x + x_offset + glyph->bitmap_left * scale_factor_.x;
		const float vy = cursor_y + y_offset + glyph->bitmap_top * scale_factor_.y;
		const float w = glyph->bitmap.width * scale_factor_.x;
		const float h = glyph->bitmap.rows * scale_factor_.y;

		struct {
			float x, y, s, t;
		} data[6] = {
			{vx    , vy    , 0, 0},
			{vx    , vy - h, 0, 1},
			{vx + w, vy    , 1, 0},
			{vx + w, vy    , 1, 0},
			{vx    , vy - h, 0, 1},
			{vx + w, vy - h, 1, 1}
		};

		glBufferData(GL_ARRAY_BUFFER, 24*sizeof(float), data, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		cursor_x += x_advance * scale_factor_.x;
		cursor_y += y_advance * scale_factor_.y;
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	GL_ERRORS();
}


TextBox::TextBox(std::vector<std::pair<glm::uvec4, std::string>> contents,
                 const glm::ivec2 &position,
                 unsigned int fontSize,
                 std::optional<float> animation_speed)
	: position_(position), font_size_(fontSize), contents_{}, animation_speed_(std::nullopt) {
	set_contents(std::move(contents), animation_speed);
}

void TextBox::update(float elapsed) {
	for (auto &line : lines_) {
		line->update(elapsed);
	}
}

void TextBox::draw() {
	for (auto &line : lines_) {
		line->draw();
	}
}
void TextBox::set_contents(std::vector<std::pair<glm::uvec4, std::string>> contents, std::optional<float> animation_speed) {
	animation_speed_ = animation_speed;
	contents_ = std::move(contents);
	lines_.clear();
	if (animation_speed_.has_value()) {
		for (size_t i = 0; i < contents_.size(); i++) {
			lines_.push_back(std::make_shared<TextLine>(contents_.at(i).second,
			                    position_.x,
			                    position_.y + int(font_size_ * i),
			                    contents_.at(i).first,
			                    font_size_,
			                    animation_speed_,
			                    i == 0)); //< only make the first line initially visible
			if (i + 1 < contents_.size()) {
				lines_.at(i)->setAnimationCallback(std::make_optional([i, this]() {
					this->lines_.at(i + 1)->setVisibility(true);
				}));
			} else {
				lines_.at(i)->setAnimationCallback(callback_);
			}
		}
	} else {
		for (size_t i = 0; i < contents_.size(); i++) {
			lines_.push_back(std::make_shared<TextLine>(contents_.at(i).second,
			                    position_.x,
			                    position_.y + int(font_size_ * i),
			                    contents_.at(i).first,
			                    font_size_,
			                    std::nullopt,
			                    true));
		}
	}
}
Dialog::Dialog(std::vector<std::pair<glm::uvec4, std::string>> prompts, std::vector<std::string> options)
	: prompt_{prompts},
	  options_{options},
	  prompt_box_{
		  std::make_shared<TextBox>(prompts, glm::uvec2(PADDING_LEFT, PADDING_TOP), 16, std::make_optional(50.0f))} {
	for (size_t i = 0; i<options_.size(); i++) {
		int POS_Y = PADDING_TOP + prompt_box_->get_height() + 16 + i * 16;
		auto choice = std::make_shared<TextLine>("[ ]", PADDING_LEFT, POS_Y, glm::uvec4(255), 16, std::nullopt, false, "IBMPlexMono-Regular.ttf");
		auto text = std::make_shared<TextLine>(options_.at(i), PADDING_LEFT + 32, POS_Y, glm::uvec4(255), 16, std::nullopt, false);
		option_lines_.emplace_back(choice, text);
	}
	prompt_box_->set_callback([this]() {
		this->options_shown_ = true;
		for (auto &p : this->option_lines_) {
			p.first->setVisibility(true);
			p.second->setVisibility(true);
		}
	});
	if (!option_lines_.empty()) {
		option_lines_.at(option_focus_).first->setText("[x]", std::nullopt);
	}
}
}
