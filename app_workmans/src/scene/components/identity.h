#pragma once
#include <string>

namespace AppWMs {

    // "What is this" -- the name and metadata.
    // Visibility (VisComponent) is a render attribute, so it went to shading.h.
    struct TagComponent {
        // Serialization key = a compile-time constant. Zero bytes per instance.
        static constexpr const char* TypeName() { return "TagComponent"; }

        std::string name;
        bool nameEditable = true;

        TagComponent() = default;
        TagComponent(const std::string& name) :name(name) {};
        TagComponent(const TagComponent&) = default;
    };

}
