#include "read_write_chunk.hpp"
#include "data_path.hpp"

#include <iostream>
#include <vector>
#include <fstream>
#include <string>

#include "PackLevelData.hpp"
#include "json.hpp"
using JSON = nlohmann::json;

// uses JSON tile map data exported from Tiled Editor - https://www.mapeditor.org/
int main(int argc, char **argv)
{
	std::cout << "Packing level data..." << std::endl;

	JSON jsonData;
	std::string packed_stream;

	try
	{
		std::ifstream dataFile("level-data/Sample.json");
		dataFile >> jsonData;
	}
	catch(const std::exception& e)
	{
		std::cerr << "JSON read failed. " << e.what() << '\n';
	}

	// Done parsing JSON. Now start reading the data
	LevelData level_data;
	(void) level_data;

	// Read width (in num of tiles), height (in num of tiles), tile width and tile height (in px)
	level_data.map_width = jsonData["width"];
	level_data.map_height = jsonData["height"];

	level_data.tile_width = jsonData["tilewidth"];
	level_data.tile_height = jsonData["tileheight"];

	std::cout << "tilesets: " << "\n";
	size_t num_tilesets = 0;

	// for each tileset
	JSON tilesets = jsonData["tilesets"];
	for (JSON::iterator it = tilesets.begin(); it != tilesets.end(); ++it)
	{
		std::cout << it.value()["firstgid"] << " | " << it.value()["source"] << "\n";

		// Read tileset info: source, first-gid
		num_tilesets++;
	}
	
	std::ofstream datFile("dist/level-data.dat", std::ios::binary);
	std::vector< uint32_t > data;

	// for each layer
	size_t num_levels = 0;
	JSON layers = jsonData["layers"];
	try
	{
		for (JSON::iterator it = layers.begin(); it != layers.end(); ++it)
		{
			// store as a 1D array
			for (auto &tile : it.value()["data"])
			{
				data.emplace_back(tile);
			}
			num_levels++;
		}
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	
	// #TODO: Currently only assuming 1 level. Not packing considering multiple levels.
	// 		  Will get back to this after successfully rendering a level in game

	write_chunk("lvdt", data, &datFile);
	datFile.close();

	std::cout << "Packed " << num_levels << " levels\n";
	
	// TESTING
	std::cout << "TESTING DATA:\n";
	data.clear();
	
	try
	{
		std::ifstream test("dist/level-data.dat", std::ios::binary);
		read_chunk(test, "lvdt", &data);
		
		std::cout << "level data:\n";
		for (auto &&tile : data)
		{
			std::cout << tile << " ";
		}
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}

	return 0;
}