#pragma once
#include "renderer/camera/sceneCamera.h"
#include "scene/scriptable_entity.h"
#include "core/core.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>


namespace AppWMs {

	struct TagComponent {
		std::string name;
		std::string typeName = "TagComponent";
		bool nameEditable = true;
		TagComponent() = default;
		TagComponent(const std::string& name) :name(name) {};
		TagComponent(const TagComponent&) = default;

	};

	struct VisComponent {
		int visible = true;
		std::string typeName = "VisComponent";
		VisComponent(const int& v) :visible(v) {};
		VisComponent(const VisComponent& other) :visible(other.visible) {};
		VisComponent() = default;
	};


	struct TransformComponent {
		glm::vec3 translate = { 0.0f, 0.0f , 0.0f };
		glm::vec3 scale = { 1.0f, 1.0f, 1.0f };
		glm::vec3 rotate = { 0.0f, 0.0f, 0.0f };
		std::string typeName = "TransformComponent";
		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::vec3& pos)
			: translate(pos) {}


		glm::mat4 getTransform() const {
			glm::quat quatRotation = glm::quat(glm::radians(rotate)); // Convert Euler to Quaternion
			return glm::translate(glm::mat4(1.0f), translate) *
				glm::mat4_cast(quatRotation) * // Convert quaternion to rotation matrix
				glm::scale(glm::mat4(1.0f), scale);
		}

		void setTransform(glm::mat4& new_transform) {
			glm::vec3 trans_v, skew_v, scale_v;
			glm::quat q_rotate;
			glm::vec4 persp;
			glm::decompose(new_transform, scale_v, q_rotate, trans_v, skew_v, persp);
			translate = trans_v;
			scale = scale_v;
			rotate = glm::degrees(glm::eulerAngles(q_rotate));

		}
	};

	struct CameraMatrixComponent {
		std::string typeName = "CameraMatrixComponent";
		SceneCamera sceneCam;
		bool primary = false;
		bool fixed_aspectRatio = false;
		bool defaultCam = false;
		CameraMatrixComponent() = default;
		CameraMatrixComponent(const CameraMatrixComponent&) = default;
	};



	struct ShadingComponent {
		glm::vec4 color{ 0.0f, 1.0f, 1.0f , 1.0f };
		std::string typeName = "ShadingComponent";

		ShadingComponent() = default;
		ShadingComponent(const ShadingComponent&) = default;
		ShadingComponent(const glm::vec4& color) :color(color) {};
	};



	struct NativeScriptComponent {
		std::string typeName = "NativeScriptComponent";

		ScriptableEntity* scriptable_entt_ptr = nullptr;
		float x = 0.0f, y = 0.0f;
		std::function<void()> instantiate_func;
		std::function<void()> destroyInst_func;
		std::function<void(ScriptableEntity*)> onCreate_func;
		std::function<void(ScriptableEntity*)> onDestroy_func;
		std::function<void(ScriptableEntity*, Timer ts)> onUpdate_func;

		template<typename T>
		void bind() {
			instantiate_func = [&]() {scriptable_entt_ptr = new T(); };
			destroyInst_func = [&]() {delete static_cast<T*>(scriptable_entt_ptr);
			scriptable_entt_ptr = nullptr; };
			onCreate_func = [](ScriptableEntity* scriptable_entt_ptr)
				{static_cast<T*>(scriptable_entt_ptr)->onCreate(); };
			onDestroy_func = [](ScriptableEntity* scriptable_entt_ptr)
				{static_cast<T*>(scriptable_entt_ptr)->onDestroy(); };
			onUpdate_func = [this](ScriptableEntity* scriptable_entt_ptr, Timer ts)
				{static_cast<T*>(scriptable_entt_ptr)->onUpdate(ts); };
		}

	};

}
