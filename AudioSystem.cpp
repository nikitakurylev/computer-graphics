#include "AudioSystem.h"

AudioSystem::AudioSystem()
{
	auto result = ma_engine_init(NULL, &engine);
	if (result != MA_SUCCESS) {
	}
}

void AudioSystem::PlaySoundClip(const char* pFilePath)
{
	auto result = ma_engine_play_sound(&engine, pFilePath, NULL);
	if (result != MA_SUCCESS) {
	}
}
