#pragma once
#include "SimpleMath.h"
#include <DirectXCollision.h>
#include <iostream>

using namespace DirectX;
using namespace DirectX::SimpleMath;


class ManualFrustumCuller
{
public:
	struct Plane {
		Vector3 normal;
		float distance;
		
		void Normalize() {
			float len = normal.Length();
			if (len > 0.0001f) {
				normal /= len;
				distance /= len;
			}
		}
		
		float DistanceToPoint(const Vector3& point) const {
			return normal.Dot(point) + distance;
		}
	};
	
	Plane planes[6]; // Left, Right, Bottom, Top, Near, Far
	
	void ExtractPlanes(const Matrix& viewProj) {
		// Left plane: row4 + row1
		planes[0].normal = Vector3(
			viewProj._14 + viewProj._11,
			viewProj._24 + viewProj._21,
			viewProj._34 + viewProj._31
		);
		planes[0].distance = viewProj._44 + viewProj._41;
		planes[0].Normalize();
		
		// Right plane: row4 - row1
		planes[1].normal = Vector3(
			viewProj._14 - viewProj._11,
			viewProj._24 - viewProj._21,
			viewProj._34 - viewProj._31
		);
		planes[1].distance = viewProj._44 - viewProj._41;
		planes[1].Normalize();
		
		// Bottom plane: row4 + row2
		planes[2].normal = Vector3(
			viewProj._14 + viewProj._12,
			viewProj._24 + viewProj._22,
			viewProj._34 + viewProj._32
		);
		planes[2].distance = viewProj._44 + viewProj._42;
		planes[2].Normalize();
		
		// Top plane: row4 - row2
		planes[3].normal = Vector3(
			viewProj._14 - viewProj._12,
			viewProj._24 - viewProj._22,
			viewProj._34 - viewProj._32
		);
		planes[3].distance = viewProj._44 - viewProj._42;
		planes[3].Normalize();
		
		// Near plane: row3
		planes[4].normal = Vector3(
			viewProj._13,
			viewProj._23,
			viewProj._33
		);
		planes[4].distance = viewProj._43;
		planes[4].Normalize();
		
		// Far plane: row4 - row3
		planes[5].normal = Vector3(
			viewProj._14 - viewProj._13,
			viewProj._24 - viewProj._23,
			viewProj._34 - viewProj._33
		);
		planes[5].distance = viewProj._44 - viewProj._43;
		planes[5].Normalize();
	}
	
	bool IntersectsAABB(const BoundingBox& aabb) const {
		for (int i = 0; i < 6; i++) {
			Vector3 positiveVertex;
			positiveVertex.x = (planes[i].normal.x >= 0) ? (aabb.Center.x + aabb.Extents.x) : (aabb.Center.x - aabb.Extents.x);
			positiveVertex.y = (planes[i].normal.y >= 0) ? (aabb.Center.y + aabb.Extents.y) : (aabb.Center.y - aabb.Extents.y);
			positiveVertex.z = (planes[i].normal.z >= 0) ? (aabb.Center.z + aabb.Extents.z) : (aabb.Center.z - aabb.Extents.z);
			
			if (planes[i].DistanceToPoint(positiveVertex) < 0) {
				return false;
			}
		}
		
		return true;
	}
	
	void PrintDebugInfo() const {
		const char* names[] = { "Left", "Right", "Bottom", "Top", "Near", "Far" };
		std::cout << "[Frustum Planes]" << std::endl;
		for (int i = 0; i < 6; i++) {
			std::cout << "  " << names[i] << ": normal=(" 
			          << planes[i].normal.x << ", " 
			          << planes[i].normal.y << ", " 
			          << planes[i].normal.z << "), distance=" 
			          << planes[i].distance << std::endl;
		}
	}
};
