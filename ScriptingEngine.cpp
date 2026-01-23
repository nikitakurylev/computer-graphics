#include <stdexcept>
#include <vector>
#include <iostream>
#include <mono/metadata/assembly.h>
#include <mono/metadata/class.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>
#include <mono/metadata/attrdefs.h>

#include "Logger.hpp"
#include "ScriptingEngine.h"

ScriptingEngine::ScriptingEngine(ILogger* logger, const Configuration& config) : config_(config), logger_(logger) { }

ScriptingEngine::~ScriptingEngine()
{
	if (domain_)
		mono_jit_cleanup(domain_);
}

bool ScriptingEngine::Init()
{
	try
	{
		mono_set_dirs(config_.monoLibPath.c_str(), config_.monoEtcPath.c_str());

		domain_ = mono_jit_init(config_.domainName.c_str());

		if (!domain_)
			throw std::runtime_error(std::string("Failed to init mono domain with name: ") + config_.domainName);

		LoadAssembly();

		is_initialized_ = true;
		logger_->LogInfo("Scripting engine initialized successfully");

		return true;
	}
	catch (const std::exception& e)
	{
		logger_->LogError(std::string("Initialization failed: ") + e.what());
		return false;
	}
}

void ScriptingEngine::GatherComponents()
{
	if (!is_initialized_)
	{
		logger_->LogError("Trying to GatherComponents(), but Scripting engine is not initialized");
		return;
	}

	MonoClass* baseComponentClass = mono_class_from_name(
		image_,
		config_.baseComponentNamespace.c_str(),
		config_.baseComponentClassName.c_str()
	);

	if (!baseComponentClass)
	{
		logger_->LogInfo("Failed to load base component class");
		return;
	}

	bool isBaseComponentClassAbstract = ValidateBaseComponentClassIsAbstract(baseComponentClass);
	if (!isBaseComponentClassAbstract)
		return;

	const MonoTableInfo* typeDefTable = mono_image_get_table_info(image_, MONO_TABLE_TYPEDEF);
	int rows = mono_table_info_get_rows(typeDefTable);

	gathered_component_layouts_.clear();
	gathered_component_layouts_.reserve(rows);

	for (int i = 0; i < rows; i++)
	{
		ProcessClassDefinition(i, typeDefTable, baseComponentClass);
	}

	logger_->LogInfo("Gathered " + std::to_string(gathered_component_layouts_.size()) +
		" component types");
}

void ScriptingEngine::CallUpdateComponents()
{
	// Layout inside of scripting component, API through scripting component, not layout
	// objects are not here, there must be parsed from scene

	for (ScriptingComponentLayout component : gathered_component_layouts_)
	{
		//mono_method_desc_search_in_class()
	}
}

bool ScriptingEngine::LoadAssembly()
{
	assembly_ = mono_domain_assembly_open(domain_, config_.domainPath.c_str());

	if (!assembly_)
		throw std::runtime_error(std::string("Failed to open C# assembly at path: ") + config_.domainPath);

	image_ = mono_assembly_get_image(assembly_);

	if (!image_)
		throw std::runtime_error(std::string("Failed to load image from domain assembly at path: "));

	return false;
}

bool ScriptingEngine::ValidateBaseComponentClassIsAbstract(MonoClass* baseClass)
{
	uint32_t baseComponentClassFlags = mono_class_get_flags(baseClass);

	if (!(baseComponentClassFlags & MONO_TYPE_ATTR_ABSTRACT))
	{
		std::stringstream ss;
		ss << "Base component class must be abstract. Namespace: "
			<< config_.baseComponentNamespace
			<< ", Class name: " << config_.baseComponentClassName;
		logger_->LogError(ss.str());
		return false;
	}

	return true;
}

void ScriptingEngine::ProcessClassDefinition(int rowIndex,
	const MonoTableInfo* typeDefTable,
	MonoClass* baseComponentClass)
{
	MonoClass* gatheredClass = nullptr;
	uint32_t cols[MONO_TYPEDEF_SIZE];

	mono_metadata_decode_row(typeDefTable, rowIndex, cols, MONO_TYPEDEF_SIZE);

	std::string name = mono_metadata_string_heap(image_, cols[MONO_TYPEDEF_NAME]);
	std::string name_space = mono_metadata_string_heap(image_, cols[MONO_TYPEDEF_NAMESPACE]);

	gatheredClass = mono_class_from_name(
		image_,
		name_space.c_str(),
		name.c_str()
	);

	if (IsBaseComponentClass(name, name_space))
		return;

	if (mono_class_is_assignable_from(baseComponentClass, gatheredClass))
	{
		std::string fullStartMethodName = name_space + ":" + name + ":Start()";
		std::string fullUpdateMethodName = name_space + ":" + name + ":Update()";

		MonoMethodDesc* start_method_descriptor = mono_method_desc_new(fullStartMethodName.c_str(), true);
		MonoMethodDesc* update_method_descriptor = mono_method_desc_new(fullUpdateMethodName.c_str(), true);

		MonoMethod* start_method = start_method_descriptor ?
			mono_method_desc_search_in_class(start_method_descriptor, gatheredClass) : nullptr;

		MonoMethod* update_method = update_method_descriptor ?
			mono_method_desc_search_in_class(update_method_descriptor, gatheredClass) : nullptr;

		if (start_method_descriptor)
			mono_method_desc_free(start_method_descriptor);

		if (update_method_descriptor)
			mono_method_desc_free(update_method_descriptor);

		ScriptingComponentLayout layout(
			gatheredClass,
			name_space,
			name,
			start_method,
			update_method
		);

		gathered_component_layouts_.push_back(layout);

		logger_->LogError(layout.GetDescription());
	}
}

bool ScriptingEngine::IsBaseComponentClass(const std::string& name,
	const std::string& name_space) const
{
	return name == config_.baseComponentClassName &&
		name_space == config_.baseComponentNamespace;
}

void ScriptingEngine::LogComponent(const ScriptingComponentLayout& layout)
{
	std::stringstream ss;
	ss << "Gathered component - " << layout.GetDescription();
	logger_->LogInfo(ss.str());
}