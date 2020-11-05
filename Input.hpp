#pragma once

#include <glm/glm.hpp>
#include <SDL.h>
#include <unordered_map>

class Input {
  friend class InputManager;

  public:
    // Number of presses since last tick
    int downs();

    // Whether the key is currently being held
    bool held();

    // Whether the key was pressed this tick
    // Note that key repeats will repeatedly trigger this when the button is
    // held
    bool pressed();
    // Whether the key was released this tick
    bool released();

    // Versions of pressed/released that depend on whether the held state
    // was changed instead of reading button inputs
    bool just_pressed();
    bool just_released();

  private:
    int _downs;
    bool _held;
    bool _last_held;
    bool _pressed;
    bool _released;
};

class InputManager {
  typedef uint8_t InputId;
  public:
    // Creates a Input
    // Returns nullptr if the key or mouse button is already in use
    Input* register_key(SDL_Keycode key);
    Input* register_mouse_button(uint8_t button);

    // Handles an SDL Event
    bool handle_event(SDL_Event const& evt);

    // Cleans the state of the InputManager after a game tick. Should be
    // called at the end of a game tick (after collecting all necessary
    // inputs)
    void tick();

    glm::vec2 mouse_position_abs();
    glm::vec2 mouse_position_rel();

  private:
    std::unordered_map<SDL_Keycode, Input> _key_inputs;
    std::unordered_map<uint8_t, Input> _mouse_inputs;

    float _mouse_x;
    float _mouse_y;
};
