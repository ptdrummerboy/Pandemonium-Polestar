#include "ZBRA.h"

#include <iostream>

void breakHere();

void ZBRA::printDimsRecursive(){
	for (vector<ZBRA*>::iterator test = subArea.begin(); test != subArea.end(); test++){
		(*test)->printDimsRecursive();
	}
	for (vector<ZBRA*>::iterator test = adjacent.begin(); test != adjacent.end(); test++){
		(*test)->printDimsRecursive();
	}
	cout << dims.x << ',' << dims.y << ':' << dims.x + dims.width << ',' << dims.y + dims.height << endl; 
}

ZBRA::ZBRA()
{
	dims.x = INT_MAX;
	dims.width = 0;
	dims.y = INT_MAX;
	dims.width = 0;
	dims.size = 0;
	flags = 0;
	effDims.x = INT_MAX;
	effDims.y = INT_MAX;
	effDims.width = 0;
	effDims.height = 0;
}

ZBRA::~ZBRA()
{
	delete map;
}

int ZBRA::getIdealSize(){
	int size = 0;
	for (vector<ZBRA*>::iterator i = subArea.begin(); i != subArea.end(); i++){
		size += (*i)->idealSize;
		for (vector<ZBRA*>::iterator j = (*i)->adjacent.begin(); j != (*i)->adjacent.end(); j++)
			size += (*j)->idealSize;
	}
	return size;
}

int ZBRA::getMinSize()
{
	int size = 0;
	for (vector<ZBRA*>::iterator i = subArea.begin(); i != subArea.end(); i++)
		size += (*i)->minSize;
	return size;
}

void ZBRA::solveRecursive(){
	//cout << seed << '\n';
	for (vector<ZBRA*>::iterator test = subArea.begin(); test != subArea.end(); test++){
		(*test)->solveRecursive();
	}
	DimSolv lambda(this);
}

void ZBRA::buildMap(Tile* superParent){
	map = new Map(dims.x,dims.y,dims.width,dims.height,floor,wall);
	map->populate();

	//Add the fops to the tiles after the map has been populated
	placeFops();

	//if (subArea.size()) {
	for (vector<ZBRA*>::iterator child = subArea.begin(); child < subArea.end(); child++){
		(*child)->buildMap(map->ref);//readies children maps
		Tile* pPoint;	//parent reference tile for stiching
		Tile* cPoint;	//child reference tile for stiching
		
		pPoint = map->ref;
		cPoint = (*child)->map->ref;
		
		Map::stitch(pPoint,cPoint,(*child)->wall,(*child)->map);
		map->checkConsistency((*child)->map->ref);
	}
	
	for (vector<ZBRA*>::iterator adj = adjacent.begin(); adj < adjacent.end(); adj++){
		(*adj)->buildMap(map->ref);//readies children maps
		Tile* pPoint;	//parent reference tile for stiching
		Tile* cPoint;	//child reference tile for stiching
		
		pPoint = superParent;
		cPoint = (*adj)->map->ref;
		
		Map::stitch(pPoint,cPoint,(*adj)->wall,(*adj)->map);
		map->checkConsistency((*adj)->map->ref);
	}
}

void ZBRA::findWalls(Tile* ref, set<Tile*> pWalls){
	if (floor == Tile::UDEF){
		for (set<Tile*>::iterator it = pWalls.begin(); it != pWalls.end(); it++){
			walls.insert(*it);
		}
	}
	else{
		while(ref->x < dims.x){
			if (ref->east != NULL){
				ref = ref->east;
			}
			else{
				cerr << "Bump x\n";
				return;
			}
		}
		while(ref->x > dims.x){
			if (ref->west != NULL){
				ref = ref->west;
			}
			else{
				cerr << "Bump x\n";
				return;
			}
		}
		while(ref->y < dims.y){
			if (ref->south != NULL){
				ref = ref->south;
			}
			else{
				cerr << "Bump y\n";
				return;
			}
		}
		while(ref->y > dims.y){
			if (ref->north != NULL){
				ref = ref->north;
			}
			else{
				cerr << "Bump y\n";
				return;
			}
		}
		
		//make sure we're where we're supposed to be.
		bool east = false;
		while (ref->type != floor){
			if (east){
				if (ref->south && ref->south->type == floor){
					ref = ref->south;
				}
				else if (ref->east){
					ref = ref->east;
				}
				else if (ref->south){
					ref = ref->south;
				}
				else{//Should NOT ever happen.
					ref->type = floor;
				}
			}
			else{
				if (ref->east && ref->east->type == floor){
					ref = ref->east;
				}
				else if (ref->south){
					ref = ref->south;
				}
				else if (ref->east){
					ref = ref->east;
				}
				else{//Should NOT ever happen.
					ref->type = floor;
				}
			}
			east = !east; 
		}
		//we need the walls surrounding the area. (read, not floor)
		set<Tile*> toCheck;
		toCheck.insert(ref);
		//ref->type = Tile::ROAD;
		//create a list of things that have already been checked for walls
		set<Tile*> isChecked;
		while(!toCheck.empty()){
			set<Tile*>::iterator it = toCheck.begin(); 
			Tile* curr = *it;
			if (!curr) return;
			if (curr->north && !isChecked.count(curr->north)){
				if (curr->north->type == floor)
					toCheck.insert(curr->north);
				else{
					walls.insert(curr->north);
				}
			}
			if (curr->west && !isChecked.count(curr->west)){
				if (curr->west->type == floor)
					toCheck.insert(curr->west);
				else{
					walls.insert(curr->west);
				}
			}
			if (curr->south && !isChecked.count(curr->south)){
				if (curr->south->type == floor)
					toCheck.insert(curr->south);
				else{
					walls.insert(curr->south);
				}
			}
			if (curr->east && !isChecked.count(curr->east)){
				if (curr->east->type == floor)
					toCheck.insert(curr->east);
				else{
					walls.insert(curr->east);
				}
			}
			isChecked.insert(curr);
			toCheck.erase(it);
		}
	}
	
	for (vector<ZBRA*>::iterator child = subArea.begin(); child < subArea.end(); child++){
		(*child)->findWalls(ref,walls);//readies children maps
	}
	
	for (vector<ZBRA*>::iterator adj = adjacent.begin(); adj < adjacent.end(); adj++){
		(*adj)->findWalls(ref,walls);//readies adjacent maps
	}
}

void ZBRA::addDoors(){
	for (vector<ZBRA*>::iterator child = subArea.begin(); child < subArea.end(); child++){
		vector<Tile*> intersect;
		if ((*child)->floor != Tile::UDEF){
			set<Tile*>::iterator pSet, cSet;
			cerr << walls.size() << ',' << (*child)->walls.size() << endl;
			for (pSet = walls.begin(); pSet != walls.end(); pSet++){
				for (cSet = (*child)->walls.begin(); cSet != (*child)->walls.end(); cSet++){
					//cerr<< "Here?\n";
					if((*pSet)->x == (*cSet)->x && (*pSet)->y == (*cSet)->y){
						intersect.push_back(*pSet);
					}
					else{
						//(*cSet)->type = Tile::ROAD;
						//(*pSet)->type = Tile::UDEF;
					}
				}
			}
		}
		
		if (intersect.size() > 0)
			intersect[random() / (RAND_MAX / intersect.size())]->type = Tile::DOOR;
		//else
			//cerr << "Non-intersecting wall chunks\n";
		(*child)->addDoors();//readies children maps
	}
	
	
	//NEIGHBORS
	for (vector<ZBRA*>::iterator adj = adjacent.begin(); adj < adjacent.end(); adj++){
		vector<Tile*> intersect;
		if ((*adj)->floor != Tile::UDEF){
			set<Tile*>::iterator pSet, cSet;
			//cerr << walls.size() << ',' << (*adj)->walls.size() << endl;
			for (pSet = walls.begin(); pSet != walls.end(); pSet++){
				for (cSet = (*adj)->walls.begin(); cSet != (*adj)->walls.end(); cSet++){
					//cerr<< "Here?\n";
					if((*pSet)->x == (*cSet)->x && (*pSet)->y == (*cSet)->y){
						intersect.push_back(*pSet);
					}
					else{
						//(*cSet)->type = Tile::ROAD;
						//(*pSet)->type = Tile::UDEF;
					}
				}
			}
		}
		
		if (intersect.size() > 0)
			intersect[random() / (RAND_MAX / intersect.size())]->type = Tile::DOOR;
		(*adj)->addDoors();//readies adjacent maps
	}
	
}

void ZBRA::recursiveWiggle(int x, int y){
	dims.x += x;
	dims.y += y;
	for (vector<ZBRA*>::iterator i = adjacent.begin(); i != adjacent.end();i++){
		(*i)->recursiveWiggle(x,y);
	}
	for (vector<ZBRA*>::iterator i = subArea.begin(); i != subArea.end();i++){
		(*i)->recursiveWiggle(x,y);
	}
}

void ZBRA::AddFop(FOP* fop, Tile* tile)
{
	tile->fops.push_back(fop);
}

void ZBRA::AddFop(FOP* fop)
{
	fopslist.push_back(fop);
}

/********************************************
 *placeFops()
 *
 *This function will iterate through all of
 *the ZBRAs and place the FOPs of them into
 *each of their tiles.
 ********************************************/
void ZBRA::placeFops()
{
	vector<FOP*>::iterator fops;
	for (fops = fopslist.begin(); fops < fopslist.end(); fops++)
	{
		int x,y;
		do{
			x = random() / (RAND_MAX / dims.width);
			y = random() / (RAND_MAX / dims.height);
			cerr << x << ',' << y << "of" << dims.width << ',' << dims.height << endl;
		}while(map->area[x][y].type != floor);
		map->area[x][y].fops.push_back(*fops);
		(*fops)->x = map->area[x][y].x * Tile::SIZE + Tile::SIZE / 2;
		(*fops)->y = map->area[x][y].y * Tile::SIZE + Tile::SIZE / 2;
	}
}

ZBRA* ZBRA::City()
{
	seed = time(NULL);
	srandom(seed);
	floor = Tile::GRASS;
	wall = Tile::UDEF;
	subArea.push_back((new ZBRA)->House(random())); //only one test house for now
	minSize = 25 + getMinSize();
	idealSize = 900 + getIdealSize();
	solveRecursive();
	printDimsRecursive();
	//add a few grass to the city
	for(int i=0; i<20; i++)
		//AddFop((new FOP)->Grass());
	buildMap(NULL);
	findWalls(map->ref,walls);
	addDoors();
	AddFop((new FOP)->Character(320, 240, map->ref), map->ref);
	return this;
}
		
ZBRA* ZBRA::House(int randSeed)
{
	seed = randSeed;
	flags = ZBRA::SOFT;
	floor = Tile::UDEF;
	wall = Tile::UDEF;
	//for now every house will contain 1 bathroom kitchen and living room and 2 bedrooms
	subArea.push_back((new ZBRA)->LivingRoom(random()));
	//for now every house will contain 1 cat
	AddFop((new FOP)->Cat());
	AddFop((new FOP)->Lamp());
	minSize = 20 + getMinSize();
	idealSize = 20 + getIdealSize();
	return this;
}

ZBRA* ZBRA::BathRoom(int randSeed){
	seed = randSeed;
	flags = 0;
	minSize = 20;
	idealSize = 30;
	floor = Tile::BATHROOM;
	wall = Tile::WALL;
	//every bathroom for now will contain only 1 toilet
	AddFop((new FOP)->Toilet());
	return this;
}

ZBRA* ZBRA::Kitchen(int randSeed){
	seed = randSeed;
	flags = 0;
	floor = Tile::KITCHEN;
	wall = Tile::WALL;
	//every kitchen for now will contain only 1 fridge
	AddFop((new FOP)->Refrigerator());
	minSize = 25;
	idealSize = 40;
	return this;
}

ZBRA* ZBRA::LivingRoom(int randSeed){
	seed = randSeed;
	flags = 0;
	floor = Tile::FLOOR;
	wall = Tile::WALL;
	adjacent.push_back((new ZBRA)->Kitchen(random()));
	//adjacent.push_back((new ZBRA)->HouseHallway(random()));	
	adjacent.push_back((new ZBRA)->BathRoom(random()));
	adjacent.push_back((new ZBRA)->BedRoom(random()));
	adjacent.push_back((new ZBRA)->BedRoom(random()));
	//every living room for now will contain only 1 sofa and now a tv
	AddFop((new FOP)->Sofa());
	AddFop((new FOP)->Tv());
	AddFop((new FOP)->Lamp());
	minSize = 30;
	idealSize = 45;
	return this;
}

ZBRA* ZBRA::HouseHallway(int randSeed){
	seed = randSeed;
	flags = ZBRA::LONG;
	floor = Tile::ROAD;
	wall = Tile::WALL;

	minSize = 30;
	idealSize = 45;
	return this;
}

ZBRA* ZBRA::BedRoom(int randSeed){ 
	seed = randSeed;
	floor = Tile::BEDROOM;
	wall = Tile::WALL;
	//every bedroom for now will contain only 1 bed
	AddFop((new FOP)->Bed());
	AddFop((new FOP)->Lamp());
	minSize = 20;
	idealSize = 35;
	return this;
}
