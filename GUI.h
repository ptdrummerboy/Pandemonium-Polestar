#ifndef _GUI_
#define _GUI_

#include <SDL/SDL.h>
#include <iostream>

#include "ZBRA.h"
#include "Tile.h"
#include "SurfaceLoader.h"
#include "Events.h"
#include "FOP.h"

class GUI : public Events
{

	private:
	bool running;
	SDL_Surface* screen;
	SDL_Surface *icon;
	SDL_Surface* TILES;
	SDL_Surface* CHARACTER;

	public:
	SDL_Event Event;
	ZBRA city;	//declaration of the highest ZBRA (city)
	FOP* character; //declaration of the character pointing to any FOP being controlled by the player
	int charX;	//X position of the player
	int charY;	//Y position of the player
	int charXvel;	//X velocity of the player
	int charYvel;	//Y velocity of the player

	public:
	int onExecute();
	bool Initialize();
	void EventHandler(SDL_Event* Event);
	void Logic();
	void Render(Tile* ref);
	void onExit();
	void Cleanup();
	void RenderMap(Tile* refTile);

	void OnKeyDown(SDLKey sym, SDLMod mod, Uint16 unicode);
	void OnKeyUp(SDLKey sym, SDLMod mod, Uint16 unicode);
	bool checkCollision();
};

#endif
