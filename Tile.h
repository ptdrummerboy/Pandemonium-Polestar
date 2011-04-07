#ifndef _TILE_
#define _TILE_

#include <SDL/SDL.h>

class Tile
{
	public:
		static const int SIZE = 5;
		static const short int UDEF = 0, WALL = 1, FLOOR = 2, ROAD = 3, GRASS = 4;
	private:
		int x;
		int y;
		Uint8 sprite;
		bool solid;
		Tile* north;
		Tile* south;
		Tile* east;
		Tile* west;

	public: 
		Tile(int x, int y, Uint8 sprite, bool solid = false);
		Tile();
		~Tile();
};


#endif