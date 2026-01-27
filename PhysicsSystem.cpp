#include "PhysicsSystem.h"
#define FRAME_TIME 1.0 / 60.0

PhysicsSystem::PhysicsSystem() : _scene(q3Scene(FRAME_TIME)), _time(0)
{
}

q3Body* PhysicsSystem::CreateBody(q3BodyDef bodyDef)
{
    return _scene.CreateBody(bodyDef);
}

bool PhysicsSystem::Raycast(Vector3 origin, Vector3 direction, float length)
{
    q3RaycastData raycast;
    raycast.Set(q3Vec3(origin.x, origin.y, origin.z), q3Vec3(direction.x, direction.y, direction.z), length);
    RaycastCallback callback;
    callback.Init(raycast);
    _scene.RayCast(&callback, raycast);
    return callback.impactBody;
}

void PhysicsSystem::Update(float deltaTime)
{
    _time += deltaTime;
    while (_time > FRAME_TIME) {
        _time -= FRAME_TIME;
        _scene.Step();
    }
}