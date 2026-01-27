#pragma once
#include <string>
#include <mono/jit/jit.h>
#include "ScriptingComponentLayout.hpp"

class ScriptingComponent
{
public:
	ScriptingComponent(ScriptingComponentLayout* scriptingComponentLayout, MonoObject* classInstance)
		: scripting_component_layout_(scriptingComponentLayout), class_instance_(classInstance) {
	}

	void Start()
	{
		mono_runtime_invoke(scripting_component_layout_->start_method, class_instance_, NULL, NULL);
	}

	void Update(float deltaTime)
	{
		void* args[1];
		args[0] = &deltaTime;

		mono_runtime_invoke(scripting_component_layout_->update_method, class_instance_, args, NULL);
	}

private:
	ScriptingComponentLayout* scripting_component_layout_;
	MonoObject* class_instance_;
};