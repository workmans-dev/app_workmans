#pragma once
#include "renderer/camera/sceneCamera.h"
#include "scene/scriptable_entity.h"

namespace AppWMs {
	struct TagComponent {
		std::string name;
		TagComponent() = default;
		TagComponent(const std::string& name) :name(name) {};
		TagComponent(const TagComponent&)= default;

	};


	struct TransformComponent {
		glm::mat4 transform = glm::mat4(1.0f);
		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::mat4& transform) :transform(transform) {};
		operator const glm::mat4& () const { return transform; }
	};

	struct ShadingComponent {
		glm::vec4 color{ 0.0f, 1.0f, 1.0f , 1.0f };
		ShadingComponent() = default;
		ShadingComponent(const ShadingComponent&) = default;
		ShadingComponent(const glm::vec4& color) :color(color) {};
	};

	struct CameraMatrixComponent {
		SceneCamera sceneCam;
		//std::string& camType= sceneCam.get_camType();
		bool primary = true;
		bool fixed_aspectRatio = false;
		CameraMatrixComponent() = default;
		CameraMatrixComponent(const CameraMatrixComponent&) = default;
	};

	struct NativeScriptComponent {
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
			scriptable_entt_ptr = nullptr;};
			onCreate_func = [](ScriptableEntity* scriptable_entt_ptr) 
				{static_cast<T*>(scriptable_entt_ptr)->onCreate(); };
			onDestroy_func = [](ScriptableEntity* scriptable_entt_ptr)
				{static_cast<T*>(scriptable_entt_ptr)->onDestroy(); };
			onUpdate_func = [this](ScriptableEntity* scriptable_entt_ptr,Timer ts)
				{static_cast<T*>(scriptable_entt_ptr)->onUpdate(ts); };
		}
		
	};

}
