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

ScriptingEngine::ScriptingEngine(ILogger* logger, const Configuration& config) : config_(config), logger_(logger) 
{
	domain_ = nullptr;
	assembly_ = nullptr;
	image_ = nullptr;
	is_initialized_ = false;
}

ScriptingEngine::~ScriptingEngine()
{
	if (domain_)
		mono_jit_cleanup(domain_);

	for (auto componentLayout : component_layouts_)
	{
		delete componentLayout;
		delete native_bridge_;
		delete scripting_transform_layout_;
	}
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

void ScriptingEngine::GatherLayouts()
{
	if (!is_initialized_)
	{
		logger_->LogError("Trying to GatherLayouts(), but Scripting engine is not initialized");
		return;
	}

	GatherNativeBridge();
	GatherScriptingTransformLayout();
	GatherComponents();
}

ScriptingTransformComponent* ScriptingEngine::CreateScriptingTransformComponent(int32_t uid, SimpleMath::Vector3& position, SimpleMath::Vector3& scale)
{
	auto transformMonoInstance = native_bridge_->CreateTransform(uid, position, scale);
	return new ScriptingTransformComponent(scripting_transform_layout_, transformMonoInstance, logger_, uid);
}

void ScriptingEngine::CreateScriptingGameObject(int32_t uid, std::string name)
{
	native_bridge_->CreateGameObject(uid, name);
}

ScriptingComponent* ScriptingEngine::CreateComponentForObjectByName(int32_t uid, std::string name)
{
	auto componentLayout = GetComponentLayoutByName(name);
	auto componentMonoInstance = native_bridge_->CreateComponentForObjectByName(uid, name);

	if (!componentMonoInstance)
	{
		logger_->LogError("Can't create mono instance of component: " + name + " by id: " + std::to_string(uid));
		throw std::exception();
	}

	return new ScriptingComponent(componentLayout, componentMonoInstance);
}

ScriptingComponentLayout* ScriptingEngine::GetComponentLayoutByName(std::string& name)
{
	for (ScriptingComponentLayout* component : component_layouts_)
	{
		if (component->class_name == name)
			return component;
	}

	logger_->LogError("Can't find component in layouts by name: " + name);
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

void ScriptingEngine::GatherComponents()
{
	MonoClass* baseComponentClass = mono_class_from_name(
		image_,
		config_.coreNamespace.c_str(),
		config_.coreComponentClassName.c_str()
	);

	if (!baseComponentClass)
	{
		logger_->LogInfo("Failed to load base component class layout");
		return;
	}

	bool isBaseComponentClassAbstract = ValidateBaseComponentClassIsAbstract(baseComponentClass);
	if (!isBaseComponentClassAbstract)
		return;

	const MonoTableInfo* typeDefTable = mono_image_get_table_info(image_, MONO_TABLE_TYPEDEF);
	int rows = mono_table_info_get_rows(typeDefTable);

	component_layouts_.clear();
	component_layouts_.reserve(rows);

	for (int i = 0; i < rows; i++)
	{
		ProcessClassDefinition(i, typeDefTable, baseComponentClass);
	}

	logger_->LogInfo("Gathered " + std::to_string(component_layouts_.size()) +
		" component type layouts");
}

void ScriptingEngine::GatherNativeBridge()
{
	MonoClass* nativeBridgeLayoutClass = mono_class_from_name(
		image_,
		config_.coreNamespace.c_str(),
		config_.coreNativeBridgeClassName.c_str()
	);

	if (!nativeBridgeLayoutClass)
	{
		logger_->LogInfo("Failed to load native bridge class layout");
		return;
	}
	
	MonoMethodDesc* create_transform_method_descriptor = mono_method_desc_new(config_.createTransformMethodName.c_str(), true);
	MonoMethodDesc* create_game_object_method_descriptor = mono_method_desc_new(config_.createGameObjectMethodName.c_str(), true);
	MonoMethodDesc* create_component_method_descriptor = mono_method_desc_new(config_.createComponentMethodName.c_str(), true);

	MonoMethod* create_transform_method = create_transform_method_descriptor ?
		mono_method_desc_search_in_class(create_transform_method_descriptor, nativeBridgeLayoutClass) : nullptr;

	MonoMethod* create_game_object_method = create_game_object_method_descriptor ?
		mono_method_desc_search_in_class(create_game_object_method_descriptor, nativeBridgeLayoutClass) : nullptr;

	MonoMethod* create_component_method = create_component_method_descriptor ?
		mono_method_desc_search_in_class(create_component_method_descriptor, nativeBridgeLayoutClass) : nullptr;

	if (create_transform_method_descriptor)
		mono_method_desc_free(create_transform_method_descriptor);

	if (create_game_object_method_descriptor)
		mono_method_desc_free(create_game_object_method_descriptor);

	if (create_component_method_descriptor)
		mono_method_desc_free(create_component_method_descriptor);

	NativeBridge* layout = new NativeBridge(
		nativeBridgeLayoutClass,
		domain_,
		config_.coreNamespace,
		config_.coreNativeBridgeClassName,
		create_transform_method,
		create_game_object_method,
		create_component_method
	);

	layout->Initialize();
	native_bridge_ = layout;

	logger_->LogInfo(layout->GetDescription());
}

void ScriptingEngine::GatherScriptingTransformLayout()
{
	MonoClass* sciptingTransformLayoutClass = mono_class_from_name(
		image_,
		config_.coreNamespace.c_str(),
		config_.coreTransformClassName.c_str()
	);

	if (!sciptingTransformLayoutClass)
	{
		logger_->LogInfo("Failed to load transform class layout");
		return;
	}

	MonoClassField* positionField = mono_class_get_field_from_name(sciptingTransformLayoutClass, config_.transformPositionFieldName.c_str());
	MonoClassField* rotationField = mono_class_get_field_from_name(sciptingTransformLayoutClass, config_.transformRotationFieldName.c_str());
	MonoClassField* scaleField = mono_class_get_field_from_name(sciptingTransformLayoutClass, config_.transformScaleFieldName.c_str());

	ScriptingTransformLayout* layout = new ScriptingTransformLayout(
		sciptingTransformLayoutClass,
		config_.coreNamespace,
		config_.coreTransformClassName,
		positionField,
		scaleField,
		rotationField
	);

	scripting_transform_layout_ = layout;

	logger_->LogInfo(layout->GetDescription());
}

bool ScriptingEngine::ValidateBaseComponentClassIsAbstract(MonoClass* baseClass)
{
	uint32_t baseComponentClassFlags = mono_class_get_flags(baseClass);

	if (!(baseComponentClassFlags & MONO_TYPE_ATTR_ABSTRACT))
	{
		std::stringstream ss;
		ss << "Base component class must be abstract. Namespace: "
			<< config_.coreNamespace
			<< ", Class name: " << config_.coreComponentClassName;
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
		std::string fullStartMethodName = name_space + ":" + name + ":Start";
		std::string fullUpdateMethodName = name_space + ":" + name + ":Update";

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

		ScriptingComponentLayout* layout = new ScriptingComponentLayout(
			gatheredClass,
			name_space,
			name,
			start_method,
			update_method
		);

		component_layouts_.push_back(layout);

		logger_->LogInfo(layout->GetDescription());
	}
}

bool ScriptingEngine::IsBaseComponentClass(const std::string& name,
	const std::string& name_space) const
{
	return name == config_.coreComponentClassName &&
		name_space == config_.coreNamespace;
}

void ScriptingEngine::LogComponent(const ScriptingComponentLayout& layout)
{
	std::stringstream ss;
	ss << "Gathered component - " << layout.GetDescription();
	logger_->LogInfo(ss.str());
}