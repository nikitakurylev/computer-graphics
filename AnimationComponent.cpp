#include "AnimationComponent.h"

AnimationComponent::AnimationComponent(std::vector<TimeKey<Vector3>> positions, std::vector<TimeKey<Vector3>> scales, std::vector<TimeKey<Quaternion>> rotations)
	: _positions(positions), _scales(scales), _rotations(rotations), playbackTime(0)
{
	length = max(max(_positions.back().time, _scales.back().time), _rotations.back().time);
}

void AnimationComponent::Update(float deltaTime)
{
	playbackTime = std::fmod((playbackTime + deltaTime), length);

	auto transform = gameObject->GetTransform();

	int i = 0;

	for (i = 0; i < _positions.size() - 1 && _positions[i].time < playbackTime; i++);

	if (i == _positions.size() - 1)
		transform->position = _positions.back().value;
	else
		transform->position = Vector3::Lerp(_positions[i].value, _positions[i + 1].value, 
			InverseLerp(_positions[i].time, _positions[i + 1].time, playbackTime));

	for (i = 0; i < _scales.size() - 1 && _scales[i].time < playbackTime; i++);

	if (i == _scales.size() - 1)
		transform->scale = _scales.back().value;
	else
		transform->scale = Vector3::Lerp(_scales[i].value, _scales[i + 1].value,
			InverseLerp(_scales[i].time, _scales[i + 1].time, playbackTime));

	for (i = 0; i < _rotations.size() - 1 && _rotations[i].time < playbackTime; i++);

	if (i == _rotations.size() - 1)
		transform->rotation = _rotations.back().value;
	else
		transform->rotation = Quaternion::Lerp(_rotations[i].value, _rotations[i + 1].value,
			InverseLerp(_rotations[i].time, _rotations[i + 1].time, playbackTime));
}

float AnimationComponent::InverseLerp(float a, float b, float t)
{
	return (t - a) / (b - a);
}
