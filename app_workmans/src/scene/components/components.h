#pragma once
// ─────────────────────────────────────────────────────────────
//  Component umbrella header
//
//  The classification rule is "the question this component answers":
//    identity          what is this          (name, metadata)
//    transform         where is it           (local transform + world result)
//    hierarchy         what is it attached to (parent/child topology)
//    mesh/curve/nurbs  what shape is it      (what gets drawn)
//    shading           how does it look      (color, visibility)
//    camera            how does it see       (projection)
//    script            how does it move      (behavior)
//
//  Why shapes are split by kind: their data structures and render paths both differ. Mixing
//  them into one component behind an enum adds a branch per object and loses ECS's contiguous iteration.
//  Conversely, primitives (cube/plane/sphere) are not components -- they are factories that stamp
//  out an initial shape (renderer/geometry/*), and the result is just a MeshComponent.
//
//  WARNING: usage rules
//    - sandbox (the client) uses this through app_workmans.h. That is fine.
//    - New engine-internal code should include the specific headers it needs, not this umbrella.
//      If everything uses the umbrella, touching one component recompiles the whole engine
//      and the reason for splitting it up is gone. This header is a transitional bridge.
// ─────────────────────────────────────────────────────────────

#include "scene/components/identity.h"
#include "scene/components/transform.h"
#include "scene/components/hierarchy.h"
#include "scene/components/mesh.h"
#include "scene/components/curve.h"
#include "scene/components/nurbs.h"
#include "scene/components/shading.h"
#include "scene/components/camera.h"
#include "scene/components/script.h"
