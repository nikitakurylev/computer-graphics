#pragma once
#include <string>
#include <sstream>
#include <mono/jit/jit.h>
#include "MarshalledVector3.hpp"
#include "SimpleMath.h"

using namespace DirectX;

class NativeBridge
{
public:
	NativeBridge(MonoClass* mono_class, MonoDomain* mono_domain, std::string& class_namespace, std::string& class_name,
		MonoMethod* create_transform_method, MonoMethod* create_game_object_method, MonoMethod* create_component_method)
		: mono_class(mono_class)
		, mono_domain_(mono_domain)
		, class_namespace(class_namespace)
		, class_name(class_name)
		, create_transform_method_(create_transform_method)
		, create_game_object_method_(create_game_object_method)
		, create_component_method_(create_component_method)
	{
	}

	void Initialize()
	{
		// Вызываем статический конструктор
		mono_class_init(mono_class);
	}

	void CreateGameObject(int32_t uid, std::string name)
	{
		void* args[2];
		args[0] = &uid;
		args[1] = mono_string_new(mono_domain_, name.c_str());

		mono_runtime_invoke(create_game_object_method_, NULL, args, NULL);
	}

	MonoObject* CreateComponentForObjectByName(int32_t uid, std::string& name)
	{
		void* args[2];
		args[0] = &uid;
		args[1] = mono_string_new(mono_domain_, name.c_str());

		return mono_runtime_invoke(create_component_method_, NULL, args, NULL);
	}

	MonoObject* CreateTransform(int32_t uid, SimpleMath::Vector3 position, SimpleMath::Vector3 scale)
	{
		MarshalledVector3 marshalledPosition;
		marshalledPosition.x = position.x;
		marshalledPosition.y = position.y;
		marshalledPosition.z = position.z;

		MarshalledVector3 marshalledScale;
		marshalledScale.x = scale.x;
		marshalledScale.y = scale.y;
		marshalledScale.z = scale.z;

		void* args[3];
		args[0] = &uid;
		args[1] = &marshalledPosition;
		args[2] = &scale;

		return mono_runtime_invoke(create_transform_method_, NULL, args, NULL);
	}

	std::string GetDescription() const
	{
		std::ostringstream ss;
		ss << "Gathered native bridge (namespace=" << class_namespace
			<< "), (name=" << class_name
			<< "), (createTransformMethodFound=" << (create_transform_method_ != nullptr)
			<< "), (createGameObjectMethodFound=" << (create_game_object_method_ != nullptr)
			<< "), (createComponentMethodFound=" << (create_component_method_ != nullptr) << ")";
		return ss.str();
	}

	MonoClass* mono_class;

	MonoMethod* create_transform_method_;
	MonoMethod* create_game_object_method_;
	MonoMethod* create_component_method_;
	MonoDomain* mono_domain_;

	std::string class_name;
	std::string class_namespace;
};