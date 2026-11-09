#include <emscripten.h>
#include <emscripten/html5.h>
#include <GLES3/gl3.h>

#include <cstdio>

namespace {
EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = 0;
GLuint program = 0;
GLuint vertex_array = 0;
GLuint vertex_buffer = 0;
GLint viewport_location = -1;
int frame_count = 0;

const char* vertex_shader_source = R"GLSL(#version 300 es
layout(location = 0) in vec2 position;
layout(location = 1) in vec3 color;
out vec3 vertex_color;
void main() {
    vertex_color = color;
    gl_Position = vec4(position, 0.0, 1.0);
}
)GLSL";

const char* fragment_shader_source = R"GLSL(#version 300 es
precision mediump float;
in vec3 vertex_color;
out vec4 fragment_color;
uniform float pulse;
void main() {
    fragment_color = vec4(vertex_color * pulse, 1.0);
}
)GLSL";

GLuint compile_shader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
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

bool create_pipeline() {
    const GLuint vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex_shader_source);
    const GLuint fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_shader_source);
    if (!vertex_shader || !fragment_shader) return false;

    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    GLint linked = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
        char log[1024] = {};
        glGetProgramInfoLog(program, sizeof(log), nullptr, log);
        std::printf("[RoConstruct WebGL] program link failed: %s\n", log);
        return false;
    }

    // Interleaved position (x,y) and RGB color, forming a simple test triangle.
    const GLfloat vertices[] = {
         0.00f,  0.72f, 0.95f, 0.70f, 0.25f,
        -0.72f, -0.62f, 0.20f, 0.72f, 1.00f,
         0.72f, -0.62f, 0.35f, 0.95f, 0.55f
    };

    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat),
                          reinterpret_cast<void*>(2 * sizeof(GLfloat)));

    viewport_location = glGetUniformLocation(program, "pulse");
    return glGetError() == GL_NO_ERROR;
}

void render_frame() {
    int width = 0;
    int height = 0;
    emscripten_get_canvas_element_size("#canvas", &width, &height);
    glViewport(0, 0, width, height);
    glClearColor(0.035f, 0.055f, 0.09f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const float pulse = 0.72f + 0.28f *
        (float)(0.5 + 0.5 * __builtin_sin((double)frame_count++ * 0.035));

    glUseProgram(program);
    glUniform1f(viewport_location, pulse);
    glBindVertexArray(vertex_array);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

EM_BOOL on_key(int event_type, const EmscriptenKeyboardEvent* event, void*) {
    if (event_type == EMSCRIPTEN_EVENT_KEYDOWN) {
        std::printf("[RoConstruct WebGL] keydown: %s\n", event->key);
    }
    return EM_FALSE;
}

EM_BOOL on_click(int, const EmscriptenMouseEvent* event, void*) {
    std::printf("[RoConstruct WebGL] pointer: %d,%d\n", event->canvasX, event->canvasY);
    return EM_FALSE;
}
}

int main() {
    EmscriptenWebGLContextAttributes attributes;
    emscripten_webgl_init_context_attributes(&attributes);
    attributes.alpha = EM_FALSE;
    attributes.depth = EM_TRUE;
    attributes.stencil = EM_FALSE;
    attributes.antialias = EM_TRUE;
    attributes.majorVersion = 2;
    attributes.minorVersion = 0;

    context = emscripten_webgl_create_context("#canvas", &attributes);
    if (context <= 0) {
        std::printf("[RoConstruct WebGL] WebGL 2 context creation failed (%d).\n", context);
        return 1;
    }
    if (emscripten_webgl_make_context_current(context) != EMSCRIPTEN_RESULT_SUCCESS) {
        std::printf("[RoConstruct WebGL] Could not activate WebGL 2 context.\n");
        return 2;
    }
    if (!create_pipeline()) {
        std::printf("[RoConstruct WebGL] Could not initialize the WebGL test pipeline.\n");
        return 3;
    }

    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_FALSE, on_key);
    emscripten_set_click_callback("#canvas", nullptr, EM_FALSE, on_click);
    emscripten_set_main_loop(render_frame, 60, EM_FALSE);
    return 0;
}
