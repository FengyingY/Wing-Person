#pragma once
#include <string>

#define CHARACTER_SELECT_BRANCH "Story14"       // the name of the dialog where player need to choose a character to date
#define P_INDEX 0                               // assume that the first option is the one prefer passion
#define R_INDEX 1                               // and the second prefer respect

#define STAGE0 0
#define STAGE1 2
#define STAGE2 4
#define STAGE3 6
#define STAGE4 8

struct Character {
	std::string name = "???";           // name of the character to date; assuming it's the same as the option line
	std::string preference = "N";	    // P: Preference, R: Respect, N: no preference
	std::string hate = "N";
	int affinity = 0;					// affinity value

	std::string affinity_string() {	// a value -> string map, can make it more funny :P
		if (affinity < STAGE0) {
            return ":(";
        } else if (affinity < STAGE1) {
            return "Stage 1";
        } else if (affinity < STAGE2) {
            return "Stage 2";
        } else if (affinity < STAGE3) {
            return "Stage 3";
        }  else if (affinity < STAGE4) {
            return "Stage 4";
        } else {
            return "Dating";
        }
	}
};