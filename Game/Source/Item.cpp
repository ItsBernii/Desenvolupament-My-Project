#include "Item.h"
#include "App.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Point.h"
#include "Physics.h"
#include "Animation.h"

Item::Item() : Entity(EntityType::ITEM)
{
	name.Create("item");

	coinAnim.LoadAnimations("coinAnim");
}

Item::~Item() {}

bool Item::Awake() {

	position.x = parameters.attribute("x").as_int();
	position.y = parameters.attribute("y").as_int();
	texturePath = parameters.attribute("texturepath").as_string();

	return true;
}

bool Item::Start() {

	//initilize textures
	texture = app->tex->Load(texturePath);
	pbody = app->physics->CreateCircle(position.x + 8, position.y + 8, 4, bodyType::KINEMATIC);
	pbody->ctype = ColliderType::ITEM;
	pbody->listener = this;
	

	return true;
}

bool Item::Update(float dt)
{
	// L07 DONE 4: Add a physics to an item - update the position of the object from the physics. 
	currentAnim = &coinAnim;
	coinAnim.Update();
	return true;
}

bool Item::PostUpdate()
{
	SDL_Rect rect = currentAnim->GetCurrentFrame();
	app->render->DrawTexture(texture, position.x, position.y, SDL_FLIP_NONE, &rect);

	return true;
}

bool Item::CleanUp()
{
	return true;
}


