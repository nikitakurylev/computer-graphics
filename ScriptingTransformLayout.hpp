#pragma once
#include <string>
#include <sstream>
#include <mono/jit/jit.h>

class ScriptingTransformLayout
{
public:
	ScriptingTransformLayout(MonoClass* mono_class, std::string& class_namespace, std::string& class_name, MonoClassField* position_field, MonoClassField* scale_field, MonoClassField* rotation_field)
		: mono_class(mono_class)
		, class_namespace(class_namespace)
		, class_name(class_name)
		, position_field(position_field)
		, scale_field(scale_field)
		, rotation_field(rotation_field)
	{
	}

	std::string GetDescription() const
	{
		std::ostringstream ss;
		ss << "Gathered transform layout (namespace=" << class_namespace
			<< "), (name=" << class_name
			<< "), (positionFieldFound=" << (position_field != nullptr)
			<< "), (scaleFieldFound=" << (scale_field != nullptr)
			<< "), (rotationFieldFound=" << (rotation_field != nullptr) << ")";
		return ss.str();
	}

	MonoClass* mono_class;
	MonoClassField* position_field;
	MonoClassField* scale_field;
	MonoClassField* rotation_field;

	std::string class_name;
	std::string class_namespace;
};