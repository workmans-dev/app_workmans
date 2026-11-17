#pragma once
#include <string>
#include <functional>

#include "core/timer.h"
#include "scene/scriptable_entity.h"

namespace AppWMs {

    // "How does it move" -- attaches a C++ script to an entity.
    //
    // WARNING: currently not wired up. The traversal in Scene::onUpdate is commented out,
    //    so bind() has no effect. This is the spot to revive when camera control gets attached.
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
