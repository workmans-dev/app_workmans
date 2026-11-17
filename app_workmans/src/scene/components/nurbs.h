#pragma once

namespace AppWMs {

    // "What shape is it" -- a NURBS surface.
    //
    // Why it is separate from mesh and curve: the data is a control point grid plus a knot
    // vector, so the structure itself differs, and it only becomes triangles after tessellation.
    //
    // TODO: fill in the contents later -- control point grid / u,v degree / knot vector /
    //    weights / trim curves, etc.
    struct NurbsComponent {
        static constexpr const char* TypeName() { return "NurbsComponent"; }
    };

}
