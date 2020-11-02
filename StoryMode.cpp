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
#include "read_write_chunk.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>

#include <hb.h>
#include <hb-ft.h>
#include <freetype/freetype.h>
#include <fstream>
#include <string>
#include <utility>

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
		std::string characters = t.substr(start, d.character_length);
		dlg.character_name = characters; // TODO multiple characters?
		
		ret->dialog[name] = dlg;
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

StoryMode::StoryMode() : story(*test_story), girl(*female_sprite), zombie(*zombie_sprite), background(*forest_background), textbox(*textbox_sprite) {
	// set the timer and print the first line
	setCurrentBranch(story.dialog.at("Opening"));

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
					setCurrentBranch(story.dialog.at(current.next_branch_names.at(next_branch.value())));
					
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


void StoryMode::setCurrentBranch(const Story::Dialog &new_dialog) {
	current = new_dialog;
	option = true;
	std::vector<std::pair<glm::u8vec4, std::string>> prompts;
	for (const auto &line : current.lines) {
		glm::u8vec4 color = glm::u8vec4(255, 255, 255, 255);//glm::u8vec4(story.characters.at(line.character_idx).second * 255.0f);
		std::string to_show = " " + line;
		prompts.emplace_back(color, to_show);
	}
	main_dialog = std::make_shared<view::Dialog>(prompts, current.option_lines);
}
