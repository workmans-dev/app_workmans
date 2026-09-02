#pragma once

namespace AppWMs {

    // "어떤 형상인가" — 커브 (B-Spline 등).
    //
    // 메쉬와 별도 컴포넌트인 이유: 데이터가 정점 배열이 아니라 컨트롤 포인트고,
    // 렌더도 삼각형이 아니라 라인 파이프라인을 탄다. 한 컴포넌트에 enum으로
    // 섞으면 오브젝트마다 분기가 생겨 ECS의 연속 순회 이점이 사라진다.
    //
    // 🚧 내용은 나중에 채운다 — 컨트롤 포인트 / 차수 / knot vector / 닫힘 여부 등.
    struct CurveComponent {
        static constexpr const char* TypeName() { return "CurveComponent"; }
    };

}
