#include <emscripten.h>
#include <emscripten/html5.h>

#include "webgl2_device.h"

#include <cstdio>

namespace {
EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = 0;
WebGL2Device device;
int frameCount = 0;

void renderFrame() {
    int width = 0;
    int height = 0;
    emscripten_get_canvas_element_size("#canvas", &width, &height);
    device.beginFrame(width, height);

    const float intensity = 0.72f + 0.28f *
        (float)(0.5 + 0.5 * __builtin_sin((double)frameCount++ * 0.035));
    device.drawDiagnosticGeometry(intensity);
}

EM_BOOL onKey(int eventType, const EmscriptenKeyboardEvent* event, void*) {
    if (eventType == EMSCRIPTEN_EVENT_KEYDOWN) {
        std::printf("[RoConstruct WebGL] keydown: %s\n", event->key);
    }
    return EM_FALSE;
}

EM_BOOL onClick(int, const EmscriptenMouseEvent* event, void*) {
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
    if (!device.initialize()) {
        std::printf("[RoConstruct WebGL] Could not initialize WebGL 2 device.\n");
        return 3;
    }

    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_FALSE, onKey);
    emscripten_set_click_callback("#canvas", nullptr, EM_FALSE, onClick);
    emscripten_set_main_loop(renderFrame, 60, EM_FALSE);
    return 0;
}
