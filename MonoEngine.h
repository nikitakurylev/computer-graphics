#pragma once
#include <string>
#include "mono/jit/jit.h"

class MonoEngine
{
public:
	MonoEngine(bool verbose);
	~MonoEngine();
	void GatherComponents();

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

	bool verbose;
};