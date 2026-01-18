#pragma once
#include "Component.h"

template<class T>
struct TimeKey
{
	TimeKey() {}
	TimeKey(float time, T value) : time(time), value(value) {}
	float time;
	T value;
};

class AnimationComponent : public Component
{
public:
	AnimationComponent(std::vector<TimeKey<Vector3>> positions, std::vector<TimeKey<Vector3>> scales, std::vector<TimeKey<Quaternion>> rotations);
	void Update(float deltaTime) override;

private:
	float InverseLerp(float a, float b, float t);

	std::vector<TimeKey<Vector3>> _positions;
	std::vector<TimeKey<Vector3>> _scales;
	std::vector<TimeKey<Quaternion>> _rotations;
	float length;
	float playbackTime;
};
