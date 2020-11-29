#include "IntroMode.hpp"
#include "SDL_events.h"
#include "SDL_keycode.h"
#include "StoryMode.hpp"
#include "View.hpp"
#include "gl_errors.hpp"
#include "Sprite.hpp"

bool IntroMode::handle_event(const SDL_Event &evt, const glm::uvec2 &window_size) {
	if ( evt.type == SDL_MOUSEMOTION ) {
		int x, y;
		SDL_GetMouseState( &x, &y );
		if (237 <= x && x <= 562) {
			if (346 <= y && y <= 393) {
				start_selected = true;
				load_selected = false;
				tutorial_selected = false;
				return true;
			}
			if (426 <= y && y <= 473) {
				start_selected = false;
				load_selected = true;
				tutorial_selected = false;
				return true;
			}
			if (506 <= y && y <= 553) {
				start_selected = false;
				load_selected = false;
				tutorial_selected = true;
				return true;
			}
		}
		start_selected = false;
		load_selected = false;
		tutorial_selected = false;
		return true;
	}

	if (evt.type == SDL_MOUSEBUTTONDOWN) {
		if (start_selected) {
			music_loop->stop();
			Mode::set_current(std::make_shared<StoryMode>());
		}
		if (load_selected) {
			// TODO load from slots
			std::cout << "loading!" << std::endl;
		}
		if (tutorial_selected) {
			// TODO build tutorial mode
			std::cout << "tutorial!" << std::endl;
		}
	}
	return false;
}

Load< Sound::Sample > intro_background_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("dusty-floor.opus"));
});

Sprite* menu_background, *menu_button, *menu_button_select, *menu_title, *menu_subtitle, *menu_button_text;
Load<void>load_menu_sprites(LoadTagDefault, []() -> void {
	menu_background = new Sprite(data_path("HomeScreenBackground.png"), "background");
	menu_button = new Sprite(data_path("MenuButton.png"), "button");
	menu_button_select = new Sprite(data_path("MenuButtonPressed.png"), "button_select");
	menu_title = new Sprite(data_path("Wing-Person.png"), "title");
	menu_subtitle = new Sprite(data_path("Subtitle.png"), "subtitle");
	menu_button_text = new Sprite(data_path("buttons_text.png"), "buttons_text");
});

IntroMode::IntroMode() {
	music_loop = Sound::loop_3D(*intro_background_sample, 1.0f, glm::vec3(0, 0, 0));
	headline_bg = std::make_shared<view::TextLine>();
	headline_bg->set_text("Wing-Person")
		.set_position(125, 150)
		.set_color(glm::u8vec4(255))
		.set_font(view::FontFace::BUILT_BD)
		.set_font_size(106)
		.disable_animation()
		.set_visibility(true);

	headline = std::make_shared<view::TextLine>();
	headline->set_text("Wing-Person")
		.set_position(135, 152)
		.set_color(glm::u8vec4(52, 172, 244, 255))
		.set_font(view::FontFace::BUILT_BD)
		.set_font_size(102)
		.disable_animation()
		.set_visibility(true);

	prompt = std::make_shared<view::TextLine>();
	prompt->set_text("Press enter to start...")
		.set_position(250, 500)
		.set_color(glm::u8vec4(255))
		.set_font_size(32)
		.disable_animation()
		.set_visibility(true);
}

void IntroMode::update(float elapsed) {
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDisable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.
	{
		if (headline) { headline->update(elapsed); }
		if (prompt) { prompt->update(elapsed); }
	}
	GL_ERRORS();
}

void IntroMode::draw(glm::uvec2 const &drawable_size) {
	glm::vec2 center = glm::vec2(drawable_size.x * 0.5f, drawable_size.y * 0.5f);
	menu_background->draw(center, drawable_size, 0.3f, 0.9f);
	if (start_selected) {
		menu_button_select->draw(glm::vec2(400.f, 230.f), drawable_size, 0.3f, 1.f);
	} else {
		menu_button->draw(glm::vec2(400.f, 230.f), drawable_size, 0.3f, 1.f);
	}
	if (load_selected) {
		menu_button_select->draw(glm::vec2(400.f, 150.f), drawable_size, 0.3f, 1.f);
	} else {
		menu_button->draw(glm::vec2(400.f, 150.f), drawable_size, 0.3f, 1.f);
	}
	if (tutorial_selected) {
		menu_button_select->draw(glm::vec2(400.f, 70.f), drawable_size, 0.3f, 1.f);
	} else {
		menu_button->draw(glm::vec2(400.f, 70.f), drawable_size, 0.3f, 1.f);
	}
	
	menu_title->draw(center, drawable_size, 0.45f, 1.f);
	menu_subtitle->draw(center, drawable_size, 0.45f, 1.f);
	menu_button_text->draw(center, drawable_size, 0.4f, 1.f);
}