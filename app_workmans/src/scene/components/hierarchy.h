#pragma once
#include <vector>
// 92,967줄짜리 단일 헤더지만 pch.h:22가 이미 물고 있어 실제 파싱 비용은 0이다.
#include "entt.hpp"

namespace AppWMs {

    // "무엇에 붙어 있나" — 순수 토폴로지. 행렬은 한 톨도 없다.
    //
    // 계산 결과(월드 행렬)는 transform.h의 WorldTransformComponent가 받는다.
    // 여기에 행렬을 섞으면 "누가 부모인가"(가끔 바뀜)와 "그래서 어디에 있나"
    // (매 프레임 읽힘)가 한 캐시라인에 앉아, 계층 편집과 렌더 순회가 서로를
    // 밀어낸다. 바뀌는 빈도가 다른 데이터는 같이 두지 않는다.
    //
    // ⚠️ parent/children을 직접 대입하지 말 것 — Scene::setParent()를 쓴다.
    //    (1) 양방향 링크라 한쪽만 고치면 조용히 어긋난다.
    //    (2) 사이클(A→B→A)이 생기면 Scene::updateWorldTransform()의 재귀가
    //        스택을 태운다. 그 검사가 setParent 안에 있다.
    //    (3) 부모를 붙여도 물체가 화면에서 안 움직이게 하는 로컬 보정도 거기 있다.
    //
    // ⚠️ 직렬화 미대응 — entt::entity는 저장/로드를 넘기지 못하는 런타임 id다.
    //    (sceneSerialize.cpp:180이 id가 아니라 이름으로 매칭하는 이유가 이것이다)
    //    계층을 .wmsa에 넣으려면 안정 id(UUID) 도입이 선행 작업이다.
    //
    // 자식을 intrusive list(first/next/prev)가 아니라 vector로 든 이유:
    // 아웃라이너가 형제 순서를 그대로 보여주고 드래그로 재정렬하게 될 자리다.
    // 순서가 1급 정보면 vector가 그걸 그대로 표현한다. 자식 추가/삭제가
    // 프로파일에 잡힐 만큼 잦아지면 그때 intrusive로 바꾼다.
    struct RelationshipComponent {
        static constexpr const char* TypeName() { return "RelationshipComponent"; }

        entt::entity parent = entt::null;
        std::vector<entt::entity> children;

        RelationshipComponent() = default;
        RelationshipComponent(const RelationshipComponent&) = default;
    };

}
