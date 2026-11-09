#pragma once

#include <GLES3/gl3.h>

// Browser-side graphics backend boundary. This wraps WebGL 2 primitives;
// it is deliberately not presented as a G3D::RenderDevice implementation.
class WebGL2Device {
public:
    WebGL2Device() = default;
    ~WebGL2Device();

    WebGL2Device(const WebGL2Device&) = delete;
    WebGL2Device& operator=(const WebGL2Device&) = delete;

    bool initialize();
    void beginFrame(int width, int height);
    void drawDiagnosticGeometry(float intensity);
    void shutdown();

private:
    GLuint compileShader(GLenum type, const char* source);

    GLuint program_ = 0;
    GLuint vertexArray_ = 0;
    GLuint vertexBuffer_ = 0;
    GLint intensityLocation_ = -1;
};
