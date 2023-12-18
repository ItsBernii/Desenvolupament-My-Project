#include "EnemyFly.h"
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
#include "Timer.h"
#include "PerfTimer.h"
#include "Window.h"

EnemyFly::EnemyFly() : Entity(EntityType::ENEMYFLY)
{
	name.Create("EnemyFly");

	//idle
	idleAnim.LoadAnimations("idleAnimFly");

}

EnemyFly::~EnemyFly() {

}

bool EnemyFly::Awake() {

	position.x = parameters.attribute("x").as_int();
	position.y = parameters.attribute("y").as_int();
	texturePath = parameters.attribute("texturepath").as_string();
	speed = parameters.attribute("speed").as_float();

	return true;
}

bool EnemyFly::Start() {

	//initilize textures
	texture = app->tex->Load(texturePath);

	pbody = app->physics->CreateCircle(position.x, position.y + 12, 14, bodyType::DYNAMIC);
	pbody->listener = this;
	pbody->ctype = ColliderType::ENEMY;
	pbody->body->SetFixedRotation(false);

	initialPos = pbody->body->GetTransform();


	return true;
}

bool EnemyFly::Update(float dt)
{
	currentAnim = &idleAnim;

	return true;
}

bool EnemyFly::PostUpdate() {

	SDL_Rect rect = currentAnim->GetCurrentFrame();

	if (isFacingLeft) {
		app->render->DrawTexture(texture, position.x, position.y, SDL_FLIP_HORIZONTAL, &rect);
	}
	else {
		app->render->DrawTexture(texture, position.x, position.y, SDL_FLIP_NONE, &rect);
	}

	return true;
}
bool EnemyFly::CleanUp()
{

	return true;
}

void EnemyFly::OnCollision(PhysBody* physA, PhysBody* physB) {

	switch (physB->ctype)
	{
	case ColliderType::ENEMY:
		LOG("Collision ENEMY");
		break;
	case ColliderType::ITEM:
		LOG("Collision ITEM");
		break;

	case ColliderType::WALL:
		LOG("Collision WALL");
		break;

	case ColliderType::PLATFORM:
		LOG("Collision PLATFORM");
		break;

	case ColliderType::SPIKES:

		LOG("Collision SPIKES");
		break;

	case ColliderType::UNKNOWN:
		LOG("Collision UNKNOWN");
		break;
	}

}

bool EnemyFly::isOutOfBounds(int x, int y) {
	return true;
}