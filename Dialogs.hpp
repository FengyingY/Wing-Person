#include <string>
#include <vector>

// structure is based on https://github.com/lassyla/game4/blob/master/TextScene.hpp
struct Dialog {
    uint8_t id; 
    size_t num_choices; 
    size_t choice_start; 
    size_t text_start; 
    size_t text_length; 
};

struct Choice {
    uint8_t scene_id; 
    size_t text_start; 
    size_t text_length; 
};