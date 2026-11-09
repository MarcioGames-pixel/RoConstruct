#include "webgl2_device.h"

#include <cstdio>

namespace {
const char* kVertexShader = R"GLSL(#version 300 es
layout(location = 0) in vec2 position;
layout(location = 1) in vec3 color;
out vec3 vertexColor;
void main() {
    vertexColor = color;
    gl_Position = vec4(position, 0.0, 1.0);
}
)GLSL";

const char* kFragmentShader = R"GLSL(#version 300 es
precision mediump float;
in vec3 vertexColor;
out vec4 fragmentColor;
uniform float intensity;
void main() {
    fragmentColor = vec4(vertexColor * intensity, 1.0);
}
)GLSL";
}

WebGL2Device::~WebGL2Device() {
    shutdown();
}

GLuint WebGL2Device::compileShader(GLenum type, const char* source) {
    const GLuint shader = glCreateShader(type);
    if (!shader) return 0;

    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint compiled = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        char log[1024] = {};
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        std::printf("[RoConstruct WebGL] shader compile failed: %s\n", log);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

bool WebGL2Device::initialize() {
    const GLuint vertexShader = compileShader(GL_VERTEX_SHADER, kVertexShader);
    const GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, kFragmentShader);
    if (!vertexShader || !fragmentShader) {
        if (vertexShader) glDeleteShader(vertexShader);
        if (fragmentShader) glDeleteShader(fragmentShader);
        return false;
    }

    program_ = glCreateProgram();
    glAttachShader(program_, vertexShader);
    glAttachShader(program_, fragmentShader);
    glLinkProgram(program_);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    GLint linked = GL_FALSE;
    glGetProgramiv(program_, GL_LINK_STATUS, &linked);
    if (!linked) {
        char log[1024] = {};
        glGetProgramInfoLog(program_, sizeof(log), nullptr, log);
        std::printf("[RoConstruct WebGL] program link failed: %s\n", log);
        shutdown();
        return false;
    }

    // Diagnostic geometry only; real RenderLib geometry is not connected yet.
    const GLfloat vertices[] = {
         0.00f,  0.72f, 0.95f, 0.70f, 0.25f,
        -0.72f, -0.62f, 0.20f, 0.72f, 1.00f,
         0.72f, -0.62f, 0.35f, 0.95f, 0.55f
    };

    glGenVertexArrays(1, &vertexArray_);
    glBindVertexArray(vertexArray_);
    glGenBuffers(1, &vertexBuffer_);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat),
                          reinterpret_cast<void*>(2 * sizeof(GLfloat)));

    intensityLocation_ = glGetUniformLocation(program_, "intensity");
    return glGetError() == GL_NO_ERROR;
}

void WebGL2Device::beginFrame(int width, int height) {
    glViewport(0, 0, width, height);
    glClearColor(0.035f, 0.055f, 0.09f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void WebGL2Device::drawDiagnosticGeometry(float intensity) {
    if (!program_ || !vertexArray_) return;
    glUseProgram(program_);
    glUniform1f(intensityLocation_, intensity);
    glBindVertexArray(vertexArray_);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void WebGL2Device::shutdown() {
    if (vertexBuffer_) glDeleteBuffers(1, &vertexBuffer_);
    if (vertexArray_) glDeleteVertexArrays(1, &vertexArray_);
    if (program_) glDeleteProgram(program_);
    vertexBuffer_ = 0;
    vertexArray_ = 0;
    program_ = 0;
    intensityLocation_ = -1;
}
