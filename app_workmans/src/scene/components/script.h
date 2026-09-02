#pragma once
#include <string>
#include <functional>

#include "core/timer.h"
#include "scene/scriptable_entity.h"

namespace AppWMs {

    // "어떻게 움직이나" — C++ 스크립트를 엔티티에 붙인다.
    //
    // ⚠️ 현재 미연결 상태다. Scene::onUpdate의 순회가 주석 처리돼 있어서
    //    bind()를 해도 실행되지 않는다. 카메라 조작을 붙일 때 되살릴 자리.
    struct NativeScriptComponent {
        static constexpr const char* TypeName() { return "NativeScriptComponent"; }

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
