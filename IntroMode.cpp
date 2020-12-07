#include "IntroMode.hpp"
#include "SDL_events.h"
#include "SDL_keycode.h"
#include "Sound.hpp"
#include "StoryMode.hpp"
#include "View.hpp"
#include "gl_errors.hpp"
#include "Sprite.hpp"

#include <chrono>
#include <thread>

Load< Sound::Sample > intro_background_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("menu/CR2_Jealousy_Lite_Loop.wav"));
});

Load< Sound::Sample > button_sound_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("menu/CGM3_Cute_Chirpy_Button_03_2.wav"));
});

Load< Sound::Sample > load_sound_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("menu/CGM3_Save_Load_02_2.wav"));
});

Sprite* menu_background, *menu_button, *menu_button_select, *menu_title, *menu_subtitle, *menu_button_text, *loading;
Load<void>load_menu_sprites(LoadTagDefault, []() -> void {
	menu_background = new Sprite(data_path("menu/HomeScreenBackground.png"), "background");
	menu_button = new Sprite(data_path("menu/MenuButton.png"), "button");
	menu_button_select = new Sprite(data_path("menu/MenuButtonPressed.png"), "button_select");
	menu_title = new Sprite(data_path("menu/Wing-Person.png"), "title");
	menu_subtitle = new Sprite(data_path("menu/Subtitle.png"), "subtitle");
	menu_button_text = new Sprite(data_path("menu/buttons_text.png"), "buttons_text");
	loading = new Sprite(data_path("menu/Loading_slots.png"), "loading");
});

IntroMode::IntroMode() {
	music_loop = Sound::loop(*intro_background_sample);
	GameSaveLoad::read();

	prompt = std::make_shared<view::TextLine>();
}

IntroMode::~IntroMode() {
}


bool IntroMode::handle_event(const SDL_Event &evt, const glm::uvec2 &window_size) {
	if ( evt.type == SDL_MOUSEMOTION ) {
		int x, y;
		SDL_GetMouseState( &x, &y );
		if (!loading_page_shown) {
			if (237 <= x && x <= 562) {
				if (346 <= y && y <= 393) {
					if (!start_selected) {
						Sound::play(*button_sound_sample);
					}
					start_selected = true;
					load_selected = false;
					tutorial_selected = false;
					return true;
				}
				if (426 <= y && y <= 473) {
					if (!load_selected) {
						Sound::play(*button_sound_sample);
					}
					start_selected = false;
					load_selected = true;
					tutorial_selected = false;
					return true;
				}
				if (506 <= y && y <= 553) {
					if (!tutorial_selected) {
						Sound::play(*button_sound_sample);
					}
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
	}

	if (evt.type == SDL_MOUSEBUTTONDOWN) {
		if (!loading_page_shown) {
			if (start_selected) {
				Sound::stop_all_samples();
				GameSaveLoad::write();
				Mode::set_current(std::make_shared<StoryMode>());
				return true;
			}
			if (load_selected) {
				GameSaveLoad::read();
				loading_page_shown = true;
				return true;
			}
			if (tutorial_selected) {
				// TODO build tutorial mode
				std::cout << "tutorial!" << std::endl;
				return true;
			}
		} else {
			int x, y;
			SDL_GetMouseState( &x, &y );

			// load from slots
			if (109 <= x && x <= 716) {
				size_t slot_idx = 3;
				if (169 <= y && y <= 253) {
					// slot 1
					slot_idx = 0;
				}
				if (304 <= y && y <= 400) {
					// slot 2
					slot_idx = 1;
				}
				if (438 <= y && y <= 528) {
					// slot 3
					slot_idx = 2;
				}

				if (slot_idx < 3) {
					std::shared_ptr< Sound::PlayingSample > sound = Sound::play(*load_sound_sample);
					unsigned long usec = 4LU * 100000LU;
					std::this_thread::sleep_for(std::chrono::microseconds(usec));
					GameSaveLoad::mtx.lock();
					GameStatus s = GameSaveLoad::slots[slot_idx];
					std::string branch_name = s.story_name;
					Character character = s.character;
					std::string background_music = s.background_music;
					GameSaveLoad::mtx.unlock();
					if (branch_name != "Empty") {
						Sound::stop_all_samples();
						GameSaveLoad::write();
						Mode::set_current(std::make_shared<StoryMode>(branch_name, character, background_music));
						return true;
					}
				}
			}

			// back button
			if (635 <= x && x <= 756 && 100 <= y && y <= 149) {
				load_selected = false;
				loading_page_shown = false;
				return true;
			}
		}
	}
	return false;
}

void IntroMode::update(float elapsed) {
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

	if (loading_page_shown) {
		loading->draw(center, drawable_size, 0.4f, 1.f);

		// text
		glDisable(GL_DEPTH_TEST);
		{
			for (int i = 0; i < 3; ++i) {
				std::vector<std::pair<glm::u8vec4, std::string> > contents;
				glm::vec4 color(0, 97, 146, 255);
				GameSaveLoad::mtx.lock();
				if (GameSaveLoad::slots[i].story_name != "Empty") {
					
					slot_info[i] = std::make_shared<view::TextLine>();
					slot_info[i]->set_font(view::FontFace::BUILT_BD)
								.set_text(GameSaveLoad::slots[i].info_line1())
								.set_font_size(20)
								.set_position(glm::vec2(230.5f, 200.f + i * 135.f - 15.f))
								.disable_animation()
								.set_visibility(true)
								.set_color(color);
					slot_info[i]->draw();

					slot_info[i+1] = std::make_shared<view::TextLine>();
					slot_info[i+1]->set_font(view::FontFace::BUILT_BD)
								.set_text(GameSaveLoad::slots[i].info_line2())
								.set_font_size(20)
								.set_position(glm::vec2(230.5f, 200.f + i * 135.f + 15.f))
								.disable_animation()
								.set_visibility(true)
								.set_color(color);
					slot_info[i+1]->draw();

				} else {
					slot_info[i] = std::make_shared<view::TextLine>();
					slot_info[i]->set_font(view::FontFace::BUILT_BD)
								.set_text("Empty")
								.set_font_size(30)
								.set_position(glm::vec2(230.5f, 200.f + i * 135.f))
								.disable_animation()
								.set_visibility(true)
								.set_color(color);
					slot_info[i]->draw();
				}
				GameSaveLoad::mtx.unlock();
				
			}
		}
	}
}