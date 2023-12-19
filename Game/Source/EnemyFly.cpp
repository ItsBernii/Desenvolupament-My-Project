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
#include "Map.h"

EnemyFly::EnemyFly() : Entity(EntityType::ENEMYFLY)
{
	name.Create("EnemyFly");

	//idle
	idleAnim.LoadAnimations("idleAnimFly");

	//dying
	dieAnim.LoadAnimations("dyingAnimBat");

	//dead
	deadAnim.LoadAnimations("deadAnimBat");

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

	pbody = app->physics->CreateCircle(position.x, position.y, 6, bodyType::DYNAMIC);
	pbody->listener = this;
	pbody->ctype = ColliderType::ENEMY;
	pbody->body->SetFixedRotation(false);
	pbody->body->SetGravityScale(0);

	initialPos = pbody->body->GetTransform();

	isFacingLeft = true;

	return true;
}

bool EnemyFly::Update(float dt)
{
	currentAnim = &idleAnim;

	b2Vec2 vel = b2Vec2(0, -GRAVITY_Y);
	vel.y = pbody->body->GetLinearVelocity().y;

	if (app->input->GetKey(SDL_SCANCODE_K) == KEY_REPEAT) {
		vel = b2Vec2(speed * dt, 0);
	}

	if (isDead) {
		currentAnim = &deadAnim;
		pbody->body->SetActive(false);
		if (deadAnim.HasFinished() == true) {
			app->entityManager->DestroyEntity(pbody->listener);
		}
	}

	if (app->input->GetKey(SDL_SCANCODE_I) == KEY_DOWN) {
		pbody->body->SetGravityScale(1);
		pbody->body->SetLinearVelocity(b2Vec2(0, -GRAVITY_Y));
		currentAnim = &dieAnim;
	}

	position.x = METERS_TO_PIXELS(pbody->body->GetTransform().p.x) - 16;
	position.y = METERS_TO_PIXELS(pbody->body->GetTransform().p.y) - 16;

	pbody->body->SetLinearVelocity(vel);

	currentAnim->Update();

	return true;
}

bool EnemyFly::PostUpdate() {

	SDL_Rect rect = currentAnim->GetCurrentFrame();

	if (isFacingLeft) {
		app->render->DrawTexture(texture, position.x - 4, position.y + 7, SDL_FLIP_HORIZONTAL, &rect);
	}
	else {
		app->render->DrawTexture(texture, position.x + 3, position.y + 2, SDL_FLIP_NONE, &rect);
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
	case ColliderType::PLAYER:
		LOG("Collision PLAYER");
		break;

	case ColliderType::ATTACK:
		LOG("Collision ATTACK");
		pbody->body->SetGravityScale(1);
		pbody->body->SetLinearVelocity(b2Vec2(0, -GRAVITY_Y));
		currentAnim = &dieAnim;
		break;

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
		isDead = true;
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

bool EnemyFly::Flyfinding()
{
	if (app->map->pathfinding->GetDistance(app->scene->GetPLayer()->position, position) <= 200) {

		app->map->pathfinding->CreatePath(position, app->scene->GetPLayer()->position);
		lastPath = *app->map->pathfinding->GetLastPath();
		// L13: Get the latest calculated path and draw
		for (uint i = 0; i < lastPath.Count(); ++i)
		{
			iPoint pos = app->map->MapToWorld(lastPath.At(i)->x, lastPath.At(i)->y);
			app->render->DrawTexture(app->map->pathfinding->mouseTileTex, pos.x, pos.y, SDL_FLIP_NONE);
		}

	}
	return true;
}