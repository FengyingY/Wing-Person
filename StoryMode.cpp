#include "StoryMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>

#include <hb.h>
#include <hb-ft.h>
#include <freetype/freetype.h>
#include <dirent.h>	
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

GLuint story_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > story_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("hexapod.pnct"));
	story_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > story_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("hexapod.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = story_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = story_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;
	});
});

Load< Sound::Sample > story_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("dusty-floor.opus"));
});


StoryMode::StoryMode() : scene(*story_scene), story(*jill_story) {
	// set the timer and print the first line
	current = story.stories["Opening"];
	timer_left = 1.0; // 1s per line

	show_next_line();

	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();
	
	//start music loop playing:
	// (note: position will be over-ridden in update())
	leg_tip_loop = Sound::loop_3D(*story_sample, 1.0f, glm::vec3(0, 0, 0), 10.0f);
}

StoryMode::~StoryMode() {
}

bool StoryMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (option) {
		int key = evt.key.keysym.sym - SDLK_1;
		if (key >= 0 && unsigned(key) < current.next_branch_names.size()) {
			current = story.stories[current.next_branch_names[key]];
			option = false;
			on_screen_options.clear();
			return true;
		}
		return false;
	}
	return false;
}

void StoryMode::update(float elapsed) {
	timer_left -= elapsed;
	if (timer_left <= 0) {
		show_next_line();
	}
}

void StoryMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	glUseProgram(0);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	scene.draw(*camera);

	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));

		constexpr float H = 0.09f;
		lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
			glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		float ofs = 2.0f / drawable_size.y;
		lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
			glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + + 0.1f * H + ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f),  glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));
	}
	GL_ERRORS();
}


bool StoryMode::show_next_line() {
	if (current.line_idx < current.lines.size()) {
		// show the current line on the screen
		Story::Line current_line = current.lines[current.line_idx];
		std::string to_show = story.characters[current_line.character_idx].first + " " + current_line.line;
		// reset timer - TODO set it according to the length of the sentence
		timer_left = 1.0;
		// go to next line
		current.line_idx += 1;

		// TODO show it on the screen
		std::cout << to_show << std::endl;
		return true;
	} else {
		if (current.option_lines.size() > 0) {
			if (!option) {
				option = true;
				assert(on_screen_options.size() == 0);
				for (size_t i = 0; i < current.option_lines.size(); ++i) {
					// TODO show options on screen
					std::string option = "\t" + std::to_string(i+1) + " " + current.option_lines[i];
					std::cout  << option << std::endl;
					on_screen_options.push_back(option);
				}
			}
			return true;
		}
	}
	return false;
}