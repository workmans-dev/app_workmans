#pragma once
// ─────────────────────────────────────────────────────────────
//  컴포넌트 우산 헤더
//
//  분류 기준은 "이 컴포넌트가 답하는 질문"이다:
//    identity          이건 무엇인가      (이름·메타)
//    transform         어디에 있나        (로컬 변환 + 월드 결과)
//    hierarchy         무엇에 붙어 있나   (부모·자식 토폴로지)
//    mesh/curve/nurbs  어떤 형상인가      (무엇을 그리나)
//    shading           어떻게 보이나      (색·표시 여부)
//    camera            어떻게 보는가      (투영)
//    script            어떻게 움직이나    (동작)
//
//  형상을 종류별로 나눈 이유: 데이터 구조도 렌더 경로도 다르다. 하나의 컴포넌트에
//  enum으로 섞으면 오브젝트마다 분기가 생겨 ECS의 연속 순회 이점이 사라진다.
//  반대로 프리미티브(cube/plane/sphere)는 컴포넌트가 아니다 — 초기 형상을
//  찍어내는 팩토리(renderer/geometry/*)이고, 결과물은 그냥 MeshComponent다.
//
//  ⚠️ 사용 규칙
//    - sandbox(클라이언트)는 app_workmans.h를 통해 이걸 쓴다. 정상.
//    - 엔진 내부 신규 코드는 이 우산 대신 필요한 헤더만 직접 include할 것.
//      전부가 우산을 쓰면 컴포넌트 하나 고칠 때마다 엔진 전체가 재컴파일되어
//      분할한 이유가 사라진다. 이 헤더는 이행기용 다리다.
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
