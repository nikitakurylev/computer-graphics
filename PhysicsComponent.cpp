#include "PhysicsComponent.h"

void PhysicsComponent::Start()
{
	auto transform = gameObject->GetTransform();
	_bodyDef.position = q3Vec3(transform->position.x, transform->position.y, transform->position.z);

    auto axisAngle = MatrixToAxisAngle(transform->GetMatrix());

	_bodyDef.axis = q3Vec3(axisAngle.axis.x, axisAngle.axis.y, axisAngle.axis.z);
	_bodyDef.angle = axisAngle.angle;
	_body = gameObject->GetGame()->Physics.CreateBody(_bodyDef);

    q3BoxDef boxDef; // See q3Box.h for settings details
    q3Transform localSpace; // Contains position and orientation, see q3Transform.h for details
    q3Identity(localSpace);

    // Create a box at the origin with width, height, depth = (1.0, 1.0, 1.0)
    // and add it to a rigid body. The transform is defined relative to the owning body
    boxDef.Set(localSpace, q3Vec3(transform->scale.x * 2, transform->scale.y * 2, transform->scale.z * 2));
    _body->AddBox(boxDef);
}

void PhysicsComponent::Update(float deltaTime)
{
    auto transform = gameObject->GetTransform();
    auto matrix = transform->GetMatrix();
    auto axisAngle = MatrixToAxisAngle(matrix);
    Quaternion rotation;
    Vector3 scale;
    Vector3 translation;
    matrix.Decompose(scale, rotation, translation);
    auto position = q3Vec3(translation.x, translation.y, translation.z);

    _body->SetTransform(position, q3Vec3(axisAngle.axis.x, axisAngle.axis.y, axisAngle.axis.z), axisAngle.angle);
}

AxisAngle PhysicsComponent::MatrixToAxisAngle(const Matrix& m)
{
    Quaternion q = Quaternion::CreateFromRotationMatrix(m);
    q.Normalize();

    AxisAngle result;

    result.angle = 2.0f * acosf(q.w);

    float s = sqrtf(1.0f - q.w * q.w);
    if (s < 0.001f) {
        result.axis = Vector3(1.0f, 0.0f, 0.0f);
    }
    else {
        result.axis = Vector3(q.x / s, q.y / s, q.z / s);
    }

    return result;
}
