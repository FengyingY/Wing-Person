#include "IntroMode.hpp"
#include "StoryMode.hpp"
#include "gl_errors.hpp"

bool IntroMode::handle_event(const SDL_Event &evt, const glm::uvec2 &window_size) {
	if (evt.type == SDL_KEYDOWN) {
		SDL_Keycode keyCode = evt.key.keysym.sym;
		if (keyCode == SDLK_RETURN) {
			Mode::set_current(std::make_shared<StoryMode>());
			return true;
		}
	}
	return false;
}
IntroMode::IntroMode() {
	this->headline = std::make_shared<view::TextLine>("Mario's", 250, 150, glm::uvec4(255), 96, std::nullopt, true);
	this->prompt = std::make_shared<view::TextLine>("Press enter to start...", 250, 500, glm::uvec4(255), 32, std::nullopt, true);
}
void IntroMode::update(float elapsed) {
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.
	{
		if (headline) { headline->update(elapsed); }
		if (prompt) { prompt->update(elapsed); }
	}
	GL_ERRORS();
}
