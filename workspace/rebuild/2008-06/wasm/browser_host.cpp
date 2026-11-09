#include <emscripten.h>
#include <emscripten/html5.h>
#include <GLES3/gl3.h>

#include <cstdio>

namespace {
EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = 0;
int frame_count = 0;

void render_frame() {
    const float pulse = 0.5f + 0.5f * (float)((frame_count++ % 240) / 239.0f);
    glViewport(0, 0, 960, 540);
    glClearColor(0.035f, 0.055f + pulse * 0.025f, 0.09f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

EM_BOOL on_key(int event_type, const EmscriptenKeyboardEvent* event, void*) {
    if (event_type == EMSCRIPTEN_EVENT_KEYDOWN) {
        std::printf("[RoConstruct WASM host] keydown: %s\n", event->key);
    }
    return EM_FALSE;
}

EM_BOOL on_click(int, const EmscriptenMouseEvent* event, void*) {
    std::printf("[RoConstruct WASM host] pointer: %d,%d\n", event->canvasX, event->canvasY);
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
        std::printf("[RoConstruct WASM host] WebGL 2 context creation failed (%d).\n", context);
        return 1;
    }
    if (emscripten_webgl_make_context_current(context) != EMSCRIPTEN_RESULT_SUCCESS) {
        std::printf("[RoConstruct WASM host] Could not activate WebGL 2 context.\n");
        return 2;
    }

    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_FALSE, on_key);
    emscripten_set_click_callback("#canvas", nullptr, EM_FALSE, on_click);
    emscripten_set_main_loop(render_frame, 60, EM_FALSE);
    return 0;
}
