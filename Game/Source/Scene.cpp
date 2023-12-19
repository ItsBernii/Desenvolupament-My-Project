#include "App.h"
#include "Input.h"
#include "Textures.h"
#include "Audio.h"
#include "Render.h"
#include "Window.h"
#include "Scene.h"
#include "Map.h"
#include "Player.h"
#include "EnemyFly.h"
#include "EnemyWalk.h"

#include "Defs.h"
#include "Log.h"

Scene::Scene() : Module()
{
	name.Create("scene");
}

// Destructor
Scene::~Scene()
{}

// Called before render is available
bool Scene::Awake(pugi::xml_node& config)
{
	LOG("Loading Scene");
	bool ret = true;

	// iterate all objects in the scene
	// Check https://pugixml.org/docs/quickstart.html#access
	for (pugi::xml_node itemNode = config.child("item"); itemNode; itemNode = itemNode.next_sibling("item"))
	{
		Item* item = (Item*)app->entityManager->CreateEntity(EntityType::ITEM);
		item->parameters = itemNode;
	}

	if (config.child("player")) {
		player = (Player*)app->entityManager->CreateEntity(EntityType::PLAYER);
		player->parameters = config.child("player");
	}

	for (pugi::xml_node itemNode = config.child("enemyFly"); itemNode; itemNode = itemNode.next_sibling("enemyFly"))
	{
		EnemyFly* enemyFly = (EnemyFly*)app->entityManager->CreateEntity(EntityType::ENEMYFLY);
		enemyFly->parameters = itemNode;
	}

	for (pugi::xml_node itemNode = config.child("enemyWalk"); itemNode; itemNode = itemNode.next_sibling("enemyWalk"))
	{
		EnemyWalk* enemyWalk = (EnemyWalk*)app->entityManager->CreateEntity(EntityType::ENEMYWALK);
		enemyWalk->parameters = itemNode;
	}

	if (config.child("map")) {
		//Get the map name from the config file and assigns the value in the module
		app->map->name = config.child("map").attribute("name").as_string();
		app->map->path = config.child("map").attribute("path").as_string();
	}

	if (config.child("map").child("mouseTileTex")) {
		texturePath = config.child("map").child("mouseTileTex").attribute("texturepath").as_string();
	}

	return ret;
}
// Called before the first frame
bool Scene::Start()
{
	// NOTE: We have to avoid the use of paths in the code, we will move it later to a config file
	//background1 = app->tex->Load("Assets/Maps/background1.png");
	//Music is commented so that you can add your own music
	//app->audio->PlayMusic("Assets/Audio/Music/music_spy.ogg");

	//Get the size of the window

	//Get the size of the texture

	SString title("Map:%dx%d Tiles:%dx%d Tilesets:%d",
		app->map->mapData.width,
		app->map->mapData.height,
		app->map->mapData.tileWidth,
		app->map->mapData.tileHeight,
		app->map->mapData.tilesets.Count());

	/*app->audio->LoadMusic("backgroundMusic");*/

	return true;
}

// Called each loop iteration
bool Scene::PreUpdate()
{
	return true;
}

// Called each loop iteration
bool Scene::Update(float dt)
{
	float camSpeed = 1; 

	if(app->input->GetKey(SDL_SCANCODE_UP) == KEY_REPEAT && app->godmode)
		app->render->camera.y += (int)ceil(camSpeed * dt);

	if(app->input->GetKey(SDL_SCANCODE_DOWN) == KEY_REPEAT && app->godmode)
		app->render->camera.y -= (int)ceil(camSpeed * dt);

	if(app->input->GetKey(SDL_SCANCODE_LEFT) == KEY_REPEAT && app->godmode)
		app->render->camera.x += (int)ceil(camSpeed * dt);

	if(app->input->GetKey(SDL_SCANCODE_RIGHT) == KEY_REPEAT && app->godmode)
		app->render->camera.x -= (int)ceil(camSpeed * dt);

	// Renders the image in the center of the screen 
	//app->render->DrawTexture(img, (int)textPosX, (int)textPosY);
	//Request App to Load / Save when pressing the keys F5(save) / F6(load)
	if (app->input->GetKey(SDL_SCANCODE_F5) == KEY_DOWN) app->SaveRequest();
	if (app->input->GetKey(SDL_SCANCODE_F6) == KEY_DOWN) app->LoadRequest();

	return true;
}

// Called each loop iteration
bool Scene::PostUpdate()
{
	bool ret = true;

	if(app->input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN)
		ret = false;

	return ret;
}

// Called before quitting
bool Scene::CleanUp()
{
	LOG("Freeing scene");

	return true;
}

Player* Scene::GetPLayer()
{
	return player;
}

//Implement a method to load the state
// for now load camera's x and y
bool Scene::LoadState(pugi::xml_node node) {

	int x = node.child("player").attribute("x").as_int();
	int y = node.child("player").attribute("y").as_int();

	player->pbody->body->SetTransform(b2Vec2(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y)), 0);

	return true;
}

//Create a method to save the state of the renderer
// using append_child and append_attribute
bool Scene::SaveState(pugi::xml_node node) {

	pugi::xml_node camNode = node.append_child("player");
	camNode.append_attribute("x").set_value(player->position.x);
	camNode.append_attribute("y").set_value(player->position.y);

	return true;
}