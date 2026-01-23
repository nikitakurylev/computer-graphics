#include <stdexcept>
#include <vector>
#include <iostream>
#include <mono/metadata/assembly.h>
#include <mono/metadata/class.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>
#include <mono/metadata/attrdefs.h>

#include "MonoEngine.h"

ScriptingEngine::ScriptingEngine(bool verbose = true)
{
	this->verbose = verbose;

	mono_set_dirs(mono_lib_path, mono_etc_path);

	domain = mono_jit_init(domain_name.c_str());

	if (!domain)
		throw std::runtime_error(std::string("Failed to init mono domain with name: ") + domain_name);

	assembly = mono_domain_assembly_open(domain, domain_path.c_str());

	if (!assembly)
		throw std::runtime_error(std::string("Failed to open C# assembly at path: ") + domain_path);

	image = mono_assembly_get_image(assembly);

	if (!image)
		throw std::runtime_error(std::string("Failed to load image from domain assembly at path: "));
}

ScriptingEngine::~ScriptingEngine()
{
	mono_jit_cleanup(domain);
}

void ScriptingEngine::GatherComponents()
{
	MonoClass* baseComponentClass = mono_class_from_name(image, base_component_namespace, base_component_class_name);

	if (!baseComponentClass)
		throw std::runtime_error(
			std::string("Failed to load base component class from image. \nNamespace: ") + 
			std::string(base_component_namespace) + 
			std::string("\nClass name: ") + 
			std::string(base_component_class_name));

	uint32_t baseComponentClassFlags = mono_class_get_flags(baseComponentClass);
	if (!(baseComponentClassFlags & MONO_TYPE_ATTR_ABSTRACT)) 
		throw std::runtime_error(
			std::string("Base component class must be abstract. \nNamespace: ") +
			std::string(base_component_namespace) +
			std::string("\nClass name: ") +
			std::string(base_component_class_name));

	const MonoTableInfo* typeDefTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
	int rows = mono_table_info_get_rows(typeDefTable);

	for (int i = 0; i < rows; i++)
	{
		MonoClass* gatheredClass = nullptr;
		uint32_t cols[MONO_TYPEDEF_SIZE];

		mono_metadata_decode_row(typeDefTable, i, cols, MONO_TYPEDEF_SIZE);
		
		std::string name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);
		std::string name_space = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
		
		gatheredClass = mono_class_from_name(image, name_space.c_str(), name.c_str());

		if (name == base_component_class_name && 
			name_space == base_component_namespace)
			continue;

		if (mono_class_is_assignable_from(baseComponentClass, gatheredClass))
		{
			MonoMethodDesc* start_method_descriptor = mono_method_desc_new((name_space + ":Start").c_str(), true);
			MonoMethodDesc* update_method_descriptor = mono_method_desc_new((name_space + ":Update").c_str(), true);

			MonoMethod* start_method = nullptr; 
			MonoMethod* update_method = nullptr; 

			if (start_method_descriptor)
				start_method = mono_method_desc_search_in_class(start_method_descriptor, gatheredClass);

			if (update_method_descriptor)
				update_method = mono_method_desc_search_in_class(update_method_descriptor, gatheredClass);

			ScriptingComponentLayout gathered_component_layout = ScriptingComponentLayout(gatheredClass, name_space, name, start_method, update_method);
			gathered_component_layouts.push_back(gathered_component_layout);

			if (verbose)
				std::cout << gathered_component_layout.GetDescription() << std::endl;
		}
	}
}

void ScriptingEngine::CallUpdateComponents()
{
	// Layout inside of scripting component, API through scripting component, not layout
	// objects are not here, there must be parsed from scene

	for (ScriptingComponentLayout component : gathered_component_layouts)
	{
		//mono_method_desc_search_in_class()
	}
}