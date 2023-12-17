#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "Entity.h"
#include "Point.h"
#include "SDL/include/SDL.h"
#include "Animation.h"
#include "PerfTimer.h"
#include "Timer.h"

#include "Physics.h"

struct SDL_Texture;

class Player : public Entity
{
public:

	Player();
	
	virtual ~Player();

	bool Awake();

	bool Start();

	bool Update(float dt);

	bool PostUpdate();

	bool CleanUp();

	void OnCollision(PhysBody* physA, PhysBody* physB);

	bool isOutOfBounds(int x, int y);

public:
	Timer timerDash;
	float speed;
	float speedDash;
	float jumpForce;
	const char* texturePath;
	SDL_Texture* texture = NULL;
	PhysBody* pbody;
	int pickCoinFxId;
	int offsetTexY = 6;
	bool isJumping = false;
	bool isFacingLeft = false;
	bool isDashing = false;
	bool isAttacking = false;
	bool isCrouching = false;
	bool isDying = false;

	Animation* currentAnim;

private:
	Animation idleAnim;
	Animation runAnim;
	Animation jumpAnim;
	Animation dashAnim;
	Animation crouchAnim;
	Animation attackAnim;
	Animation dieAnim;

	b2Transform initialPos;
};

#endif // __PLAYER_H__