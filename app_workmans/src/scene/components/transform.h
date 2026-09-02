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
#include <glm/gtx/euler_angles.hpp>       // extractEulerAngle*** (순서별 추출)

namespace AppWMs {

    // 회전을 무엇으로 저장할지 — 블렌더 방식(결정됨).
    //
    // 마야는 rotateOrder로 **회전 순서**만 고르게 한다. 순서를 바꿔도 짐벌락은
    // 안 없어지고, 사용자가 축 순서를 머리로 시뮬레이션해야 한다.
    // 블렌더는 **표현 방식**을 고르게 해서 짐벌락이 없는 선택지(Quaternion)를
    // 실제로 제공한다. 애니메이션 키프레임이 오일러를 만나는 지점이 여기라
    // 그쪽을 택했다.
    //
    // ⚠️ 이름은 **축 적용 순서**다. EulerXYZ = X를 먼저 돌리고, 그다음 Y, 마지막 Z.
    //    쿼터니언 곱은 그 역순으로 쓴다(qz * qy * qx). glm의 함수 이름(예:
    //    extractEulerAngleXYZ)은 반대로 **곱 순서**를 가리키므로 이름만 보고
    //    짝지으면 조용히 틀린다. 아래 quatToEulerDegrees의 대응표는 수치 왕복으로
    //    확인한 것이다.
    enum class RotationMode : uint8_t {
        EulerXYZ = 0,   // 기본값. glm::quat(vec3)와 비트 단위로 동일한 결과다.
        EulerXZY,
        EulerYXZ,
        EulerYZX,
        EulerZXY,
        EulerZYX,
        Quaternion      // 짐벌락 없음. 기즈모 되돌리기가 무손실이 된다.
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

    // 길이가 0인 쿼터니언을 정규화하면 NaN이 되어 물체가 화면에서 사라진다.
    // 패널에서 성분을 드래그하다 보면 실제로 0에 닿는다.
    inline glm::quat safeNormalizeQuat(const glm::quat& q) {
        const float len = glm::length(q);
        if (len < 1e-6f)
            return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        return q / len;
    }

    // 오일러(라디안) → 쿼터니언. 축별 회전을 angleAxis로 따로 만들고
    // **적용 순서의 역순으로** 곱한다. glm의 이름 규약에 안 기대므로 헷갈릴 여지가 없다.
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

    // 쿼터니언 → 오일러(도).
    //
    // ⚠️ 아래 대응은 수치 왕복(6×6 전수 비교)으로 확인한 것이다. glm의 함수 이름은
    //    곱 순서라 우리 명명과 정확히 뒤집혀 있다 — XYZ는 ZYX 함수와 짝이다.
    //    그리고 추출 함수는 **자기 이름 순서대로** 값을 돌려주므로(첫 글자축이 t1),
    //    x/y/z 슬롯에 되꽂는 순서까지 같이 맞춰야 한다.
    inline glm::vec3 quatToEulerDegrees(const glm::quat& q, RotationMode mode) {
        // XYZ는 glm::eulerAngles가 glm::quat(vec3)의 정확한 역함수라(왕복 오차 0)
        // 행렬을 거치지 않고 바로 쓴다.
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


    // "어디에 있나" — 이 물체의 공간 상태. 단, 기준은 **부모**다(로컬).
    //
    // ⚠️ 여기엔 부모 개념이 없다. 계층(누구에게 붙어 있나)은 hierarchy.h가 맡고,
    //    부모 체인을 다 접은 결과는 아래 WorldTransformComponent가 받는다.
    struct TransformComponent {
        static constexpr const char* TypeName() { return "TransformComponent"; }

        glm::vec3 translate = { 0.0f, 0.0f , 0.0f };
        glm::vec3 scale = { 1.0f, 1.0f, 1.0f };

        // 회전은 두 벌을 나란히 든다. 어느 쪽이 진짜인지는 mode가 정한다.
        //
        // 블렌더도 오브젝트에 오일러·쿼터니언·축각을 전부 들고 rotmode로 고른다.
        // 하나만 저장하고 나머지를 그때그때 뽑는 방식이 아닌 이유:
        // 쿼터니언만 저장하면 UI에 보여줄 때마다 오일러를 추출해야 하고,
        // 그 추출이 특정 각도에서 180° 튀는 바로 그 연산이다(카메라에서 이미 걷어냄).
        glm::vec3 rotate = { 0.0f, 0.0f, 0.0f };            // 오일러, 단위 = 도
        glm::quat rotateQuat = { 1.0f, 0.0f, 0.0f, 0.0f };  // 생성자 인자 순서는 (w,x,y,z)

        TransformComponent() = default;
        TransformComponent(const TransformComponent&) = default;
        TransformComponent(const glm::vec3& pos)
            : translate(pos) {}

        RotationMode getRotationMode() const { return m_RotationMode; }

        // ⚠️ 모드만은 public 필드가 아니다. translate/scale/rotate는 그냥 값이라
        //    누가 대입하든 다음 getLocalMatrix()가 비교로 잡아낸다. 그런데 모드는
        //    바뀌는 순간 **변환**이 필요하다 — 안 하면 모드를 고르자마자 물체가
        //    회전 0으로 튄다. 변환이 일어나는 지점을 이 함수 하나로 못박는다.
        void setRotationMode(RotationMode mode) {
            if (mode == m_RotationMode)
                return;

            // 표현만 바꾸고 회전 자체는 유지한다.
            const glm::quat current = (m_RotationMode == RotationMode::Quaternion)
                ? safeNormalizeQuat(rotateQuat)
                : eulerToQuat(glm::radians(rotate), m_RotationMode);

            m_RotationMode = mode;

            if (mode == RotationMode::Quaternion) {
                rotateQuat = current;
            }
            else {
                // ⚠️ 여기가 손실 지점이다 — 쿼터니언에서 오일러를 뽑으면 특정 각도에서
                //    180° 분기로 다른 표현이 나온다. 그래도 괜찮은 이유: 이건 사용자가
                //    한 번 누르는 명시적 조작이지, 매 프레임 델타를 누적하는 경로가
                //    아니다. 카메라에서 걷어낸 추출은 후자여서 오염이 영구화됐다.
                //    블렌더의 rotation mode 전환도 똑같이 동작한다.
                rotate = quatToEulerDegrees(current, mode);
            }
        }


        // 🟢 행렬 캐시 (dirty의 국소 형태 = 메모이제이션)
        //
        // 예전엔 호출할 때마다 오일러→쿼터니언 변환 + mat4 곱 2회를 새로 했다.
        // 오브젝트가 가만히 있어도 매 프레임 전부 다시 계산하는 구조라,
        // 본이 수백 개로 늘면 그대로 프레임 비용이 된다.
        //
        // translate/rotate/scale이 public이라 패널·기즈모·역직렬화가 직접 대입한다.
        // 그래서 "쓸 때 dirty를 세우는" 방식은 한 곳만 빠뜨려도 오브젝트가
        // 조용히 안 움직인다(크래시가 아니라 오동작이라 찾기 어렵다).
        // 대신 마지막 계산에 쓴 입력을 함께 보관하고 값이 달라졌을 때만 재계산한다.
        // → 표시를 잊는 것이 원리적으로 불가능하다.
        //
        // 모드도 비교 대상에 넣는다. setRotationMode가 캐시 무효화를 잊어도
        // 여기서 잡힌다 — 플래그를 안 쓰는 이유와 같다. 비용은 1바이트 비교.
        //
        // ⚠️ 이름이 getTransform이 아니라 getLocalMatrix인 이유 —
        //    계층이 붙은 뒤로는 "transform"이 로컬인지 월드인지 호출부가 착각할 수
        //    있고, 그 실수는 크래시가 아니라 조용한 오동작이다. 이름이 좌표계를
        //    말하게 해서 호출부가 매번 한 번씩 고르게 만든다.
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

        // 이 로컬 행렬의 세대. 재계산될 때마다 1 오른다.
        // WorldTransformComponent가 "내가 마지막에 접어넣은 그 로컬이 맞나"를
        // 이 값으로 판정한다 (월드 캐시의 무효화 근거).
        //
        // getLocalMatrix()를 먼저 부르는 이유: 세대는 재계산 시점에 오르므로
        // 갱신을 거치지 않고 읽으면 한 박자 늦은 값이 나온다. "호출 순서를
        // 기억해야 맞는" 구조는 이 파일이 dirty 플래그를 버린 이유와 같은 함정이라
        // 안 틀리는 쪽을 산다. 이미 최신이면 비교만 하고 끝난다.
        uint32_t getVersion() const { getLocalMatrix(); return m_Version; }

        // 행렬을 받아 T/R/S로 분해해 넣는다. 기즈모와 리페어런팅이 쓴다.
        //
        // 🟢 Quaternion 모드에선 분해가 돌려준 쿼터니언을 **그대로** 넣는다 — 무손실이다.
        //    오일러 모드에선 여기서 한 번 추출이 일어난다. 계층 작업 때 회전 표현을
        //    같이 본 이유가 이 줄이다: 기즈모 되돌리기의 정확도가 모드에 직접 걸린다.
        //
        // ⚠️ 분해 자체는 어느 모드에서도 무손실이 아니다 — 부모에 비균등 스케일이
        //    걸리면 자식 로컬에 shear가 생기는데 T/R/S로는 표현할 수 없다.
        //    마야가 shear 어트리뷰트와 segmentScaleCompensate를 따로 두는 이유다. (미해결)
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
            // 캐시를 여기서 건드릴 필요가 없다 — 위에서 값이 바뀌었으므로
            // 다음 getLocalMatrix()가 비교에서 차이를 잡아내고 알아서 재계산한다.
        }

    private:
        // 실제 데이터라 mutable이 아니다. 바꾸려면 setRotationMode를 거쳐야 한다.
        RotationMode m_RotationMode = RotationMode::EulerXYZ;

        // mutable — getLocalMatrix()가 const라서. 논리적 상태는 안 바뀌고 캐시만 갱신된다.
        mutable glm::mat4 m_CachedMatrix = glm::mat4(1.0f);
        mutable glm::vec3 m_CachedTranslate{ 0.0f };
        mutable glm::vec3 m_CachedRotate{ 0.0f };
        mutable glm::vec3 m_CachedScale{ 0.0f };
        mutable glm::quat m_CachedQuat{ 1.0f, 0.0f, 0.0f, 0.0f };
        mutable RotationMode m_CachedMode = RotationMode::EulerXYZ;
        mutable bool      m_CacheValid = false;
        mutable uint32_t  m_Version = 0;
    };


    // "최종적으로 세상 어디에 있나" — 부모 체인을 다 접은 결과.
    //
    // 값은 Scene::updateTransforms()가 프레임당 한 번 루트→자손 순으로 채운다.
    // 여기 있는 건 결과와 "그 결과를 무엇으로부터 만들었나"의 기록뿐이다.
    //
    // ⚠️ 왜 TransformComponent에 안 합쳤나 —
    //    연산 횟수는 어느 쪽이든 같다(엔티티당 mat4 곱 1회). 저장 위치는 산술을
    //    바꾸지 않는다. 갈리는 건 **순회할 때 끌어오는 바이트 수**다.
    //      합침: 한 덩어리 ≈230B, 렌더 순회는 그중 월드 64B만 쓴다
    //            → 캐시라인 4줄을 끌어와 1줄만 사용
    //      나눔: 이 컴포넌트끼리 연속으로 붙어 있어 읽는 대로 다 쓴다
    //    entt는 타입마다 별도의 연속 배열을 쥐므로 이 분리가 그대로 레이아웃이 된다.
    //    큐브 1개에선 측정되지 않는다. 본 수백 개에서 갈리고, 이 앱의 목표가 거기다.
    //
    // ⚠️ 모든 엔티티가 갖는다(Scene::createEntity). 부모가 없으면 로컬의 사본이지만,
    //    그래야 렌더 순회가 "계층 있음/없음" 분기 없이 view 하나로 돈다.
    struct WorldTransformComponent {
        static constexpr const char* TypeName() { return "WorldTransformComponent"; }

        glm::mat4 matrix{ 1.0f };

        // 이 월드 행렬의 세대. 자식이 "부모가 움직였나"를 이 값으로 판정한다.
        uint32_t version = 0;

        // 마지막 계산에 쓴 입력들의 세대.
        // 초기값이 ~0u라 첫 패스는 비교가 반드시 어긋나 무조건 계산된다.
        uint32_t localVersionUsed = ~0u;
        uint32_t parentVersionUsed = ~0u;

        WorldTransformComponent() = default;
        WorldTransformComponent(const WorldTransformComponent&) = default;
    };

}
