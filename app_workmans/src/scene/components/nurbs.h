#pragma once

namespace AppWMs {

    // "어떤 형상인가" — NURBS 서피스.
    //
    // 메쉬·커브와 별도인 이유: 데이터가 컨트롤 포인트 격자 + knot vector라
    // 구조 자체가 다르고, 렌더도 테셀레이션을 거친 뒤에야 삼각형이 된다.
    //
    // 🚧 내용은 나중에 채운다 — 컨트롤 포인트 그리드 / u,v 차수 / knot vector /
    //    가중치 / 트림 커브 등.
    struct NurbsComponent {
        static constexpr const char* TypeName() { return "NurbsComponent"; }
    };

}
