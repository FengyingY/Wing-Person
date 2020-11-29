#include "StoryMode.hpp"

#include "GL.hpp"
#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "SDL_keycode.h"
#include "Sound.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"
#include "load_save_png.hpp"
#include "read_write_chunk.hpp"
#include "IntroMode.hpp"
#include "PuzzleMode.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>

#include <hb.h>
#include <hb-ft.h>
#include <freetype/freetype.h>
#include <fstream>
#include <string>
#include <utility>
#if defined(__linux__)
#include <dirent.h>
DIR *dpdf;
struct dirent *epdf;
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif

void split_string(std::string s, std::string delimiter, std::vector<std::string>&vec) {
	// ref: https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
	size_t start = 0;
	size_t end = s.find(delimiter);
	while (end != std::string::npos) {
		vec.push_back(s.substr(start, end-start));
		start = end + delimiter.length();
		end = s.find(delimiter, start);
	}
	if (end != std::string::npos)
		vec.push_back(s.substr(start, end));
}

Load<Story> test_story(LoadTagDefault, []() -> Story * {
	Story *ret = new Story();
	// read chunk
	std::vector<char> texts;
    std::vector<DialogChunk> dialog_chunk;

    std::ifstream in(data_path("dialogs.dat"), std::ios::binary);
    read_chunk(in, "txts", &texts);
    read_chunk(in, "dlgs", &dialog_chunk);

	std::string t(texts.begin(), texts.end());
	
    for (DialogChunk d : dialog_chunk) {
		Story::Dialog dlg;
		size_t start = d.dialog_start;
		std::string name = t.substr(start, d.name_length);
		dlg.dlg_name = name;

		start += d.name_length;
		std::string text = t.substr(start, d.text_length);
		split_string(text, "\n", dlg.lines);

		start += d.text_length;
		std::string choices = t.substr(start, d.choice_length);
		std::vector<std::string> choice_target_pairs;
		split_string(choices, "\n", choice_target_pairs);

		for (std::string choice_target : choice_target_pairs) {
			size_t pos = choice_target.find(":");
			dlg.option_lines.push_back(choice_target.substr(0, pos));
			dlg.next_branch_names.push_back(choice_target.substr(pos+1, choice_target.length() - pos));
		}

		start += d.choice_length;
		std::string character = t.substr(start, d.character_length);
		dlg.character_name = character;

		start += d.character_length;
		std::string sprites = t.substr(start, d.sprite_length);
		split_string(sprites, "\n", dlg.sprites_name);

		start += d.sprite_length;
		dlg.background = t.substr(start, d.background_length);
		
		ret->dialog[name] = dlg;
    }

	Story::Dialog disagree_dlg;
	disagree_dlg.dlg_name = "disagree";
	disagree_dlg.lines.push_back("Too bad, please discuss and make an agreement.");
	disagree_dlg.option_lines.push_back("back");
	ret->dialog["disagree"] = disagree_dlg;

	return ret;
});

Load< Sound::Sample > dusty_floor_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("dusty-floor.opus"));
});

// images
std::map<std::string, Sprite*> story_sprites;
// load all of the sprite under folder 'dist/story_sprites'
Load< void > load_sprite(LoadTagDefault, []() -> void {
	#if defined(__linux__)
	std::string path = data_path("story_sprites");
	dpdf = opendir(path.c_str());
	if (dpdf != NULL) {
		while ((epdf = readdir(dpdf))) {
			std::string file_name = epdf->d_name;
			if (file_name != "." && file_name != "..") {
				std::string sprite_name = file_name.substr(0, file_name.find("."));
				story_sprites[sprite_name] = (new Sprite(path + "/" + file_name, sprite_name));
			}
		}
	}
	#else
	// https://stackoverflow.com/questions/612097/how-can-i-get-the-list-of-files-in-a-directory-using-c-or-c
	for (const auto & entry : fs::directory_iterator(data_path("story_sprites"))) {
		std::string file_name = entry.path().filename().string();
		// sprites.push_back(new Sprite(entry.path().string(), file_name.substr(0, file_name.find("."))));
		std::string sprite_name = file_name.substr(0, file_name.find("."));
		story_sprites[sprite_name] = (new Sprite(entry.path().string(), sprite_name));
	}
	#endif
});


StoryMode::StoryMode() : story(*test_story) {
	setCurrentBranch(story.dialog.at("Opening"));

	music_loop = Sound::loop_3D(*dusty_floor_sample, 1.0f, glm::vec3(0, 0, 0));	
	GameSaveLoad::read();
}

// go back to story mode at the specified branch
StoryMode::StoryMode(std::string branch_name) : story(*test_story) {
	story = *test_story;
	setCurrentBranch(story.dialog.at(branch_name));
	GameSaveLoad::read();
}


StoryMode::~StoryMode() {
	GameSaveLoad::write();
}

bool StoryMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (!loading_page_shown) {
		if (evt.type == SDL_KEYDOWN) {
			SDL_Keycode keyCode = evt.key.keysym.sym;
			if (keyCode == SDLK_UP) {
				main_dialog->MoveUp();
				return true;
			} else if (keyCode == SDLK_DOWN) {
				main_dialog->MoveDown();
				return true;
			} else if (keyCode == SDLK_w) {
				main_dialog->MoveUp2();
				return true;
			} else if (keyCode == SDLK_s) {
				main_dialog->MoveDown2();
				return true;
			}
			else if (keyCode == SDLK_RETURN) {
				if (main_dialog->finished()) {
					if (main_dialog->agree()) {
						std::optional<int> next_branch = main_dialog->Enter();
						if (next_branch.has_value()) {
							std::string next_branch_name = current.next_branch_names.at(next_branch.value());
							if (next_branch_name == "PuzzleMode") {
								// jump to the puzzle mode
								// TODO using the introMode for testing, please change it to PuzzleMode at intergration
								Mode::set_current(std::make_shared<PuzzleMode>());
							} else {
								setCurrentBranch(story.dialog.at(current.next_branch_names.at(next_branch.value())));
							}
						}
					} else {
						// disagree
						story.dialog["disagree"].background = current.background;
						story.dialog["disagree"].character_name = current.character_name;
						if (story.dialog["disagree"].next_branch_names.empty()) {
							story.dialog["disagree"].next_branch_names.push_back(current.dlg_name);
						} else {
							story.dialog["disagree"].next_branch_names[0] = current.dlg_name;
						}
						setCurrentBranch(story.dialog["disagree"]);
					}
				} else {
					// text skip
					main_dialog->show_all_text();
					return true;
				}
				return false;
			}
		}
	}

	if ( evt.type == SDL_MOUSEMOTION ) {
		int x, y;
		SDL_GetMouseState( &x, &y );
		
		if (!loading_page_shown) {
			if (y <= 30.f) {
				if (620 <= x && x < 680) {
					save_selected = true;
					load_selected = false;
					menu_selected = false;
					return true;
				} else if (680 <= x && x < 740) {
					save_selected = false;
					load_selected = true;
					menu_selected = false;
					return true;
				} else if (740 <= x) {
					save_selected = false;
					load_selected = false;
					menu_selected = true;
					return true;
				}
			}
			save_selected = false;
			load_selected = false;
			menu_selected = false;
			return true;
		}
	}

	if (evt.type == SDL_MOUSEBUTTONDOWN) {
		if (!loading_page_shown) {
			if (save_selected) {
				// TODO save the current status
				std::cout << "saved!" << std::endl;
				loading_page_shown = true;
				return true;
			}
			if (load_selected) {
				// TODO load from selected slot
				std::cout << "loading!" << std::endl;
				loading_page_shown = true;
				return true;
			}
			if (menu_selected) {
				Sound::stop_all_samples();
				Mode::set_current(std::make_shared<IntroMode>());
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
					if (save_selected) {
						GameSaveLoad::save(current.dlg_name, slot_idx);
						return true;
					}
					if (load_selected) {
						GameSaveLoad::mtx.lock();
						std::string branch_name = GameSaveLoad::slots[slot_idx].story_name;
						GameSaveLoad::mtx.unlock();
						if ( story.dialog.find(branch_name) != story.dialog.end() ) {
							loading_page_shown = false;
							load_selected = false;
							setCurrentBranch(story.dialog.at(branch_name));
							return true;
						} else {
							std::cout << "Can't find branch: " << branch_name << std::endl;
						}
					}
					std::cout << "Neither save or load selected" << std::endl;
				}
			}

			// back button
			if (635 <= x && x <= 756 && 100 <= y && y <= 149) {
				save_selected = false;
				load_selected = false;
				loading_page_shown = false;
				return true;
			}
		}
	}

	return false;
}

void StoryMode::update(float elapsed) {
	main_dialog->update(elapsed);
}

void StoryMode::draw(glm::uvec2 const &drawable_size) {
	glClearColor(0.1f, 0.01f, 0.01f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// glm::vec2 center = glm::vec2(drawable_size.x * 0.5f, drawable_size.y * 0.5f);
	glm::vec2 center(400.f, 300.f);
	
	// background
	if (current.background.length() > 0)
		story_sprites[current.background]->draw(center, drawable_size, 0.4f, 1.0f);

	// characters
	float offset = 1.f / (current.sprites_name.size() + 1);
	for (size_t i = 0; i < current.sprites_name.size(); ++i) {
		story_sprites[current.sprites_name[i]]->draw(glm::vec2(drawable_size.x * offset*(1.f+i), center.y), drawable_size, 0.3f, 1.0f);
	}

	// textbox
	// story_sprites["textbox"]->draw(glm::vec2(center.x, center.y*0.25f), drawable_size, .21f, 1.0f);
	story_sprites["ui"]->draw(center, drawable_size, 0.5f, 1.0f);

	// buttons
	if (menu_selected) {
		story_sprites["Menu_select"]->draw(glm::vec2(800-30, 600-15), drawable_size, 0.5f, 1.f);
	} else {
		story_sprites["Menu"]->draw(glm::vec2(800-30, 600-15), drawable_size, 0.5f, 1.f);
	}
	if (load_selected) {
		story_sprites["Load_select"]->draw(glm::vec2(800-30*3, 600-15), drawable_size, 0.5f, 1.f);
	} else {
		story_sprites["Load"]->draw(glm::vec2(800-30*3, 600-15), drawable_size, 0.5f, 1.f);
	}
	if (save_selected) {
		story_sprites["Save_select"]->draw(glm::vec2(800-30*5, 600-15), drawable_size, 0.5f, 1.f);
	} else {
		story_sprites["Save"]->draw(glm::vec2(800-30*5, 600-15), drawable_size, 0.5f, 1.f);
	}

	// text
	glDisable(GL_DEPTH_TEST);
	{
		character_name->draw();
		main_dialog->draw();
	}

	if (loading_page_shown) {
		story_sprites["story_loading"]->draw(center, drawable_size, 0.4f, 1.f);
		// text
		glDisable(GL_DEPTH_TEST);
		{
			for (int i = 0; i < 3; ++i) {
				slot_info[i] = std::make_shared<view::TextBox>();
				std::vector<std::pair<glm::u8vec4, std::string> > contents;
				GameSaveLoad::mtx.lock();
				if (GameSaveLoad::slots[i].story_name != "Empty") {
					contents.emplace_back(glm::vec4(255), "Story: " + GameSaveLoad::slots[i].story_name);
					contents.emplace_back(glm::vec4(255), "Time: " + GameSaveLoad::slots[i].save_time);
				} else {
					contents.emplace_back(glm::vec4(255), "Empty");
				}
				GameSaveLoad::mtx.unlock();
				
				slot_info[i]->set_font_face(view::FontFace::BUILT_BD)
							.set_contents(contents)
							.set_font_size(22)
							.set_position(glm::vec2(230.5f, 200.f + i * 135.f))
							.disable_animation()
							.show();
				slot_info[i]->draw();
			}
		}
	}
	
	GL_ERRORS();
}


void StoryMode::setCurrentBranch(const Story::Dialog &new_dialog) {
	current = new_dialog;
	option = true;
	std::vector<std::pair<glm::u8vec4, std::string>> prompts;
	glm::u8vec4 color = glm::u8vec4(255, 255, 255, 255);
	std::string to_show = current.character_name;
	if (to_show.length() > 0) {
		// prompts.emplace_back(color, to_show);
		character_name = std::make_shared<view::TextLine>();
		character_name->set_font(view::FontFace::BUILT_BD)
					.set_text(current.character_name)
					.set_font_size(28)
					.set_position(glm::vec2(21.5f, 418.f))
					.set_color(glm::u8vec4(0, 0, 0, 255))
					.disable_animation()
					.set_visibility(true);
	}
		
	for (const auto &line : current.lines) {
		to_show = " " + line;
		prompts.emplace_back(color, to_show);
	}
	main_dialog = std::make_shared<view::Dialog>(prompts, current.option_lines);
}
