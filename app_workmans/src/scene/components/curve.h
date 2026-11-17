#pragma once

namespace AppWMs {

    // "What shape is it" -- a curve (B-Spline and the like).
    //
    // Why it is a separate component from the mesh: the data is control points rather than a
    // vertex array, and rendering goes through a line pipeline rather than triangles. Mixing
    // them behind an enum adds a branch per object and loses ECS's contiguous-iteration benefit.
    //
    // TODO: fill in the contents later -- control points / degree / knot vector / closed flag, etc.
    struct CurveComponent {
        static constexpr const char* TypeName() { return "CurveComponent"; }
    };

}
