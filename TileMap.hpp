#include <vector>
#include <map>
#include <string>

struct Level
{
	std::vector< uint32_t > data;
};

struct TileType
{
	static const uint32_t Platform = 1;
	static const uint32_t Collectible = 2;
	static const uint32_t End = 3;
	static const uint32_t Object = 4;
};

struct Tile
{
	std::string img_name;
	uint32_t tile_type;
};


struct TileMap
{
	size_t map_width;
	size_t map_height;

	size_t tile_width;
	size_t tile_height;

	size_t num_levels;
	size_t num_tilesets;

	std::map < uint32_t, Tile > tiles;
	std::vector< Level > levels;
};

