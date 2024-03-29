#include "EnemyWalk.h"
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
#include "DynArray.h"
#include "Pathfinding.h"

EnemyWalk::EnemyWalk() : Entity(EntityType::ENEMYWALK)
{
	name.Create("EnemyWalk");

	//idle
	idleAnim.LoadAnimations("idleAnimWalk");

	//dying
	dieAnim.LoadAnimations("dyingAnimWalk");

	//attacking
	attackAnim.LoadAnimations("attackAnimWalk");

}

EnemyWalk::~EnemyWalk() {

}

bool EnemyWalk::Awake() {

	position.x = parameters.attribute("x").as_int();
	position.y = parameters.attribute("y").as_int();
	texturePath = parameters.attribute("texturepath").as_string();
	speed = parameters.attribute("speed").as_float();

	return true;
}

bool EnemyWalk::Start() {

	//initilize textures
	texture = app->tex->Load(texturePath);

	killFxId = app->audio->LoadAudios("Kill");

	pbody = app->physics->CreateCircle(position.x, position.y, 8, bodyType::DYNAMIC);
	pbody->listener = this;
	pbody->ctype = ColliderType::ENEMY;
	pbody->body->SetFixedRotation(false);

	initialPos.p.x = METERS_TO_PIXELS(pbody->body->GetTransform().p.x) - 3;
	initialPos.p.y = METERS_TO_PIXELS(pbody->body->GetTransform().p.y) - 3;
	isFacingLeft = true;

	return true;
}

bool EnemyWalk::Update(float dt)
{
	currentAnim = &idleAnim;
	vel = b2Vec2(0, -GRAVITY_Y);

	if (!isDead) {
		if (isAttacking) currentAnim = &attackAnim;

		Slimefinding(dt);
	}

	position.x = METERS_TO_PIXELS(pbody->body->GetTransform().p.x) - 16;
	position.y = METERS_TO_PIXELS(pbody->body->GetTransform().p.y) - 16;

	pbody->body->SetLinearVelocity(vel);
	
	if (isDead) {
		currentAnim = &dieAnim;
		pbody->body->SetActive(false);
		if (dieAnim.HasFinished()) {
			app->entityManager->DestroyEntity(pbody->listener);
			dieAnim.Reset();
		}
	}

	currentAnim->Update();

	return true;
}

bool EnemyWalk::PostUpdate() {

	SDL_Rect rect = currentAnim->GetCurrentFrame();

	if (isFacingLeft) {
		app->render->DrawTexture(texture, position.x - 18, position.y - 23, SDL_FLIP_HORIZONTAL, &rect);
	}
	else {
		app->render->DrawTexture(texture, position.x - 18, position.y - 23, SDL_FLIP_NONE, &rect);
	}

	return true;
}
bool EnemyWalk::CleanUp()
{

	return true;
}

void EnemyWalk::OnCollision(PhysBody* physA, PhysBody* physB) {

	switch (physB->ctype)
	{
	case ColliderType::PLAYER:
		LOG("Collision PLAYER");
		break;
	case ColliderType::ATTACK:
		LOG("Collision ATTACK");
		isAttacking = false;
		isDead = true;
		app->audio->PlayFx(killFxId);
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

bool EnemyWalk::isOutOfBounds(int x, int y) {
	return true;
}

bool EnemyWalk::Slimefinding(float dt)
{
	if(app->map->pathfinding_walking->GetDistance(app->scene->GetPLayer()->position, position) <= 120){

		iPoint playerPos = app->map->WorldToMap(app->scene->GetPLayer()->position.x, app->scene->GetPLayer()->position.y);
		playerPos.x += 1;
		playerPos.y +=1;
		iPoint enemyPos = app->map->WorldToMap(position.x,position.y);
		enemyPos.y += 1;

		app->map->pathfinding_walking->CreatePath(playerPos,enemyPos);
		lastPath = *app->map->pathfinding_walking->GetLastPath();

		//Get the latest calculated path and draw
		for (uint i = 0; i < lastPath.Count(); ++i)
		{
			iPoint pos = app->map->MapToWorld(lastPath.At(i)->x, lastPath.At(i)->y);
			if (app->physics->debug == true) {
				app->render->DrawTexture(app->map->pathfinding_walking->mouseTileTex, pos.x, pos.y, SDL_FLIP_NONE);
			}
		}

		if (lastPath.Count() > 2) {
			if (lastPath.At(lastPath.Count() - 2)->x < enemyPos.x) {
				vel.x -= speed * dt;
				isFacingLeft = true;
			}

			if (lastPath.At(lastPath.Count() - 2)->x > enemyPos.x) {
				vel.x += speed * dt; 
					isFacingLeft = false;
			}

			isAttacking = false;
			attackAnim.Reset();
			pbody->body->SetLinearVelocity(vel);
		}

		if (app->map->pathfinding_walking->GetDistance(app->scene->GetPLayer()->position, position) <= 59){
			
			if (isFacingLeft) {
				vel.x -= speed * dt;
			}
			else {
				vel.x += speed * dt;

			}
			isAttacking = true;
			pbody->body->SetLinearVelocity(vel);
		}
	
	}

	else {

		isAttacking = false;

		if (!isDead) {
			currentAnim = &idleAnim;

		if (isFacingLeft) {
			if(position.x > initialPos.p.x - 40) {
				vel.x -= speed * dt;
			}
			else {
				isFacingLeft = false;
			}
		}


		else{
			if (position.x < initialPos.p.x + 40) {
				vel.x += speed * dt;
			}
			else {
				isFacingLeft = true;
			}
		}

		pbody->body->SetLinearVelocity(vel);
		}
		
	}
	return true;

}
