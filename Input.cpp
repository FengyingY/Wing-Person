#include "Input.hpp"

int Input::downs() {
  return _downs;
}

bool Input::held() {
  return _held;
}

bool Input::pressed() {
  return _pressed;
}

bool Input::released() {
  return _released;
}

bool Input::just_pressed() {
  return _held && !_last_held;
}

bool Input::just_released() {
  return !_held && _last_held;
}

glm::vec2 InputManager::mouse_position_abs() {
  return glm::vec2(_mouse_x, _mouse_y);
}

glm::vec2 InputManager::mouse_position_rel() {
  //TODO adjust based on window size
  return glm::vec2(_mouse_x, _mouse_y);
}

Input* InputManager::register_key(SDL_Keycode key) {
  if (_key_inputs.emplace(key, Input()).second) {
    return &_key_inputs.at(key);
  } else {
    return nullptr;
  }
}

Input* InputManager::register_mouse_button(uint8_t button) {
  if (_mouse_inputs.emplace(button, Input()).second) {
    return &_mouse_inputs.at(button);
  } else {
    return nullptr;
  }
}

bool InputManager::handle_event(SDL_Event const& evt) {
  switch(evt.type) {
    case SDL_MOUSEMOTION:
      _mouse_x = evt.motion.x;
      _mouse_y = evt.motion.y;
      return true;

    case SDL_KEYDOWN:
      if(_key_inputs.find(evt.key.keysym.sym) != _key_inputs.end()) {
          Input& input = _key_inputs.at(evt.key.keysym.sym);

          input._downs++;
          input._held = true;
          input._pressed = true;
          return true;
      }
      break;

    case SDL_KEYUP:
      if(_key_inputs.find(evt.key.keysym.sym) != _key_inputs.end()) {
          Input& input = _key_inputs.at(evt.key.keysym.sym);

          input._held = false;
          input._released = true;
          return true;
      }
      break;

    case SDL_MOUSEBUTTONDOWN:
      if(_mouse_inputs.find(evt.button.button) != _mouse_inputs.end()) {
          Input& input = _mouse_inputs.at(evt.button.button);

          input._downs++;
          input._held = true;
          input._pressed = true;
          return true;
      }
      break;

    case SDL_MOUSEBUTTONUP:
      if(_mouse_inputs.find(evt.button.button) != _mouse_inputs.end()) {
          Input& input = _mouse_inputs.at(evt.button.button);

          input._held = false;
          input._released = true;
          return true;
      }
      break;
  }

  return false;
}

void InputManager::tick() {
  for (auto&[key, input] : _key_inputs) {
    (void) key;

    input._downs = 0;
    input._pressed = false;
    input._released = false;
    input._last_held = input._held;
  }

  for (auto&[key, input] : _mouse_inputs) {
    (void) key;

    input._downs = 0;
    input._pressed = false;
    input._released = false;
    input._last_held = input._held;
  }
}
