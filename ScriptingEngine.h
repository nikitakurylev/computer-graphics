#pragma once
#include <string>
#include "ScriptingComponent.hpp"
#include "ScriptingComponentLayout.hpp"
#include "NativeBridge.hpp"
#include "ScriptingTransformLayout.hpp"
#include "ScriptingTransformComponent.hpp";
#include "Logger.hpp"
#include "mono/jit/jit.h"
#include "SimpleMath.h"

using namespace DirectX;

class ScriptingEngine
{
public:
	struct Configuration
	{
		std::string monoLibPath = "C:\\Program Files\\Mono\\lib";
		std::string monoEtcPath = "C:\\Program Files\\Mono\\etc";
		std::string domainName = "ScriptingBackend";
		std::string domainPath = "Scripting\\Scripting\\bin\\Debug\\Scripting.dll";
		std::string coreNamespace = "Core";
		std::string coreComponentClassName = "Component";

		std::string coreNativeBridgeClassName = "NativeBridge";
		std::string createTransformMethodName = coreNamespace + ":" + coreNativeBridgeClassName + ":" + "CreateTransform";
		std::string createGameObjectMethodName = coreNamespace + ":" + coreNativeBridgeClassName + ":" + "CreateGameObject";
		std::string createComponentMethodName = coreNamespace + ":" + coreNativeBridgeClassName + ":" + "CreateComponent";

		std::string coreTransformClassName = "Transform";
		std::string transformPositionFieldName = "position";
		std::string transformRotationFieldName = "rotation";
		std::string transformScaleFieldName = "scale";
	};

	ScriptingEngine(ILogger* logger, const Configuration& config = {});
	~ScriptingEngine();

	bool Init();
	void GatherLayouts();

	ScriptingTransformComponent* CreateScriptingTransformComponent(int32_t uid, SimpleMath::Vector3& position, SimpleMath::Vector3& scale);
	void CreateScriptingGameObject(int32_t uid, std::string name); // for now there's no need for game object component (no functional of add component/ remove component)
	ScriptingComponent* CreateComponentForObjectByName(int32_t uid, std::string name);
	MonoAssembly* assembly_;
	MonoDomain* domain_;
	MonoImage* image_;
private:
	bool LoadAssembly();
	void GatherComponents();
	void GatherNativeBridge();
	void GatherScriptingTransformLayout();
	bool ValidateBaseComponentClassIsAbstract(MonoClass* baseClass);
	void ProcessClassDefinition(int rowIndex, const MonoTableInfo* typeDefTable, MonoClass* baseComponentClass);
	bool IsBaseComponentClass(const std::string& name, const std::string& name_space) const;
	void LogComponent(const ScriptingComponentLayout& layout);

	ScriptingComponentLayout* GetComponentLayoutByName(std::string& name);


	Configuration config_;
	ILogger* logger_;
	bool is_initialized_;

	NativeBridge* native_bridge_ = nullptr;
	ScriptingTransformLayout* scripting_transform_layout_ = nullptr;
	std::vector<ScriptingComponentLayout*> component_layouts_;
};