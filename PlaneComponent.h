#pragma once
#include "Renderer.h"
#include "SimpleTexturedDirectx11/Mesh.h"
#include <d3d11.h>

// ============================================================
//  PlaneComponent
//
//  Генерирует горизонтальную плоскость размером size x size
//  с центром в начале координат. Нормали смотрят вверх (Y+).
//  Использует ту же структуру VERTEX что и ModelComponent,
//  поэтому рендерится стандартным deferred-шейдером без доп. кода.
// ============================================================
class PlaneComponent : public Renderer
{
public:
    // size   — полный размер стороны в мировых единицах
    // tiling — количество повторений UV (для шахматного паттерна и т.п.)
    explicit PlaneComponent(float size = 20.0f, float tiling = 5.0f);
    ~PlaneComponent();

    void Initialize(ID3D11Device* device, ID3D11DeviceContext* context) override;
    void Draw(ID3D11Device* device, ID3D11DeviceContext* context) override;

    bool GetGlobalAABB(BoundingBox& outBox) override;

private:
    float  size_;
    float  tiling_;
    Mesh* mesh_ = nullptr;

    // Дефолтные текстуры — белая и нейтральная нормаль (1×1 пиксель)
    ID3D11ShaderResourceView* defaultWhite_ = nullptr;
    ID3D11ShaderResourceView* defaultNormal_ = nullptr;

    void CreateDefaultTextures(ID3D11Device* device);
};