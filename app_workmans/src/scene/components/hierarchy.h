#pragma once
#include <vector>
// A 92,967-line single header, but pch.h:22 already pulls it in, so the real parse cost is zero.
#include "entt.hpp"

namespace AppWMs {

    // "What is it attached to" -- pure topology. Not a single matrix in here.
    //
    // The computed result (the world matrix) is received by WorldTransformComponent in transform.h.
    // Mixing a matrix in here would put "who is the parent" (changes occasionally) and
    // "so where is it" (read every frame) on one cache line, so hierarchy editing and the
    // render traversal would evict each other. Data with different change rates stays apart.
    //
    // WARNING: never assign parent/children directly -- use Scene::setParent().
    //    (1) The link is bidirectional, so fixing one side silently desynchronizes it.
    //    (2) A cycle (A->B->A) makes the recursion in Scene::updateWorldTransform()
    //        burn the stack. That check lives inside setParent.
    //    (3) The local correction that keeps an object from moving on screen when it gets a parent is there too.
    //
    // WARNING: not serialized -- entt::entity is a runtime id that cannot survive save/load.
    //    (that is why sceneSerialize.cpp:180 matches by name rather than by id)
    //    Putting the hierarchy into .wmsa requires introducing stable ids (UUID) first.
    //
    // Why children are held in a vector rather than an intrusive list (first/next/prev):
    // this is where the outliner will show sibling order as it is and let you reorder by dragging.
    // When order is first-class information, a vector expresses it directly. If adding and
    // removing children ever shows up in a profile, that is when it becomes intrusive.
    struct RelationshipComponent {
        static constexpr const char* TypeName() { return "RelationshipComponent"; }

        entt::entity parent = entt::null;
        std::vector<entt::entity> children;

        RelationshipComponent() = default;
        RelationshipComponent(const RelationshipComponent&) = default;
    };

}
