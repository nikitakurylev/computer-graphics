#pragma once
#include <string>
#include <sstream>
#include <mono/jit/jit.h>

class ScriptingComponentLayout
{
public:
	ScriptingComponentLayout(MonoClass* mono_class, std::string class_namespace, std::string class_name, MonoMethod* start_method = nullptr, MonoMethod* update_method = nullptr) 
		: mono_class(mono_class)
		, class_namespace(std::move(class_namespace))
		, class_name(std::move(class_name))
		, start_method(start_method)
		, update_method(update_method)
	{
	}

	std::string GetDescription() const
	{
		std::ostringstream ss;
		ss << "Gathered component class (namespace=" << class_namespace
			<< "), (name=" << class_name
			<< "), (startExist=" << (start_method != nullptr)
			<< "), (updateExist=" << (update_method != nullptr) << ")";
		return ss.str();
	}

	MonoClass* mono_class;

	MonoMethod* start_method;
	MonoMethod* update_method;

	std::string class_name;
	std::string class_namespace;
};