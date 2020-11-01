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

	std::shared_ptr<view::Dialog> main_dialog = nullptr;


	// current status, true = option mode, ignore timer, waiting for player's input; false = story mode, keep showing the next line
	bool option = false;

	Story story;

	Story::Branch current;

	bool show_next_line();

	std::shared_ptr< Sound::PlayingSample > music_loop;

private:
	void setCurrentBranch(const Story::Branch &new_branch);

};
