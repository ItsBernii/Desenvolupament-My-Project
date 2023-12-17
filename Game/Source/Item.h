#ifndef __ITEM_H__
#define __ITEM_H__

#include "Entity.h"
#include "Point.h"
#include "SDL/include/SDL.h"
#include "Animation.h"

struct SDL_Texture;

class Item : public Entity
{
public:

	Item();
	virtual ~Item();

	bool Awake();

	bool Start();

	bool Update(float dt);

	bool PostUpdate();

	bool CleanUp();

public:

	bool isPicked = false;
	Animation* currentAnim;

private:

	SDL_Texture* texture;
	const char* texturePath;
	PhysBody* pbody;

	Animation coinAnim;
};

#endif // __ITEM_H__