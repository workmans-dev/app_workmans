#pragma once
#include "renderer/camera/sceneCamera.h"
#include "scene/scriptable_entity.h"
#include "core/core.h"
namespace AppWMs {

	struct TagComponent {
		std::string name;
		std::string typeName = "TagComponent";
		bool nameEditable = true;
		TagComponent() = default;
		TagComponent(const std::string& name) :name(name) {};
		TagComponent(const TagComponent&)= default;

	};

	struct VisComponent {
		int visible = true;
		std::string typeName = "VisComponent";
		VisComponent(const int& v) :visible(v) {};
		VisComponent(const VisComponent& other) :visible(other.visible) {};
		VisComponent()  = default;
	};


	struct TransformComponent {
		glm::vec3 translate = { 0.0f, 0.0f , 0.0f };
		glm::vec3 scale = { 1.0f, 1.0f, 1.0f };
		glm::vec3 rotate = { 0.0f , 0.0f , 0.0f };
		std::string typeName = "TransformComponent";

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::vec3& pos) :translate(pos) {}
		TransformComponent(const glm::vec3& pos, const glm::vec3& rot,
			const glm::vec3& sc) :translate(pos), rotate(rot), scale(sc) {}

		glm::mat4 getTransform() const{
			glm::vec3 rotate_radians = glm::radians(rotate);// radians 1.0 = dgree 360

			glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), rotate_radians.x, { 1,0,0 })*
				glm::rotate(glm::mat4(1.0f), rotate_radians.y, { 0,1,0 }) *
				glm::rotate(glm::mat4(1.0f), rotate_radians.z, { 0,0,1 });
			
			return glm::translate(glm::mat4(1.0f), translate) *
				rotation *
				glm::scale(glm::mat4(1.0f), scale);
		}
	};

	struct ShadingComponent {
		glm::vec4 color{ 0.0f, 1.0f, 1.0f , 1.0f };
		std::string typeName = "ShadingComponent";

		ShadingComponent() = default;
		ShadingComponent(const ShadingComponent&) = default;
		ShadingComponent(const glm::vec4& color) :color(color) {};
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
