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
#include <filesystem>
namespace fs = std::filesystem;

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
		std::string character = t.substr(start, d.character_length);
		dlg.character_name = character;

		start += d.character_length;
		std::string sprites = t.substr(start, d.sprite_length);
		split_string(sprites, "\n", dlg.sprites_name);

		start += d.sprite_length;
		dlg.background = t.substr(start, d.background_length);
		
		ret->dialog[name] = dlg;
    }

	return ret;
});

Load< Sound::Sample > dusty_floor_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("dusty-floor.opus"));
});

std::vector<Sprite*> sprites;
// load all of the sprite under folder 'dist/story_sprites'
Load< void > load_sprite(LoadTagDefault, []() -> void {
	// https://stackoverflow.com/questions/612097/how-can-i-get-the-list-of-files-in-a-directory-using-c-or-c
	for (const auto & entry : fs::directory_iterator(data_path("story_sprites"))) {
		std::string file_name = entry.path().filename().string();
		sprites.push_back(new Sprite(entry.path(), file_name.substr(0, file_name.find("."))));
	}
});


StoryMode::StoryMode() : story(*test_story) {
	// set the timer and print the first line
	setCurrentBranch(story.dialog.at("Opening"));

	music_loop = Sound::loop_3D(*dusty_floor_sample, 1.0f, glm::vec3(0, 0, 0));

	for (Sprite* s : sprites) {
		story.sprites[s->name] = s;
	}
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
	glClearColor(0.1f, 0.01f, 0.01f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glm::vec2 center = glm::vec2(drawable_size.x * 0.5f, drawable_size.y * 0.5f);
	
	// background
	if (current.background.length() > 0)
		story.sprites[current.background]->draw(center, drawable_size, 0.4f);

	// characters
	float offset = 1.f / (current.sprites_name.size() + 1);
	for (size_t i = 0; i < current.sprites_name.size(); ++i) {
		story.sprites[current.sprites_name[i]]->draw(glm::vec2(drawable_size.x * offset*(1.f+i), center.y*1.4f), drawable_size, 0.5f);
	}

	// textbox
	story.sprites["textbox"]->draw(glm::vec2(center.x, center.y*0.25f), drawable_size, .21f);

	// text
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
	glm::u8vec4 color = glm::u8vec4(255, 255, 255, 255);
	std::string to_show = current.character_name;
	if (to_show.length() > 0)
		prompts.emplace_back(color, to_show);

	for (const auto &line : current.lines) {
		to_show = " " + line;
		prompts.emplace_back(color, to_show);
	}
	main_dialog = std::make_shared<view::Dialog>(prompts, current.option_lines);
}
