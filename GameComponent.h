#pragma once
#include "Game.h"
class GameComponent
{
public:
	GameComponent(Game game);
	virtual void Draw();
protected:
	Game game;
private:
	virtual void Initialize();
	void Reload();
	void Update();
	void DestroyResources();
};

