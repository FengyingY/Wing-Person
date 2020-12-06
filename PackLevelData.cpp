#include "read_write_chunk.hpp"
#include "data_path.hpp"

#include <iostream>
#include <vector>
#include <fstream>
#include <string>

#include "PackLevelData.hpp"
#include "TileMap.hpp"
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
	TileMap level_data;

	// output dat file
	std::ofstream datFile("dist/level-data.dat", std::ios::binary);

	// Read width (in num of tiles), height (in num of tiles), tile width and tile height (in px)
	level_data.map_width = jsonData["width"];
	level_data.map_height = jsonData["height"];

	level_data.tile_width = jsonData["tilewidth"];
	level_data.tile_height = jsonData["tileheight"];

	std::cout << "Tiles: " << "\n";
	level_data.num_tilesets = 0;

	std::vector< TileChunk > tile_chunks;
	std::string tile_paths;
	// for each tileset
	JSON tilesets = jsonData["tilesets"];
	try
	{
		for (JSON::iterator it = tilesets.begin(); it != tilesets.end(); ++it)
		{
			JSON tiles = it.value()["tiles"];
			
			for (JSON::iterator it_tile = tiles.begin(); it_tile != tiles.end(); ++it_tile)
			{
				std::string img = it_tile.value()["image"];
				img = img.substr(img.find_last_of('/')+1);
				
				TileChunk tile_chunk;
				tile_chunk.tile_id = it_tile.value()["id"];
				tile_chunk.path_length = img.length();

				JSON properties = it_tile.value()["properties"];
				for (JSON::iterator it_prop = properties.begin(); it_prop != properties.end(); ++it_prop)
				{
					if (it_prop.value()["name"] == "tile_type")
					{
						tile_chunk.tile_type = it_prop.value()["value"];
						break;
					}
				}

				std::cout << "ID : " << tile_chunk.tile_id << " | Name : " << img << "\n";

				tile_chunks.emplace_back(tile_chunk);
				tile_paths.append(img);
			}
			
			level_data.num_tilesets++;
		}
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	

	std::cout << "Read " << level_data.num_tilesets << " tilesets" << "\n";

	// write chunk with tileset data
	// tile: id, image => 32 bytes + (img_path.length) bytes

	//convert string to vector of chars https://stackoverflow.com/questions/8247793/converting-stdstring-to-stdvectorchar
    std::cout << tile_paths << std::endl << std::endl;
    std::vector<char> tile_paths_chars (tile_paths.begin(), tile_paths.end()); 

	write_chunk("tsid", tile_chunks, &datFile);
	write_chunk("tsip", tile_paths_chars, &datFile);
	
	// for each layer. Every layer is considered to be a level
	uint32_t num_levels = 0;
	JSON layers = jsonData["layers"];
	std::vector< uint32_t > levels;
	try
	{
		uint32_t prev_level_width, prev_level_height;
		if (layers.begin() != layers.end())
		{
			// initializing size to match that of the first level
			prev_level_width = layers.begin().value()["width"];
			prev_level_height = layers.begin().value()["height"];
		}

		for (JSON::iterator it = layers.begin(); it != layers.end(); ++it)
		{
			std::cout << "Packing level : " << num_levels+1 << "\n";

			assert(it.value()["width"] == prev_level_width && "Level sizes inconsistent");
			assert(it.value()["height"] == prev_level_height && "Level sizes inconsistent");

			// store data as a 1D array
			for (auto &tile : it.value()["data"])
			{
				levels.emplace_back(tile);
			}
			num_levels++;
		}

		levels.emplace(levels.begin(), num_levels);	// the first value in the vector will be the number of levels stored in this 1D vector
	}
	catch(const std::exception& e)
	{
		std::cerr << "Error! " << e.what() << '\n';
	}

	write_chunk("lvdt", levels, &datFile);	// level data stored in a continuous array assuming constant size for all levels
	
	datFile.close();

	std::cout << "Packed " << num_levels << " levels\n";
	
	// TESTING
	std::cout << "TESTING DATA:\n";

	std::vector< TileChunk > ids;
	std::vector< char > paths;
	std::vector< uint32_t > data;
	
	try
	{
		std::ifstream test("dist/level-data.dat", std::ios::binary);
		
		read_chunk(test, "tsid", &ids);
		read_chunk(test, "tsip", &paths);
		read_chunk(test, "lvdt", &data);

		std::string imgstr(paths.begin(), paths.end());

		std::cout << "\nTileset IDS:\n";
		size_t cursor_pos = 0;
		for (auto &&id : ids)
		{
			std::string img_name = imgstr.substr(cursor_pos, id.path_length);
			std::cout << id.tile_id << ", " << id.tile_type << ", " << id.path_length << " | " << img_name << "\n";
			cursor_pos += id.path_length;
		}

		uint32_t num_levels = data[0];
		std::cout << "\n Levels : " << num_levels << "\n---------------";
		
		assert(num_levels > 0 && "No levels packed!");
		
		size_t tile_count = 0, level_count = 0;
		std::cout << "\nLevel 1\n" ;
		for (std::vector<uint32_t>::iterator it = ++data.begin(); it != data.end(); ++it)
		{
			std::cout << *it << " ";
			if (tile_count == 18 * 25)
			{
				tile_count = 0;
				std::cout << "\nLevel " << ++level_count + 1 << "\n";
			}
			tile_count++;
		}
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}

	return 0;
}