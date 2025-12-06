#include "Transform.h"

Transform::Transform()
{
    scale = Vector3(1, 1, 1);
}

void Transform::Update()
{
    world_matrix =
        Matrix::CreateScale(scale)
        * Matrix::CreateFromQuaternion(rotation)
        * Matrix::CreateTranslation(position)
        * (parent == nullptr ? Matrix::Identity : parent->world_matrix);
}
