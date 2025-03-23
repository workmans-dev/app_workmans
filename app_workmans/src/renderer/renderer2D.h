#pragma once

#include "renderer/camera/cameraController.h"
#include "core/layer.h"
#include "renderer/vertexArray.h"
#include "core/core.h"
#include "renderer/texture.h"
#include "renderer/shader.h"
#include <glm/gtc/matrix_transform.hpp>

namespace AppWMs {




    class Renderer2D {

    public:
        static void init();
        static void beginScene(const CameraController& cam, const glm::mat4& transform);

        static void endScene();
        static void flush();
        static void shutdown();
        static void drawQuad(const glm::mat4& transform, const glm::vec4& color, int e_ID);
        static void drawQuad(const glm::mat4& transform , const glm::vec4& tintColor,
             const std::string& fileName, const Ref<Texture2D>& tex_ptr, int e_ID, float tiling = 1.0f);


        struct Statistics {
            uint32_t drawCalls = 0;
            uint32_t quadCount = 0;
            uint32_t getTotalVertexCount() { return quadCount * 4; }
            uint32_t getTotalIndexCount() { return quadCount * 6; }
        };
        struct QuadInfo {
            static const int vertexCount = 4;
            glm::vec2 texCoords[vertexCount];
            glm::vec4 vertPos[vertexCount];
            QuadInfo():texCoords{ { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } }
                , vertPos{ { -0.5f, -0.5f, 0.0f, 1.0f }, { 0.5f, -0.5f, 0.0f, 1.0f },
                { 0.5f, 0.5f, 0.0f, 1.0f } , { -0.5f, 0.5f, 0.0f, 1.0f } }
            {}
        };
        static Statistics& getStats();
        static void resetStats();

    private:
        static ShaderLib pm_ShLib;

        static void end_flush();

    };
}