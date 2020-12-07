#pragma once
#ifdef _WIN32
#pragma warning(disable : 4996)
#endif

#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <chrono>
#include <ctime>
#include <mutex>
#include <vector>

#include "data_path.hpp"
#include "Character.hpp"

#define PLAYER_SLOT_COUNT 3
#define DELIMITER "###"

struct GameStatus {
    std::string story_name = "";
    Character character;
    std::string save_time = "";

    std::string background_music = "";

    // always show 2 lines per slot on the UI
    std::string info_line1() {
        return story_name + ", " + character.name + ", " + character.affinity_string();
    }

    std::string info_line2() {
        return "Saved Time: " + save_time;
    }
};
namespace {
struct GameSaveLoad {

    static GameStatus slots[PLAYER_SLOT_COUNT];
    static std::mutex mtx;

    GameSaveLoad() {
    }

    ~GameSaveLoad() {
    }

    static void read() {
        // read from file
        mtx.lock();
        size_t count = 0;
        std::ifstream infile(data_path("saved_games"));
        std::string line;
        while (std::getline(infile, line)) {
            if (count == PLAYER_SLOT_COUNT) break;

            line += DELIMITER;
            std::vector<std::string>vec;
            size_t start = 0;
            size_t end = line.find(DELIMITER);
            while (end != std::string::npos) {
                vec.push_back(line.substr(start, end-start));
                start = end + std::string(DELIMITER).length();
                end = line.find(DELIMITER, start);
            }
            if (end != std::string::npos)
                vec.push_back(line.substr(start, end));

            if (vec.size() < 6) {
                slots[count].story_name = "Empty";
            } else {
                slots[count].story_name = vec[0];
                slots[count].background_music = vec[1];
                slots[count].character.name = vec[2];
                slots[count].character.preference = vec[3];
                slots[count].character.hate = vec[4];
                slots[count].character.affinity = std::stoi(vec[5]);
                slots[count].save_time = vec[6];
            }
            count++;
        }

        for (; count < PLAYER_SLOT_COUNT; count++) {
            slots[count].story_name = "Empty";
        }

        infile.close();
        mtx.unlock();

        std::cout << "Read from file" << std::endl;
        print_status();
    }

    static void write() {

        std::cout << "Wrote to file" << std::endl;
        print_status();

        // write current status to file
        mtx.lock();
        std::ofstream ofs(data_path("saved_games"), std::ofstream::trunc);  // overwrite
        for (GameStatus s : slots) {
            ofs << s.story_name << DELIMITER 
                << s.background_music << DELIMITER
                << s.character.name << DELIMITER
                << s.character.preference << DELIMITER
                << s.character.hate << DELIMITER
                << s.character.affinity << DELIMITER
                << s.save_time << "\n";
        }
        ofs.close();
        mtx.unlock();

    }

    static void save(std::string dlg_name, std::string background_music, Character character, size_t slot_idx) {
        // Ref: https://stackoverflow.com/questions/997946/how-to-get-current-time-and-date-in-c
        if (slot_idx < 3) {
            mtx.lock();

            // story info
            slots[slot_idx].story_name = dlg_name;
            slots[slot_idx].background_music = background_music;

            // character info
            slots[slot_idx].character.name = character.name;
            slots[slot_idx].character.preference = character.preference;
            slots[slot_idx].character.hate = character.hate;
            slots[slot_idx].character.affinity = character.affinity;

            // time
            auto time = std::chrono::system_clock::now();
            std::time_t save_timestamp = std::chrono::system_clock::to_time_t(time);
            slots[slot_idx].save_time = std::ctime(&save_timestamp);
            slots[slot_idx].save_time = slots[slot_idx].save_time.substr(0, slots[slot_idx].save_time.length()-1);  // remove the newline at the end

            mtx.unlock();
        }
    }

    static void print_status() {
        mtx.lock();
        for (int i = 0; i < 3; ++i) {
            std::cout << slots[i].info_line1() << "\t" << slots[i].info_line2() << std::endl;
        }
        mtx.unlock();
    }
};

GameStatus GameSaveLoad::slots[PLAYER_SLOT_COUNT] = {GameStatus(), GameStatus(), GameStatus()};
std::mutex GameSaveLoad::mtx = std::mutex();
}