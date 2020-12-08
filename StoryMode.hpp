#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"
#include "View.hpp"
#include "Sprite.hpp"
#include "PackDialogs.hpp"
#include "GameLoadSave.hpp"
#include "Character.hpp"

#include <glm/glm.hpp>

#include <map>
#include <string>
#include <vector>
#include <deque>

#define STORY_SPRITE_DIR "story_sprites"
#define STORY_MUSIC_DIR "story_music"

struct Story {

	struct Dialog {
		std::string dlg_name;
		std::vector<std::string> lines;

		// options, ending will have zero length options
		std::vector<std::string> option_lines;
		std::vector<std::string> next_branch_names;
		std::vector<std::string> option_line_preference;

		std::string character_name;	// character's name, nullable
		std::vector<std::string> sprites_name; // name of the sprites to be shown in this dialog
		std::string background;		// name of the background sprite
		std::string background_music;	// file name of the background music
		std::string sound;	// file name of the sound effect
	};

	// all the dialogs, key is the name of the dialog
	std::map<std::string, Dialog> dialog;

};

struct StoryMode : Mode {
	StoryMode();
	StoryMode(std::string branch_name);	 // deprecated
	StoryMode(std::string branch_name, Character character, std::string background_music);
	virtual ~StoryMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----
	Character character;

	// Story control
	Story story;
	Story::Dialog current;
	bool show_next_line();

	// Text & UI
	std::shared_ptr<view::TextLine> character_name = nullptr;
	std::shared_ptr<view::TextLine> slot_info[6];
	std::shared_ptr<view::Dialog> main_dialog = nullptr;
	std::shared_ptr<view::TextLine> happiness_status = nullptr;
	std::shared_ptr<view::TextLine> respect_status = nullptr;
	bool menu_selected = false, load_selected = false, save_selected = false, loading_page_shown = false, ended = false;
	// current status, true = option mode, ignore timer, waiting for player's input; false = story mode, keep showing the next line
	bool option = false;

	// music 
	std::string background_music = "";					// current background music file name
	std::shared_ptr< Sound::PlayingSample > music_loop;	// background music
	std::shared_ptr< Sound::PlayingSample > sound;		// play once
	
	// UI sound
	std::shared_ptr< Sound::PlayingSample > button_sound;
	std::shared_ptr< Sound::PlayingSample > save_load_sound;

private:
	void setCurrentBranch(const Story::Dialog &new_branch);

};
