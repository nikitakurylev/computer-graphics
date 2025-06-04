#pragma once
#include "GameComponent.h"
#include <SimpleMath.h>
#include <d3d.h>
#include <d3d11.h>

struct alignas(16) EmitterProperties
{
    DirectX::SimpleMath::Vector3 position;
    int maxNumToEmit;
    float particleLifeSpan;
};

struct alignas(16) Particle {
    DirectX::SimpleMath::Vector3 positon;
    DirectX::SimpleMath::Vector3 velocity;
    float age;
    DirectX::SimpleMath::Vector4 color;
};

struct alignas(16) SBCounterS
{
    alignas(16) UINT deadCounter;
};

class ParticleSystemComponent : public GameComponent
{
private:
    float _deltaTime;
    ID3DBlob* emitBlobCs_;
    ID3D11ComputeShader* emitCs_;

    ID3DBlob* simulateBlobCs_;
    ID3D11ComputeShader* simulateCs_;

    ID3DBlob* resetBlboCs_;
    ID3D11ComputeShader* resetCs_;

    ID3D11VertexShader* vertexShader_;
    ID3D11GeometryShader* geometryShader_;

    ID3DBlob* vertexBlob_;
    ID3DBlob* geometryBlob_;

    ID3D11BlendState* blendState_;

    EmitterProperties emitterProps_;
    ID3D11Buffer* emitterBuffer_;

    ID3D11Buffer* frameTimeBuffer_;

    ID3D11Resource* randomTexture_;
    ID3D11ShaderResourceView* randomSrv_;

    ID3D11Resource* albedoTexture_;
    ID3D11ShaderResourceView* albedoSrv_;

    ID3D11Buffer* dispatchInfoBuffer_;
    ID3D11ComputeShader* sortStep_;
    ID3D11ComputeShader* sort512_;
    ID3D11ComputeShader* sortInner512_; 
    ID3D11ComputeShader* sortInitArgs_; 

    ID3D11Buffer* particleBuffer_;
    ID3D11ShaderResourceView* particleBufferSrv_;
    ID3D11UnorderedAccessView* particleBufferUav_;

    ID3D11Buffer* deadBuffer_;
    ID3D11UnorderedAccessView* deadBufferUav_;

    ID3D11Buffer* aliveBuffer_;
    ID3D11ShaderResourceView* aliveBufferSrv_;
    ID3D11UnorderedAccessView* aliveBufferUav_;

    ID3D11Buffer* aliveCounterBuffer_;
    ID3D11Buffer* deadCounterBuffer_;

    ID3D11Buffer* indirectDrawBuffer_;
    ID3D11UnorderedAccessView* indirectDrawBufferUav_;

    ID3D11Buffer* indirectSortArgsBuffer_;
    ID3D11UnorderedAccessView* indirectSortArgsBufferUav_;

    void Sort(ID3D11DeviceContext* context);
    bool SortInitial(ID3D11DeviceContext* context, unsigned int maxSize);
    bool SortIncremental(ID3D11DeviceContext* context, unsigned int presorted, unsigned int maxSize);
    void Simulate(ID3D11DeviceContext* context);
public:
    static constexpr int MAX_PARTICLE_COUNT = 100000;
    static constexpr int X_NUMTHREADS = 1024;

    ParticleSystemComponent(Game* game);

    virtual void Draw(ID3D11Device* device, ID3D11DeviceContext* context) override;
    void Emit(ID3D11DeviceContext* context);
    virtual void Compute(ID3D11DeviceContext* context);
    virtual void Update(float deltaTime) override;
    virtual void Initialize(ID3D11Device* device, ID3D11DeviceContext* context) override;
    virtual void Reload(ID3D11DeviceContext* context);
    virtual void DestroyResources();
    void UpdateBuffer(ID3D11DeviceContext* context, ID3D11Buffer* buffer, void* data, size_t size);
};

