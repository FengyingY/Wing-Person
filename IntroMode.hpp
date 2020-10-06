#include "Mode.hpp"
#include "SDL.h"
#include "glm/glm.hpp"
#include "View.hpp"

struct IntroMode : Mode {
	IntroMode();
	virtual ~IntroMode() = default;

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override {
		if (headline) { headline->draw(); }
		if (prompt) { prompt->draw(); }
	}

private:
	std::shared_ptr<view::TextLine> headline;
	std::shared_ptr<view::TextLine> prompt;
};
