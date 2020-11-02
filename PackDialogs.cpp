#include "Dialogs.hpp"
#include "read_write_chunk.hpp"
#include "data_path.hpp"

#include <iostream>
#include <vector>
#include <fstream>
#include <string>

// Below code used with permission from https://github.com/lassyla/game4/blob/master/PackTextScenes.cpp with minor modifications
int main(int argc, char **argv) {
    std::cout << "packing your scene...\n"; 

    // how to open and read a text file https://www.tutorialspoint.com/read-file-line-by-line-using-cplusplus
    std::vector <Dialog> textscenes; 
    std::vector <Choice> choices; 
    std::string texts; 
    std::fstream file; 
    size_t total_choices = 0; 

    file.open(data_path("dialogs.txt"), std::ios::in);
    if(file.is_open()) {
        std::string line; 

        //get the number of scenes as the first line 
        std::getline(file, line);
        line = line.substr(0, line.find("\\")); 
        size_t num_scenes = stoi(line);    

        for(size_t i = 0; i < num_scenes; i++) {
            Dialog ts; 
            //get scene ID
            std::getline(file, line, ':');

            ts.id = stoi(line); 

            //get scene number of choices
            std::getline(file, line);
            line = line.substr(0, line.find("//")); 
            ts.num_choices = stoi(line); 
            ts.choice_start = total_choices; 
            total_choices += ts.num_choices; 

            //get scene text
            std::getline(file, line);
            line = line.substr(1, line.find("//")); 
            ts.text_start = texts.length(); 
            ts.text_length = line.length(); 
            texts.append(line); 
            
            //ts.text = line; 

            for(size_t j = 0; j < ts.num_choices; j++) {
                Choice choice; 

                //get the choice text
                std::getline(file, line);
                line = line.substr(line.find("-") + 1, line.find("//")); 
                choice.text_start = texts.length(); 
                choice.text_length = line.length(); 
                texts.append(line); 

                //get the choice ID
                std::getline(file, line);
                line = line.substr(line.find(">") + 1, line.find("//")); 
                choice.scene_id = stoi(line); 

                choices.push_back(choice); 
            }
            textscenes.push_back(ts); 

        }
        file.close(); 
    }

	//convert string to vector of chars https://stackoverflow.com/questions/8247793/converting-stdstring-to-stdvectorchar

    std::vector<char> textchars (texts.begin(), texts.end()); 
    

    std::ofstream out(data_path("dialogs.dat"), std::ios::binary);
    write_chunk("scns", textscenes, &out); 
    write_chunk("txts", textchars, &out); 
    write_chunk("chcs", choices, &out); 
    std::cout << "Packed your file. \n"; 

    return 0;
}