#pragma once
#include "miniaudio.h"

class AudioSystem
{
public:
	AudioSystem();
	void PlaySoundClip(const char* pFilePath);
private:
	ma_engine engine;
};

