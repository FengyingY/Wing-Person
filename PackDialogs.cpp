#include "read_write_chunk.hpp"
#include "data_path.hpp"

#include <iostream>
#include <vector>
#include <fstream>
#include <string>

#include "PackDialogs.hpp"
#include "json.hpp"
using JSON = nlohmann::json;

// Below code used with permission from https://github.com/lassyla/game4/blob/master/PackTextScenes.cpp with minor modifications
int main(int argc, char **argv) {
    std::cout << "packing your dialogs...\n"; 

    // how to open and read a text file https://www.tutorialspoint.com/read-file-line-by-line-using-cplusplus
    std::vector <DialogChunk> dialogs; 
    
    std::string texts; 
    JSON file;

	// loading in from a json file
	try {
		std::ifstream in("dist/dialog.json");
		in >> file;
	}
	catch (int e) {
		std::cout << "A json exception occurred. Your json file is most likely formatted incorrectly. Exception #: " << e << std::endl;
	}

    try {
        file["Opening"];
    } catch (int e) {
        std::cout << "Fail to read Opening" << std::endl;
    }
    
    std::string tmp = "";
    for (JSON::iterator it = file.begin(); it != file.end(); ++it) {
        DialogChunk new_dialog;
        new_dialog.dialog_start = texts.length();

        std::string dialog_name = it.key();
        texts.append(dialog_name);
        new_dialog.name_length = dialog_name.length();

        tmp = "";
        std::vector<std::string> text = file[dialog_name]["text"];
        for (std::string t : text) {
            tmp.append(t);
            tmp.append("\n");
        }
        texts.append(tmp);
        new_dialog.text_length = tmp.length();
        
        tmp = "";
        std::vector<std::string> choices = file[dialog_name]["choices"];
        for (std::string t : choices) {
            tmp.append(t);
            tmp.append("\n");
        }
        texts.append(tmp);
        new_dialog.choice_length = tmp.length();

        tmp = "";
        tmp = file[dialog_name]["character"];
        texts.append(tmp);
        new_dialog.character_length = tmp.length();

        tmp = "";
        std::vector<std::string> sprites = file[dialog_name]["sprite"];
        for (std::string t : sprites) {
            tmp.append(t);
            tmp.append("\n");
        }
        texts.append(tmp);
        new_dialog.sprite_length = tmp.length();

        tmp = "";
        tmp = file[dialog_name]["background"];
        texts.append(tmp);
        new_dialog.background_length = tmp.length();
        
        dialogs.push_back(new_dialog);
    }

	//convert string to vector of chars https://stackoverflow.com/questions/8247793/converting-stdstring-to-stdvectorchar
    std::cout << texts << std::endl << std::endl;
    std::vector<char> textchars (texts.begin(), texts.end()); 
    
    std::ofstream out("dist/dialogs.dat", std::ios::binary);
    write_chunk("txts", textchars, &out); 
    write_chunk("dlgs", dialogs, &out); 
    std::cout << "Packed your file. \n"; 

    // testing
    /*
    std::ifstream in("dialogs.dat", std::ios::binary);
    std::vector<char> read_text;
    std::vector<DialogChunk> read_dialog;
    read_chunk(in, "txts", &read_text);
    read_chunk(in, "dlgs", &read_dialog);

    std::string t(read_text.begin(), read_text.end());
    std::cout << t << std::endl;
    for (DialogChunk d : read_dialog) {
        std::cout << d.dialog_start << " " << d.name_length << " " << d.text_length << " " << 
        d.choice_length << " " << d.character_length << " " << std::endl;
        std::cout << "name: " << t.substr(d.dialog_start, d.name_length) << std::endl;
        std::cout << "text: " << t.substr(d.dialog_start+d.name_length, d.text_length) << std::endl;
        std::cout << "choice: " << t.substr(d.dialog_start+d.name_length+d.text_length, d.choice_length) << std::endl;
        std::cout << "character: " << t.substr(d.dialog_start+d.name_length+d.text_length+d.choice_length, d.character_length) << std::endl;
    }*/
    
    return 0;
}