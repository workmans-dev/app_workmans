#pragma once
#include <string>
#include <cstdint>

#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>   // translate / scale
#include <glm/gtx/quaternion.hpp>         // quat / mat4_cast / eulerAngles
#include <glm/gtx/matrix_decompose.hpp>   // decompose
#include <glm/gtx/euler_angles.hpp>       // extractEulerAngle*** (per-order extraction)

namespace AppWMs {

    // What rotation gets stored as -- the Blender way (decided).
    //
    // Maya only lets you pick the *rotation order*, via rotateOrder. Changing the order does not
    // remove gimbal lock, and the user has to simulate the axis order in their head.
    // Blender lets you pick the *representation*, which actually offers an option with no gimbal
    // lock (Quaternion). This is the point where animation keyframes meet Euler angles, so that
    // is the side we took.
    //
    // WARNING: these names are the *axis application order*. EulerXYZ = rotate X first, then Y, then Z.
    //    The quaternion product is written in the reverse of that (qz * qy * qx). glm's function
    //    names (e.g. extractEulerAngleXYZ) refer to the *product order* instead, so pairing them
    //    by name alone is silently wrong. The mapping table in quatToEulerDegrees below was
    //    verified by numeric round-trip.
    enum class RotationMode : uint8_t {
        EulerXYZ = 0,   // the default. Bit-for-bit identical to glm::quat(vec3).
        EulerXZY,
        EulerYXZ,
        EulerYZX,
        EulerZXY,
        EulerZYX,
        Quaternion      // no gimbal lock. Gizmo round-trips become lossless.
    };

    inline const char* rotationModeName(RotationMode mode) {
        switch (mode) {
        case RotationMode::EulerXYZ:   return "XYZ Euler";
        case RotationMode::EulerXZY:   return "XZY Euler";
        case RotationMode::EulerYXZ:   return "YXZ Euler";
        case RotationMode::EulerYZX:   return "YZX Euler";
        case RotationMode::EulerZXY:   return "ZXY Euler";
        case RotationMode::EulerZYX:   return "ZYX Euler";
        case RotationMode::Quaternion: return "Quaternion";
        }
        return "XYZ Euler";
    }

    // Normalizing a zero-length quaternion yields NaN and the object vanishes from the screen.
    // Dragging a component in the panel really does hit zero.
    inline glm::quat safeNormalizeQuat(const glm::quat& q) {
        const float len = glm::length(q);
        if (len < 1e-6f)
            return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        return q / len;
    }

    // Euler (radians) -> quaternion. Each axis rotation is built separately with angleAxis and
    // multiplied *in the reverse of the application order*, so it never leans on glm's naming.
    inline glm::quat eulerToQuat(const glm::vec3& radians, RotationMode mode) {
        const glm::quat qx = glm::angleAxis(radians.x, glm::vec3(1.0f, 0.0f, 0.0f));
        const glm::quat qy = glm::angleAxis(radians.y, glm::vec3(0.0f, 1.0f, 0.0f));
        const glm::quat qz = glm::angleAxis(radians.z, glm::vec3(0.0f, 0.0f, 1.0f));
        switch (mode) {
        case RotationMode::EulerXZY: return qy * qz * qx;
        case RotationMode::EulerYXZ: return qz * qx * qy;
        case RotationMode::EulerYZX: return qx * qz * qy;
        case RotationMode::EulerZXY: return qy * qx * qz;
        case RotationMode::EulerZYX: return qx * qy * qz;
        default:                     return qz * qy * qx;   // EulerXYZ
        }
    }

    // Quaternion -> Euler (degrees).
    //
    // WARNING: the mapping below was verified by numeric round-trip (all 6x6 compared). glm's
    //    function names are product order, exactly inverted from ours -- XYZ pairs with the ZYX function.
    //    And an extraction function returns values *in its own name's order* (the first letter's axis is t1),
    //    so the order they are put back into the x/y/z slots has to match too.
    inline glm::vec3 quatToEulerDegrees(const glm::quat& q, RotationMode mode) {
        // For XYZ, glm::eulerAngles is the exact inverse of glm::quat(vec3) (round-trip error 0),
        // so it is used directly without going through a matrix.
        if (mode == RotationMode::EulerXYZ || mode == RotationMode::Quaternion)
            return glm::degrees(glm::eulerAngles(q));

        const glm::mat4 m = glm::mat4_cast(q);
        float t1 = 0.0f, t2 = 0.0f, t3 = 0.0f;
        glm::vec3 rad(0.0f);
        switch (mode) {
        case RotationMode::EulerXZY:
            glm::extractEulerAngleYZX(m, t1, t2, t3); rad.y = t1; rad.z = t2; rad.x = t3; break;
        case RotationMode::EulerYXZ:
            glm::extractEulerAngleZXY(m, t1, t2, t3); rad.z = t1; rad.x = t2; rad.y = t3; break;
        case RotationMode::EulerYZX:
            glm::extractEulerAngleXZY(m, t1, t2, t3); rad.x = t1; rad.z = t2; rad.y = t3; break;
        case RotationMode::EulerZXY:
            glm::extractEulerAngleYXZ(m, t1, t2, t3); rad.y = t1; rad.x = t2; rad.z = t3; break;
        case RotationMode::EulerZYX:
            glm::extractEulerAngleXYZ(m, t1, t2, t3); rad.x = t1; rad.y = t2; rad.z = t3; break;
        default:
            return glm::degrees(glm::eulerAngles(q));
        }
        return glm::degrees(rad);
    }


    // "Where is it" -- this object's spatial state. The frame of reference is the *parent* (local).
    //
    // WARNING: there is no notion of a parent here. The hierarchy (what it is attached to) is
    //    hierarchy.h's job, and the fully folded parent chain is received by WorldTransformComponent below.
    struct TransformComponent {
        static constexpr const char* TypeName() { return "TransformComponent"; }

        glm::vec3 translate = { 0.0f, 0.0f , 0.0f };
        glm::vec3 scale = { 1.0f, 1.0f, 1.0f };

        // Rotation is held as two representations side by side. Which one is real is decided by mode.
        //
        // Blender likewise holds Euler, quaternion and axis-angle on an object and picks with rotmode.
        // Why not store one and derive the rest on demand:
        // storing only the quaternion means extracting Euler every time it is shown in the UI, and
        // that extraction is exactly the operation that jumps 180 degrees at certain angles (already removed from the camera).
        glm::vec3 rotate = { 0.0f, 0.0f, 0.0f };            // Euler, in degrees
        glm::quat rotateQuat = { 1.0f, 0.0f, 0.0f, 0.0f };  // constructor argument order is (w,x,y,z)

        TransformComponent() = default;
        TransformComponent(const TransformComponent&) = default;
        TransformComponent(const glm::vec3& pos)
            : translate(pos) {}

        RotationMode getRotationMode() const { return m_RotationMode; }

        // WARNING: the mode alone is not a public field. translate/scale/rotate are just values,
        //    so whoever assigns them, the next getLocalMatrix() catches it by comparison. The mode,
        //    however, needs a *conversion* the moment it changes -- without one the object snaps to
        //    zero rotation as soon as the mode is picked. This function pins down where that happens.
        void setRotationMode(RotationMode mode) {
            if (mode == m_RotationMode)
                return;

            // Only the representation changes; the rotation itself is preserved.
            const glm::quat current = (m_RotationMode == RotationMode::Quaternion)
                ? safeNormalizeQuat(rotateQuat)
                : eulerToQuat(glm::radians(rotate), m_RotationMode);

            m_RotationMode = mode;

            if (mode == RotationMode::Quaternion) {
                rotateQuat = current;
            }
            else {
                // WARNING: this is the lossy point -- extracting Euler from a quaternion gives a different
                //    representation across a 180-degree branch at certain angles. Why that is acceptable: this
                //    is an explicit action the user takes once, not a path that accumulates a delta every
                //    frame. The extraction removed from the camera was the latter, which made the damage permanent.
                //    Blender's rotation mode switch behaves the same way.
                rotate = quatToEulerDegrees(current, mode);
            }
        }


        // Matrix cache (the local form of dirty = memoization)
        //
        // It used to redo the Euler->quaternion conversion plus two mat4 multiplies on every call.
        // That recomputes everything every frame even when the object is still, so with hundreds
        // of bones it turns straight into frame cost.
        //
        // translate/rotate/scale are public, so the panel, the gizmo and deserialization assign them directly.
        // That makes a "set dirty when you write" scheme fail silently if even one site is missed --
        // the object simply does not move (an incorrect result, not a crash, so it is hard to find).
        // Instead we keep the inputs used for the last computation and recompute only when a value differs.
        // -> Forgetting to mark it is impossible in principle.
        //
        // The mode is part of the comparison too. Even if setRotationMode forgets to invalidate the
        // cache, this catches it -- the same reason flags are not used. The cost is a one-byte compare.
        //
        // WARNING: why the name is getLocalMatrix rather than getTransform --
        //    once a hierarchy exists, a caller can mistake "transform" for local or world, and that
        //    mistake is a silent misbehavior, not a crash. Making the name state the coordinate space
        //    forces the caller to choose, once, every time.
        const glm::mat4& getLocalMatrix() const {
            if (!m_CacheValid
                || translate != m_CachedTranslate
                || rotate != m_CachedRotate
                || scale != m_CachedScale
                || rotateQuat != m_CachedQuat
                || m_RotationMode != m_CachedMode) {

                const glm::quat q = (m_RotationMode == RotationMode::Quaternion)
                    ? safeNormalizeQuat(rotateQuat)
                    : eulerToQuat(glm::radians(rotate), m_RotationMode);

                m_CachedMatrix = glm::translate(glm::mat4(1.0f), translate) *
                    glm::mat4_cast(q) *
                    glm::scale(glm::mat4(1.0f), scale);

                m_CachedTranslate = translate;
                m_CachedRotate = rotate;
                m_CachedScale = scale;
                m_CachedQuat = rotateQuat;
                m_CachedMode = m_RotationMode;
                m_CacheValid = true;
                ++m_Version;
            }
            return m_CachedMatrix;
        }

        // The generation of this local matrix. It increments on every recomputation.
        // WorldTransformComponent uses this value to decide "is that the local I last folded in"
        // (the basis for invalidating the world cache).
        //
        // Why getLocalMatrix() is called first: the generation rises at recomputation time, so
        // reading it without going through the update gives a value one beat behind. A structure
        // that is "only correct if you remember the call order" is the same trap that made this file
        // drop dirty flags, so we buy the side that cannot be wrong. If already current, it just compares.
        uint32_t getVersion() const { getLocalMatrix(); return m_Version; }

        // Takes a matrix and decomposes it into T/R/S. Used by the gizmo and by reparenting.
        //
        // In Quaternion mode the quaternion the decomposition returned goes in *as is* -- lossless.
        //    In Euler mode one extraction happens here. This line is why the rotation representation
        //    was looked at alongside the hierarchy work: gizmo round-trip accuracy hangs on the mode.
        //
        // WARNING: the decomposition itself is not lossless in any mode -- a non-uniform scale on the
        //    parent produces shear in the child's local, which T/R/S cannot express.
        //    That is why Maya keeps a separate shear attribute and segmentScaleCompensate. (unresolved)
        void setLocalMatrix(const glm::mat4& new_transform) {
            glm::vec3 trans_v, skew_v, scale_v;
            glm::quat q_rotate;
            glm::vec4 persp;
            glm::decompose(new_transform, scale_v, q_rotate, trans_v, skew_v, persp);
            translate = trans_v;
            scale = scale_v;
            if (m_RotationMode == RotationMode::Quaternion)
                rotateQuat = q_rotate;
            else
                rotate = quatToEulerDegrees(q_rotate, m_RotationMode);
            // There is no need to touch the cache here -- the values changed above, so the next
            // getLocalMatrix() will see the difference in its comparison and recompute on its own.
        }

    private:
        // This is the real data, so it is not mutable. Changing it has to go through setRotationMode.
        RotationMode m_RotationMode = RotationMode::EulerXYZ;

        // mutable -- because getLocalMatrix() is const. The logical state does not change, only the cache.
        mutable glm::mat4 m_CachedMatrix = glm::mat4(1.0f);
        mutable glm::vec3 m_CachedTranslate{ 0.0f };
        mutable glm::vec3 m_CachedRotate{ 0.0f };
        mutable glm::vec3 m_CachedScale{ 0.0f };
        mutable glm::quat m_CachedQuat{ 1.0f, 0.0f, 0.0f, 0.0f };
        mutable RotationMode m_CachedMode = RotationMode::EulerXYZ;
        mutable bool      m_CacheValid = false;
        mutable uint32_t  m_Version = 0;
    };


    // "Where in the world does it finally sit" -- the result of folding the whole parent chain.
    //
    // The value is filled once per frame by Scene::updateTransforms(), root down to descendants.
    // What lives here is only the result and the record of what that result was built from.
    //
    // WARNING: why this was not merged into TransformComponent --
    //    the operation count is the same either way (one mat4 multiply per entity). Where it is
    //    stored does not change the arithmetic. What differs is *how many bytes a traversal pulls in*.
    //      merged: one ~230B lump, of which the render traversal uses only the 64B world
    //            -> pulls in 4 cache lines and uses 1
    //      split:  these components sit contiguously, so everything read gets used
    //    entt holds a separate contiguous array per type, so this split becomes the memory layout.
    //    It is not measurable with one cube. It matters at hundreds of bones, and that is this app's target.
    //
    // WARNING: every entity has one (Scene::createEntity). With no parent it is a copy of the local,
    //    but that is what lets the render traversal run on one view with no "has hierarchy" branch.
    struct WorldTransformComponent {
        static constexpr const char* TypeName() { return "WorldTransformComponent"; }

        glm::mat4 matrix{ 1.0f };

        // The generation of this world matrix. A child uses it to decide "did the parent move".
        uint32_t version = 0;

        // The generations of the inputs used for the last computation.
        // The initial value is ~0u, so the first pass is guaranteed to mismatch and always computes.
        uint32_t localVersionUsed = ~0u;
        uint32_t parentVersionUsed = ~0u;

        WorldTransformComponent() = default;
        WorldTransformComponent(const WorldTransformComponent&) = default;
    };

}
