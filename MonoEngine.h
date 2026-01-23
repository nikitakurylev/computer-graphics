#pragma once
#include <string>
#include "ScriptingComponentLayout.hpp"
#include "mono/jit/jit.h"

class ScriptingEngine
{
public:
	ScriptingEngine(bool verbose);
	~ScriptingEngine();

	void GatherComponents();
	void CallUpdateComponents();

private:
	const char* mono_lib_path = "C:\\Program Files\\Mono\\lib";
	const char* mono_etc_path = "C:\\Program Files\\Mono\\etc";

	const std::string domain_name = "ScriptingBackend";
	const std::string domain_path = "Scripting\\Scripting\\bin\\Debug\\Scripting.dll";

	const char* base_component_namespace = "Core";
	const char* base_component_class_name = "Component";

	MonoAssembly* assembly;
	MonoDomain* domain;
	MonoImage* image;

	std::vector<ScriptingComponentLayout> gathered_component_layouts;

	bool verbose;
};