#pragma once
#include <string>
#include "ScriptingComponentLayout.hpp"
#include "Logger.hpp"
#include "mono/jit/jit.h"

class ScriptingEngine
{
public:
	struct Configuration
	{
		std::string monoLibPath = "C:\\Program Files\\Mono\\lib";
		std::string monoEtcPath = "C:\\Program Files\\Mono\\etc";
		std::string domainName = "ScriptingBackend";
		std::string domainPath = "Scripting\\Scripting\\bin\\Debug\\Scripting.dll";
		std::string baseComponentNamespace = "Core";
		std::string baseComponentClassName = "Component";
	};

	ScriptingEngine(ILogger* logger, const Configuration& config = {});
	~ScriptingEngine();

	bool Init();
	void GatherComponents();
	void CallUpdateComponents();

private:
	bool LoadAssembly();
	bool ValidateBaseComponentClassIsAbstract(MonoClass* baseClass);
	void ProcessClassDefinition(int rowIndex, const MonoTableInfo* typeDefTable,
		MonoClass* baseComponentClass);
	bool IsBaseComponentClass(const std::string& name,
		const std::string& name_space) const;
	void LogComponent(const ScriptingComponentLayout& layout);

	MonoAssembly* assembly_;
	MonoDomain* domain_;
	MonoImage* image_;
	Configuration config_;
	ILogger* logger_;
	bool is_initialized_;

	std::vector<ScriptingComponentLayout> gathered_component_layouts_;
};