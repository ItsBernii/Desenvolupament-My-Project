#include "Player.h"
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

Player::Player() : Entity(EntityType::PLAYER)
{
	name.Create("Player");

	//idle
	idleAnim.LoadAnimations("idleAnim");


	//run
	runAnim.LoadAnimations("runAnim");



	//jump
	jumpAnim.LoadAnimations("jumpAnim");


	//crouch
	crouchAnim.LoadAnimations("crouchAnim");


	//dash
	dashAnim.LoadAnimations("dashAnim");


	//attack
	attackAnim.LoadAnimations("attackAnim");

	//die
	dieAnim.LoadAnimations("dieAnim");

}

Player::~Player() {

}

bool Player::Awake() {

	position.x = parameters.attribute("x").as_int();
	position.y = parameters.attribute("y").as_int();
	texturePath = parameters.attribute("texturepath").as_string();
	speed = parameters.attribute("speed").as_float();
	speedDash = parameters.attribute("speedDash").as_float();
	jumpForce = parameters.attribute("jumpForce").as_float();
	timerDash = Timer();

	return true;
}

bool Player::Start() {

	//initilize textures
	texture = app->tex->Load(texturePath);

	pbody = app->physics->CreateCircle(position.x, position.y + 12, 14, bodyType::DYNAMIC);
	pbody->listener = this;
	pbody->ctype = ColliderType::PLAYER;
	pbody->body->SetFixedRotation(false);

	pickCoinFxId = app->audio->LoadAudios("pickCoin");
	jumpFxId = app->audio->LoadAudios("Jump");

	initialPos = pbody->body->GetTransform();
		

	return true;
}

bool Player::Update(float dt)
{
	currentAnim = &idleAnim;
	isCrouching = false;
	
	b2Vec2 vel = b2Vec2(0, -GRAVITY_Y);
	vel.y = pbody->body->GetLinearVelocity().y;

	if (app->input->GetKey(SDL_SCANCODE_F1) == KEY_DOWN || app->input->GetKey(SDL_SCANCODE_F2) == KEY_DOWN) {
		pbody->body->SetTransform(b2Vec2(initialPos.p.x, initialPos.p.y), initialPos.q.GetAngle());
	}
	if (app->input->GetKey(SDL_SCANCODE_F3) == KEY_DOWN) {
		pbody->body->SetTransform(b2Vec2(initialPos.p.x, initialPos.p.y), initialPos.q.GetAngle());
	}

	//Godmode
	if (app->input->GetKey(SDL_SCANCODE_F10) == KEY_DOWN) {
		app->godmode = !app->godmode;
	}
	//Movement
	if (app->godmode == true) {
		
		b2Vec2 vel = b2Vec2(0, 0);

		if (app->input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT) {
			vel = b2Vec2(0, -speed * dt);
		}
		if (app->input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT) {
			vel = b2Vec2(0, speed * dt);
		}

		if (app->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT && !isDashing) {
			vel = b2Vec2(-speed * dt, 0);
			isFacingLeft = true;
		}

		if (app->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT && !isDashing) {
			vel = b2Vec2(speed * dt, 0);
			isFacingLeft = false;
		}
		pbody->body->SetLinearVelocity(vel);
		pbody->body->SetGravityScale(0);
		pbody->body->GetFixtureList()[0].SetSensor(true);
		isDying = false;

	}

	//Jump


	if (app->godmode == false) {

		pbody->body->SetGravityScale(1);
		pbody->body->GetFixtureList()[0].SetSensor(false);
		
		
		if (app->input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT && !isDying && !isAttacking) {
			currentAnim = &crouchAnim;
			isCrouching = true;
		}
		if (app->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT && !isDashing && !isDying &&!isCrouching && !isAttacking) {
			vel = b2Vec2(-speed * dt, pbody->body->GetLinearVelocity().y);
			currentAnim = &runAnim;
			isFacingLeft = true;
		}

		if (app->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT && !isDashing && !isDying && !isCrouching && !isAttacking) {
			vel = b2Vec2(speed * dt, pbody->body->GetLinearVelocity().y);
			currentAnim = &runAnim;
			isFacingLeft = false;
		}
		
		pbody->body->SetLinearVelocity(vel);

		if (app->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN && !isDying && !isAttacking) {
			if (isJumping == false) {
				vel.y = 0;
				pbody->body->SetLinearVelocity(vel);
				pbody->body->ApplyLinearImpulse(b2Vec2(0, GRAVITY_Y * jumpForce), pbody->body->GetWorldCenter(), true);
				isJumping = true;
				app->audio->PlayFx(jumpFxId);
			}

		}
		//Dash
		if (app->input->GetKey(SDL_SCANCODE_E) == KEY_DOWN && !isDying && !isAttacking) {
			if (!isDashing) {
				timerDash.Start();
				if (isFacingLeft) {
					vel = b2Vec2(-speedDash * dt, 0);
					pbody->body->SetLinearVelocity(vel);
					currentAnim = &dashAnim;
				}
				else {
					vel = b2Vec2(speedDash * dt, 0);
					pbody->body->SetLinearVelocity(vel);
					currentAnim = &dashAnim;
				}
				pbody->body->GetFixtureList()[0].GetShape()->m_radius = PIXEL_TO_METERS(7);
				isDashing = true;
				offsetTexY = 12;
			}

		}
		if (timerDash.ReadMSec() < 500 && isDashing) {
			if (isFacingLeft) {
				vel = b2Vec2(-speedDash * dt, 0);
				pbody->body->SetLinearVelocity(vel);
				currentAnim = &dashAnim;
			}
			else {
				vel = b2Vec2(speedDash * dt, 0);
				pbody->body->SetLinearVelocity(vel);
				currentAnim = &dashAnim;
			}

			isDashing = true;
		}
		//Attack
		if (app->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_DOWN && !isDying && !isDashing && !isJumping && !isAttacking) {
			isAttacking = true;
			if (isFacingLeft) {
				sensor = app->physics->CreateRectangleSensor(position.x - 10, position.y + 15, 30, 40, bodyType::STATIC);
				sensor->ctype = ColliderType::ATTACK;
			}
			else {
				sensor = app->physics->CreateRectangleSensor(position.x + 40, position.y + 15, 30, 40, bodyType::STATIC);
				sensor->ctype = ColliderType::ATTACK;
			}
		}

		if (isAttacking) {


			currentAnim = &attackAnim;
			if (attackAnim.HasFinished() == true) {
				isAttacking = false;
				attackAnim.Reset();
				currentAnim = &idleAnim;
				sensor->body->GetWorld()->DestroyBody(sensor->body);
			}
		}

		if (app->input->GetKey(SDL_SCANCODE_R) == KEY_DOWN) {
			isDying = true;
		}

		//Die
		if (isDying) {
			currentAnim = &dieAnim;
			if (dieAnim.HasFinished()) {
				pbody->body->SetTransform(b2Vec2(initialPos.p.x, initialPos.p.y), initialPos.q.GetAngle());
				isDying = false;
				dieAnim.Reset();
				isFacingLeft = false;
			}
		}
		//Dash
		if (timerDash.ReadMSec() > 500 && isDashing) {

			isDashing = false;
			offsetTexY = 6;
			pbody->body->GetFixtureList()[0].GetShape()->m_radius = PIXEL_TO_METERS(14);
		}
	}

	if (dashAnim.HasFinished()) { isDashing = false; };
	if (isDashing == false) { dashAnim.Reset(); }
	if (isJumping == true) { currentAnim = &jumpAnim; };
	if (isJumping == false) { jumpAnim.Reset(); };
	if (isDashing && isJumping) { currentAnim = &dashAnim; };
	//Vsync

	if (app->input->GetKey(SDL_SCANCODE_F11) == KEY_DOWN) {
		app->render->vsync = !app->render->vsync;
	}


	//Update player position in pixels
	position.x = METERS_TO_PIXELS(pbody->body->GetTransform().p.x) - 16;
	position.y = METERS_TO_PIXELS(pbody->body->GetTransform().p.y) - 16;

	currentAnim->Update();
	

	// Cam Movement
	uint windowH;
	uint windowW;

	app->win->GetWindowSize(windowW, windowH);
	
	if (!app->godmode) {
		if (position.x < 255) {
			app->render->camera.x = -20;
		}
		else {
			app->render->camera.x = -position.x * app->win->GetScale() + (windowW / 2) - 25;

		}
		if (position.x > 3156) {
			app->render->camera.x = -5823;
		}
		if (position.y < 120) {
			app->render->camera.y = 0;
		}
		else {

			app->render->camera.y = -position.y * app->win->GetScale() + (windowH / 2) - 19;
		}
		if (position.y > 438) {
			app->render->camera.y = -650;
		}
	}



	return true;
}

bool Player::PostUpdate() {

	SDL_Rect rect = currentAnim->GetCurrentFrame();

	if (isFacingLeft) {
		app->render->DrawTexture(texture, position.x - 8, position.y - offsetTexY, SDL_FLIP_HORIZONTAL, &rect);
	}
	else {
		app->render->DrawTexture(texture, position.x - 8, position.y - offsetTexY, SDL_FLIP_NONE, &rect);
	}

	return true;
}
bool Player::CleanUp()
{

	return true;
}

void Player::OnCollision(PhysBody* physA, PhysBody* physB) {

	switch (physB->ctype)
	{
	case ColliderType::ITEM:
		LOG("Collision ITEM");
		app->audio->PlayFx(pickCoinFxId);
		app->entityManager->DestroyEntity(physB->listener);
		physB->body->SetActive(false);
		break;

	case ColliderType::WALL:
		LOG("Collision WALL");
		break;

	case ColliderType::PLATFORM:
		isJumping = false;
		LOG("Collision PLATFORM");
		break;

	case ColliderType::SPIKES:
		isDying = true;
		isJumping = false;
		LOG("Collision SPIKES");
		break;

	case ColliderType::ENEMY:
		if (physB->listener->isAttacking) {
			isDying = true;
		}
		isJumping = false;
		LOG("Collision ENEMY");
		break;

	case ColliderType::UNKNOWN:
		LOG("Collision UNKNOWN");
		break;
	}

}

bool Player::isOutOfBounds(int x, int y) {
	return true;
}