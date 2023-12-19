#ifndef __SCENE_H__
#define __SCENE_H__

#include "Module.h"
#include "Player.h"
#include "EnemyFly.h"
#include "EnemyWalk.h"
#include "Item.h"

struct SDL_Texture;

class Scene : public Module
{
public:

	Scene();

	// Destructor
	virtual ~Scene();

	// Called before render is available
	bool Awake(pugi::xml_node& conf);

	// Called before the first frame
	bool Start();

	// Called before all Updates
	bool PreUpdate();

	// Called each loop iteration
	bool Update(float dt);

	// Called before all Updates
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();

	
	bool LoadState(pugi::xml_node node);

	
	bool SaveState(pugi::xml_node node);

	Player* GetPLayer();

	SString texturePath;

private:
	float textPosX, textPosY = 0;
	uint texW, texH;
	uint windowW, windowH;
	Player* player;
	EnemyFly* enemyFly;
	EnemyWalk* enemyWalk;

};

#endif // __SCENE_H__