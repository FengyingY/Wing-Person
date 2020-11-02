#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"
#include "View.hpp"
#include "Sprite.hpp"
#include "PackDialogs.hpp"

#include <glm/glm.hpp>

#include <map>
#include <vector>
#include <deque>


struct Story {

	struct Dialog {
		std::vector<std::string> lines;

		// options, ending will have zero length options
		std::vector<std::string> option_lines;
		std::vector<std::string> next_branch_names;

		std::string character_name;	// character's name, nullable
	};

	// all the dialogs, key is the name of the dialog
	std::map<std::string, Dialog> dialog;

	// character's name and sprite
	std::map<std::string, Sprite> characters;
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

	Story::Dialog current;

	bool show_next_line();

	std::shared_ptr< Sound::PlayingSample > music_loop;

	Sprite girl;
	Sprite zombie;
	Sprite background;
	Sprite textbox;

private:
	void setCurrentBranch(const Story::Dialog &new_branch);

};
