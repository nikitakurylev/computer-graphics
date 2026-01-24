#pragma once
#include "Component.h"
struct AxisAngle {
	Vector3 axis;
	float angle;
};
class PhysicsComponent : public Component
{
public:
	void Start() override;
	void Update(float deltaTime) override;
protected:
	q3BodyDef _bodyDef;
	q3Body* _body;
private:
	
	AxisAngle MatrixToAxisAngle(const Matrix& m);
};

