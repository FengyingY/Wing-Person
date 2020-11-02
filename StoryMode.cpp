#include "StoryMode.hpp"

#include "GL.hpp"
#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "Sound.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"
#include "load_save_png.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>

#include <hb.h>
#include <hb-ft.h>
#include <freetype/freetype.h>
#include <fstream>
#include <string>
#include <utility>


Load<Story> jill_story(LoadTagDefault, []() -> Story * {
	Story *ret = new Story();
	std::string path = data_path("script");
	std::ifstream script_file(path);
	std::string line;
	if (script_file.is_open()) {
		// the first character is for narration
		ret->characters.emplace_back(std::make_pair("", glm::vec4(1, 1, 1, 1)));
		// read character data, first line would be the number of characters
		std::string str_num_characters;
		getline(script_file, str_num_characters);
		int num_character = std::stoi(str_num_characters);
		// the following n lines are character data with format: character name r g b a
		std::string character_name;
		float r, g, b, a;
		while (num_character--) {
			script_file >> character_name >> r >> g >> b >> a;
			(ret->characters).emplace_back(std::make_pair(character_name, glm::vec4(r, g, b, a)));
		}

		// reading branches of the story
		while (getline(script_file, line)) {
			// first line is the name of story, create a key-value pair in stories map
			Story::Branch branch;
			std::string name = line;
			
			// read the lines and options in this branch
			while (true) {
				getline(script_file, line); 
				if (line.length() == 0) {
					// the branch is finished, go to the next one
					break;
				}
				size_t pos = line.find(".");
				int index = std::stoi(line.substr(0, pos));
				// this is a line
				if (index >= 0) {
					branch.lines.push_back(Story::Line(index, line.substr(pos+1, line.length() - pos)));
				} 
				// options
				else {
					int num_options = -index;
					while (num_options--) {
						std::string option_line, branch_name;
						getline(script_file, option_line);
						getline(script_file, branch_name);
						branch.option_lines.push_back(option_line);
						branch.next_branch_names.push_back(branch_name);
					}
				}
			}
			ret->stories[name] = branch;
		}
	}
	return ret;
});

Load< Sound::Sample > dusty_floor_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("dusty-floor.opus"));
});

Load< Sprite > female_sprite(LoadTagDefault, []() -> Sprite const * {
	return new Sprite("character_femaleAdventurer_hold.png");
});

Load< Sprite > zombie_sprite(LoadTagDefault, []() -> Sprite const * {
	return new Sprite("character_zombie_fallDown.png");
});

Load< Sprite > forest_background(LoadTagDefault, []() -> Sprite const * {
	return new Sprite("backgroundColorForest.png");
});

Load< Sprite > textbox_sprite(LoadTagDefault, []() -> Sprite const * {
	return new Sprite("textbox.png");
});

StoryMode::StoryMode() : story(*jill_story), girl(*female_sprite), zombie(*zombie_sprite), background(*forest_background), textbox(*textbox_sprite) {
	// set the timer and print the first line
	setCurrentBranch(story.stories.at("Opening"));

	music_loop = Sound::loop_3D(*dusty_floor_sample, 1.0f, glm::vec3(0, 0, 0));

}

StoryMode::~StoryMode() {
}

bool StoryMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		SDL_Keycode keyCode = evt.key.keysym.sym;
		if (keyCode == SDLK_UP) {
			main_dialog->MoveUp();
			return true;
		} else if (keyCode == SDLK_DOWN) {
			main_dialog->MoveDown();
			return true;
		} else if (keyCode == SDLK_RETURN) {
			if (main_dialog->finished()) {
				std::optional<int> next_branch = main_dialog->Enter();
				if (next_branch.has_value()) {
					setCurrentBranch(story.stories.at(current.next_branch_names.at(next_branch.value())));
					
					return true;
				}
			}
			return false;
		}
	}
	return false;
}

void StoryMode::update(float elapsed) {
	main_dialog->update(elapsed);
}

void StoryMode::draw(glm::uvec2 const &drawable_size) {
	//---- compute vertices to draw ----

	glClearColor(0.1f, 0.01f, 0.01f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	
	// glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	// glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	// glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



	glm::vec2 center = glm::vec2(drawable_size.x * 0.5f, drawable_size.y * 0.5);
	background.draw(center, drawable_size, 0.4f);
	girl.draw(glm::vec2(center.x*0.7, center.y*1.4), drawable_size, 0.6);
	zombie.draw(glm::vec2(center.x*1.3, center.y*1.4), drawable_size, 0.6);
	textbox.draw(glm::vec2(center.x, center.y*0.25), drawable_size, .21f);

	glDisable(GL_DEPTH_TEST);
	{
		main_dialog->draw();
	}
	
	GL_ERRORS();
}


bool StoryMode::show_next_line() {
	if (current.line_idx < current.lines.size()) {
		// show the current line on the screen
		Story::Line current_line = current.lines.at(current.line_idx);
		std::string to_show = story.characters.at(current_line.character_idx).first + " " + current_line.line;
		// reset timer - TODO set it according to the length of the sentence
		// go to next line
		current.line_idx += 1;

		// TODO show it on the screen
		std::cout << to_show << std::endl;
		return true;
	} else {
		if (current.option_lines.size() > 0) {
			if (!option) {
				option = true;
				for (size_t i = 0; i < current.option_lines.size(); ++i) {
					// TODO show options on screen
					std::string option = "\t" + std::to_string(i+1) + " " + current.option_lines[i];
					std::cout  << option << std::endl;
				}
			}
			return true;
		}
	}
	return false;
}

void StoryMode::setCurrentBranch(const Story::Branch &new_branch) {
	current = new_branch;
	option = true;
	std::vector<std::pair<glm::u8vec4, std::string>> prompts;
	for (const auto &line : current.lines) {
		glm::u8vec4 color = glm::u8vec4(story.characters.at(line.character_idx).second * 255.0f);
		std::string to_show = story.characters.at(line.character_idx).first + " " + line.line;
		prompts.emplace_back(color, to_show);
	}
	main_dialog = std::make_shared<view::Dialog>(prompts, current.option_lines);
}
