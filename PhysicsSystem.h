#pragma once
#include "qu3e/q3.h"
#include "SimpleMath.h"
using namespace DirectX::SimpleMath;

class PhysicsSystem
{
public:
	PhysicsSystem();
	q3Body* CreateBody(q3BodyDef bodyDef);
	bool Raycast(Vector3 origin, Vector3 direction, float length);
	void Update(float deltaTime);
private:
	class RaycastCallback : public q3QueryCallback
	{
	public:
		q3RaycastData data;
		r32 tfinal;
		q3Vec3 nfinal;
		q3Body* impactBody;

		bool ReportShape(q3Box* shape)
		{
			if (data.toi < tfinal)
			{
				tfinal = data.toi;
				nfinal = data.normal;
				impactBody = shape->body;
			}

			data.toi = tfinal;
			return true;
		}

		void Init(q3RaycastData raycast)
		{
			data = raycast;
			tfinal = FLT_MAX;
			impactBody = NULL;
		}
	};
	q3Scene _scene;
	float _time;
};

