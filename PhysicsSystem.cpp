#include "PhysicsSystem.h"
#include "qu3e/q3.h"
#define FRAME_TIME 1.0 / 60.0

PhysicsSystem::PhysicsSystem() : _scene(q3Scene(FRAME_TIME)), _time(0)
{
}

q3Body* PhysicsSystem::CreateBody(q3BodyDef bodyDef)
{
    return _scene.CreateBody(bodyDef);
}

void PhysicsSystem::Update(float deltaTime)
{
    _time += deltaTime;
    while (_time > FRAME_TIME) {
        _time -= FRAME_TIME;
        _scene.Step();
    }
}