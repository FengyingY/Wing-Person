#include "PuzzleMode.hpp"

PuzzleMode::PuzzleMode() {
	// TODO : Read level data and create platforms

	// #HACK : Creating a set of test platforms
	for (size_t i = 0; i < 5; i++)
	{
		PlatformTile *platform = new PlatformTile(glm::vec2(i * 50.0f, i * 30.0f), glm::vec2(20.0f  * (float)i, 20.0f));
		std::cout << platform->position.x << ", " << platform->position.y << std::endl;
		level_platforms.emplace_back(platform);
	}
}

PuzzleMode::~PuzzleMode() {

}

bool PuzzleMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
	
	if (evt.type == SDL_KEYDOWN)
	{
		// Put all key handling code here
	}
	

	return false;
}

void PuzzleMode::update(float elapsed) {

}

void PuzzleMode::draw(glm::uvec2 const &drawable_size) {
	glClearColor(1.0f, 0.01f, 0.01f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// tile->draw(drawable_size);
	// GL_ERRORS();
	for (auto &&platform : level_platforms)
	{
		platform->draw(drawable_size);
	}
	GL_ERRORS();
}