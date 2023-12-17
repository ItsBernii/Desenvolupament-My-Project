#ifndef __ANIMATION_H__
#define __ANIMATION_H__

#include "SDL/include/SDL_rect.h"
#define MAX_FRAMES 2500

class Animation
{
public:
	float speed = 1.0f;
	SDL_Rect frames[MAX_FRAMES];
	bool loop = true;
	// Allows the animation to keep going back and forth
	bool pingpong = false;

private:
	float currentFrame = 0.0f;
	int totalFrames = 0;
	int loopCount = 0;
	int pingpongDirection = 1;

public:

	void PushBack(const SDL_Rect& rect)
	{
		frames[totalFrames++] = rect;
	}

	void Reset()
	{
		currentFrame = 0;
		loopCount = 0;
	}

	bool HasFinished()
	{
		return !loop && !pingpong && loopCount > 0;
	}

	void Update()
	{
		currentFrame += speed;
		if (currentFrame >= totalFrames)
		{
			currentFrame = (loop || pingpong) ? 0.0f : totalFrames - 1;
			++loopCount;

			if (pingpong)
				pingpongDirection = -pingpongDirection;
		}
	}

	SDL_Rect& GetCurrentFrame()
	{
		int actualFrame = currentFrame;
		if (pingpongDirection == -1)
			actualFrame = totalFrames - currentFrame;

		return frames[actualFrame];
	}

	void LoadAnimations(const char* name) {
		pugi::xml_document configFile;
		pugi::xml_node animations;
		pugi::xml_node frame;
		pugi::xml_parse_result file = configFile.load_file("config.xml");

		animations = configFile.child("config").child("animations").child(name);
		frame = animations.child("frame");
		
		while (frame != NULL) {
			this->PushBack({frame.attribute("x").as_int(),
				frame.attribute("y").as_int(),
				frame.attribute("w").as_int(),
				frame.attribute("h").as_int()});

			frame = frame.next_sibling();
		}
		this->loop = animations.attribute("loop").as_bool();
		this->speed = animations.attribute("speed").as_float();
	}
};

#endif