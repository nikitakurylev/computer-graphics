#pragma once
#include "ScriptingTransformLayout.hpp"
#include <string>
#include <exception>
#include <mono/jit/jit.h>
#include "ScriptingComponentLayout.hpp"
#include "Logger.hpp"
#include "MarshalledVector3.hpp"
#include "SimpleMath.h"

using namespace DirectX;

class ScriptingTransformComponent
{
public:
	ScriptingTransformComponent(ScriptingTransformLayout* scriptingTransformLayout, MonoObject* classInstance, ILogger* logger, int32_t uid)
		: scripting_transform_layout_(scriptingTransformLayout), class_instance_(classInstance), logger_(logger), uid_(uid) {}

	SimpleMath::Vector3 GetPosition()
	{
		MarshalledVector3 result;
		mono_field_get_value(class_instance_, scripting_transform_layout_->position_field, &result);

		return result.ConvertToVector();
	}

	SimpleMath::Vector3 GetScale()
	{
		MarshalledVector3 result;
		mono_field_get_value(class_instance_, scripting_transform_layout_->scale_field, &result);

		return result.ConvertToVector();
	}

	void UpdateTransform(SimpleMath::Vector3 position, SimpleMath::Vector3 scale)
	{
		MarshalledVector3 marshalledPosition;
		marshalledPosition.x = position.x;
		marshalledPosition.y = position.y;
		marshalledPosition.z = position.z;

		MarshalledVector3 marshalledScale;
		marshalledScale.x = scale.x;
		marshalledScale.y = scale.y;
		marshalledScale.z = scale.z;

		void* args[2];
		args[0] = &marshalledPosition;
		args[1] = &marshalledScale;

		mono_runtime_invoke(scripting_transform_layout_->update_transform_method, class_instance_, args, NULL);
	}

private:
	ScriptingTransformLayout* scripting_transform_layout_;
	MonoObject* class_instance_;
	ILogger* logger_;
	int32_t uid_;
};