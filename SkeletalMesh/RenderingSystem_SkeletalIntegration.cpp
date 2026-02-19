// ============================================================================
// RenderingSystem_SkeletalIntegration.cpp
// Integration code for RenderingSystem to support skeletal animation
// 
// ADD THIS CODE TO YOUR RenderingSystem.cpp
// ============================================================================

// ----------------------------------------------------------------------------
// STEP 1: Add member variables to RenderingSystem class (RenderingSystem.h)
// ----------------------------------------------------------------------------

class RenderingSystem {
    // ... existing members ...
    
protected:
    // ADD THESE FOR SKELETAL ANIMATION:
    ID3D11VertexShader* skinnedVertexShader = nullptr;
    ID3D11PixelShader* skinnedPixelShader = nullptr;  // Can reuse DeferredPixelShader
    ID3D11InputLayout* skinnedInputLayout = nullptr;
    
    // ... rest of class ...
};

// ----------------------------------------------------------------------------
// STEP 2: Initialize skeletal shaders (in RenderingSystem::Initialize)
// ----------------------------------------------------------------------------

void RenderingSystem::Initialize(std::vector<GameObject*> GameObjects) {
    // ... existing initialization ...
    
    // COMPILE SKINNED VERTEX SHADER
    printf("[RenderingSystem] Compiling SkinnedVertexShader.hlsl...\n");
    
    ID3DBlob* skinnedVertexBC = nullptr;
    HRESULT hr = CompileShaderFromFile(
        L"./SimpleTexturedDirectx11/Shaders/SkinnedVertexShader.hlsl",
        nullptr,
        "main",
        "vs_5_0",
        &skinnedVertexBC
    );
    
    if (FAILED(hr)) {
        printf("[RenderingSystem] ERROR: Failed to compile SkinnedVertexShader.hlsl\n");
    } else {
        hr = Device->CreateVertexShader(
            skinnedVertexBC->GetBufferPointer(),
            skinnedVertexBC->GetBufferSize(),
            nullptr,
            &skinnedVertexShader
        );
        
        if (FAILED(hr)) {
            printf("[RenderingSystem] ERROR: Failed to create skinned vertex shader\n");
        } else {
            printf("[RenderingSystem] SkinnedVertexShader created successfully\n");
        }
        
        // CREATE INPUT LAYOUT FOR SKINNED VERTICES
        D3D11_INPUT_ELEMENT_DESC skinnedLayout[] = {
            {"POSITION",     0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"NORMAL",       0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD",     0, DXGI_FORMAT_R32G32_FLOAT,       0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TANGENT",      0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"BINORMAL",     0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT,  0, 56, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"BLENDWEIGHT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 72, D3D11_INPUT_PER_VERTEX_DATA, 0}
        };
        
        hr = Device->CreateInputLayout(
            skinnedLayout,
            ARRAYSIZE(skinnedLayout),
            skinnedVertexBC->GetBufferPointer(),
            skinnedVertexBC->GetBufferSize(),
            &skinnedInputLayout
        );
        
        if (FAILED(hr)) {
            printf("[RenderingSystem] ERROR: Failed to create skinned input layout\n");
        } else {
            printf("[RenderingSystem] Skinned input layout created successfully\n");
        }
        
        skinnedVertexBC->Release();
    }
    
    // Pixel shader - can reuse deferred pixel shader or compile separately
    skinnedPixelShader = pixelShader;  // Reuse existing
    
    printf("[RenderingSystem] Skeletal animation system initialized\n");
    
    // ... rest of initialization ...
}

// ----------------------------------------------------------------------------
// STEP 3: Modify Render() method to detect and render skinned models
// ----------------------------------------------------------------------------

void RenderingSystem::Render(GameObject* gameObject, Matrix view, Matrix projection, 
                             ID3D11VertexShader* vertex, ID3D11PixelShader* pixel, 
                             Vector3 cam_world, bool culling, bool drawDebugAABB) {
    
    // Check if this is a skinned model
    auto skinnedModelComp = gameObject->GetComponent<SkinnedModelComponent>();
    
    if (skinnedModelComp) {
        // RENDER SKINNED MODEL
        
        // Use skinned shaders
        Context->IASetInputLayout(skinnedInputLayout);
        Context->VSSetShader(skinnedVertexShader, nullptr, 0);
        Context->PSSetShader(pixel, nullptr, 0);  // Same pixel shader
        
        // Update transform buffer
        Matrix world = gameObject->GetTransform()->GetMatrix();
        UpdateTransformBuffer(world, view, projection, cam_world);
        
        // Skinned model component handles bone matrices and drawing
        skinnedModelComp->Draw(Device, Context);
        
    } else {
        // RENDER NORMAL MODEL (existing code)
        
        Context->IASetInputLayout(layout);
        Context->VSSetShader(vertex, nullptr, 0);
        Context->PSSetShader(pixel, nullptr, 0);
        
        // ... existing rendering code ...
    }
}

// ----------------------------------------------------------------------------
// STEP 4: Add GetComponent helper to GameObject (GameObject.h)
// ----------------------------------------------------------------------------

// Add this to GameObject class:

template<typename T>
T* GetComponent() {
    for (auto comp : components) {
        T* result = dynamic_cast<T*>(comp);
        if (result) return result;
    }
    return nullptr;
}

// ----------------------------------------------------------------------------
// STEP 5: Clean up resources (in RenderingSystem destructor)
// ----------------------------------------------------------------------------

RenderingSystem::~RenderingSystem() {
    // ... existing cleanup ...
    
    if (skinnedVertexShader) skinnedVertexShader->Release();
    if (skinnedInputLayout) skinnedInputLayout->Release();
    // Don't release skinnedPixelShader if it's shared with pixelShader
    
    // ... rest of cleanup ...
}

// ============================================================================
// VERIFICATION CHECKLIST
// ============================================================================
/*

After adding this code, verify:

1. ✓ Shaders compile without errors
   - Check console for "SkinnedVertexShader created successfully"
   
2. ✓ Input layout created
   - Check console for "Skinned input layout created successfully"
   
3. ✓ No rendering errors
   - Check DirectX debug output
   - Look for D3D11 warnings/errors
   
4. ✓ Model renders
   - Should see Y_Bot on screen
   - May be T-pose initially (correct!)
   
5. ✓ Bone matrices updating
   - Add printf in SkinnedModelComponent::UpdateBoneMatrices()
   - Should see updates every frame

COMMON ISSUES:

Issue: "Failed to compile SkinnedVertexShader.hlsl"
Fix: Check shader file path is correct
     Make sure file is in: ./SimpleTexturedDirectx11/Shaders/

Issue: Model not visible
Fix: Check transform scale (FBX models are often huge)
     Try scale = Vector3(0.01f, 0.01f, 0.01f)

Issue: Model is black
Fix: Check default textures are set
     Make sure light system is working

Issue: Crashes in Render()
Fix: Check skinnedInputLayout is not null
     Verify SkinnedVertex size matches shader input

*/
