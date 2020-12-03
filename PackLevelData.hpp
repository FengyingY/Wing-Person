struct LevelDataChunk {
	size_t num_levels;
	size_t num_tilesets;
};

struct TileSetChunk {
	size_t num_tiles;
};

struct TileChunk {
	uint32_t tile_id;
	uint32_t tile_type;
	size_t path_length;
};