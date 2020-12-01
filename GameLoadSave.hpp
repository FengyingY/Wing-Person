#pragma once
#pragma warning(disable : 4996)

#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <chrono>
#include <ctime>
#include <mutex>

#include "data_path.hpp"

struct GameStatus {
    std::string story_name;
    std::string save_time;

    std::string info() {
        return story_name + "\n" + save_time;
    }
};
namespace {
struct GameSaveLoad {

    static GameStatus slots[3];
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
            if (count == 3) break;
            size_t pos = line.find(' ');
            slots[count].story_name = line.substr(0, pos);
            slots[count].save_time = line.substr(pos+1, line.length() - pos - 1);
            count++;
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
        std::ofstream ofs(data_path("saved_games"), std::ofstream::trunc);
        for (GameStatus s : slots) {
            ofs << s.story_name << " " << s.save_time << "\n";
        }
        ofs.close();
        mtx.unlock();

    }

    static void save(std::string name, size_t slot_idx) {
        // Ref: https://stackoverflow.com/questions/997946/how-to-get-current-time-and-date-in-c
        if (slot_idx < 3) {
            mtx.lock();
            slots[slot_idx].story_name = name;
            auto time = std::chrono::system_clock::now();
            std::time_t save_timestamp = std::chrono::system_clock::to_time_t(time);
            slots[slot_idx].save_time = std::ctime(&save_timestamp);
            slots[slot_idx].save_time = slots[slot_idx].save_time.substr(0, slots[slot_idx].save_time.length()-1);
            mtx.unlock();
        }
    }

    static void print_status() {
        mtx.lock();
        for (int i = 0; i < 3; ++i) {
            std::cout << slots[i].story_name << " " << slots[i].save_time << std::endl;
        }
        mtx.unlock();
    }
};

GameStatus GameSaveLoad::slots[3] = {GameStatus(), GameStatus(), GameStatus()};
std::mutex GameSaveLoad::mtx = std::mutex();
}