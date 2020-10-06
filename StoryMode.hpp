#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"
#include "View.hpp"

#include <glm/glm.hpp>

#include <map>
#include <vector>
#include <deque>

struct Story {
	struct Line {
		Line(int idx_, std::string line_) : character_idx(idx_), line(line_) {};
		size_t character_idx;
		std::string line;
	};

	struct Branch {
		std::vector<Line> lines;
		size_t line_idx = 0;        // current line index

		// options, ending will have zero length options
		std::vector<std::string> option_lines;
		std::vector<std::string> next_branch_names;
	};

	// all the branches, key is the name of the branches,
	std::map<std::string, Branch> stories;

	// character's name and line's color
	std::vector<std::pair<std::string, glm::vec4>> characters;
};

struct StoryMode : Mode {
	StoryMode();
	virtual ~StoryMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	view::TextBox my_text_box{
		std::vector<std::pair<glm::uvec3, std::string>>{
			{glm::uvec3(255, 0, 0), "Hiiiiiiiiiiiiiii"},
			{glm::uvec3(0, 255, 255), "I have colors"},
			{glm::uvec3(255, 255, 255), "Make America Great Again"}

		},
		glm::ivec2{32, 32},
		32,
		std::make_optional(50.0f)
	};

	// timer -> if time <= 0:,then do 1) print next line, 2) reset timer, 3) update state if needed
	float timer_left;

	// current status, true = option mode, ignore timer, waiting for player's input; false = story mode, keep showing the next line
	bool option = false;

	Story story;

	Story::Branch current;

	std::vector<std::pair<glm::vec3, std::string>> on_screen_lines;    // the lines that would be showed on screen
	std::vector<std::string> on_screen_options;    // for showing the options (cleared after selection)
	bool show_next_line();


};
