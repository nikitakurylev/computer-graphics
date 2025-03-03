#pragma once
#include "Game.h"
class GameComponent
{
public:
	GameComponent(Game* game);
	virtual void Draw();
	virtual void Update();
protected:
	Game* game;
private:
	virtual void Initialize();
	void Reload();
	void DestroyResources();
};

