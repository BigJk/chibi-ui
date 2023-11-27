#include <string>
#include <iostream>
#include <fstream>
#include <cutils.h>
#include <filesystem>

#include <GL/glew.h>

#include "./imgui.h"
#include "./imgui_internal.h"
#include "./backends/imgui_impl_glfw.h"
#include "./backends/imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>
#include <quickjs.h>

#include "./js_imgui.h"
#include "./js_util.h"

namespace fs = std::filesystem;

static GLFWwindow* window = nullptr;

GLFWwindow* get_window() {
    return window;
}

void pstrcpy(char *buf, int buf_size, const char *str)
{
    int c;
    char *q = buf;

    if (buf_size <= 0)
        return;

    for(;;) {
        c = *str++;
        if (c == 0 || q >= buf + buf_size - 1)
            break;
        *q++ = c;
    }
    *q = '\0';
}

/* strcat and truncate. */
char *pstrcat(char *buf, int buf_size, const char *s)
{
    int len;
    len = strlen(buf);
    if (len < buf_size)
        pstrcpy(buf + len, buf_size - len, s);
    return buf;
}

/**
 * Error callback for glfw
 * @param error the error code
 * @param description the description of the error
 */
static void glfw_error_callback(int error, const char *description) {
    printf("Glfw Error %d: %s\n", error, description);
}

/**
 * Inits glfw, opengl and imgui
 * @param w Window width
 * @param h Window height
 * @param title Window title
 * @return Pointer to the globals::ctx.window instance or a nullptr if something went wrong
 */
GLFWwindow *glfw_init(int w, int h, const std::string &title) {
    glfwSetErrorCallback(glfw_error_callback);

    // Init glfw
    if (!glfwInit())
        return nullptr;

#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(w, h, title.c_str(), nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Check if CHIBI_HIGH_DPI is set
    if (strcmp(getenv("CHIBI_HIGH_DPI"), "1") == 0) {
        const auto conf = new ImFontConfig();
        conf->RasterizerDensity = 2.0f;
        io.Fonts->AddFontDefault(conf);
    }

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // No Saved ImGui Settings
    ImGui::GetIO().IniFilename = nullptr;

    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return nullptr;
    }

    return window;
}

JSValue js_print(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    for (int i = 0; i < argc; ++i) {
        const char* str = JS_ToCString(ctx, argv[i]);
        printf("%s", str);
        JS_FreeCString(ctx, str);
    }
    printf("\n");
    return JS_UNDEFINED;
}

std::string read_file_full(const std::string& filename) {
    std::ifstream file(filename);

    if (file.is_open()) {
        std::string content;
        std::string line;
        while (getline(file, line)) {
            content += line + "\n";
        }
        file.close();
        return content;
    } else {
        return "";
    }
}

// TODO: modernize
int js_module_set_import_meta(JSContext *ctx, JSValueConst func_val, JS_BOOL use_realpath, JS_BOOL is_main)
{
    JSModuleDef *m;
    char buf[PATH_MAX + 16];
    JSValue meta_obj;
    JSAtom module_name_atom;
    const char *module_name;

    assert(JS_VALUE_GET_TAG(func_val) == JS_TAG_MODULE);
    m = (JSModuleDef*)JS_VALUE_GET_PTR(func_val);

    module_name_atom = JS_GetModuleName(ctx, m);
    module_name = JS_AtomToCString(ctx, module_name_atom);
    JS_FreeAtom(ctx, module_name_atom);
    if (!module_name)
        return -1;
    if (!strchr(module_name, ':')) {
        strcpy(buf, "file://");
#if !defined(_WIN32)
        /* realpath() cannot be used with modules compiled with qjsc
           because the corresponding module source code is not
           necessarily present */
        if (use_realpath) {
            char *res = realpath(module_name, buf + strlen(buf));
            if (!res) {
                JS_ThrowTypeError(ctx, "realpath failure");
                JS_FreeCString(ctx, module_name);
                return -1;
            }
        } else
#endif
        {
            pstrcat(buf, sizeof(buf), module_name);
        }
    } else {
        pstrcpy(buf, sizeof(buf), module_name);
    }
    JS_FreeCString(ctx, module_name);

    meta_obj = JS_GetImportMeta(ctx, m);
    if (JS_IsException(meta_obj))
        return -1;

    JS_DefinePropertyValueStr(ctx, meta_obj, "url", JS_NewString(ctx, buf), JS_PROP_C_W_E);
    JS_DefinePropertyValueStr(ctx, meta_obj, "main", JS_NewBool(ctx, is_main), JS_PROP_C_W_E);
    JS_FreeValue(ctx, meta_obj);
    return 0;
}

// TODO: modernize
JSModuleDef *js_module_loader(JSContext *ctx, const char *module_name, void *opaque)
{
    printf("Loading module: %s\n", module_name);

    const std::string file = read_file_full(module_name);
    if(file.empty()) {
        printf("Could not load module: %s\n", module_name);
        return nullptr;
    }

    JSValue func_val = JS_Eval(ctx, file.data(), file.length(), module_name,JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_COMPILE_ONLY);

    if (JS_IsException(func_val))
        return nullptr;

    /* XXX: could propagate the exception */
    js_module_set_import_meta(ctx, func_val, true, false);

    /* the module is already referenced, so we must free it */
    JSModuleDef *m = (JSModuleDef*)JS_VALUE_GET_PTR(func_val);
    JS_FreeValue(ctx, func_val);

    return m;
}

void js_glfw_init(JSContext* ctx) {
    JSValue global = JS_GetGlobalObject(ctx);

    JS_SetPropertyStr(ctx, global, "setWindowTitle", JS_NewCFunction(ctx, [](JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
        if (argc != 1) {
            JS_ThrowInternalError(ctx, "setWindowTitle() expects 1 argument");
            return JS_EXCEPTION;
        }

        const char* title = JS_ToCString(ctx, argv[0]);
        glfwSetWindowTitle(get_window(), title);
        JS_FreeCString(ctx, title);

        return JS_UNDEFINED;
    }, "setWindowTitle", 1));
    JS_SetPropertyStr(ctx, global, "setWindowSize", JS_NewCFunction(ctx, [](JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
        if (argc != 2) {
            JS_ThrowInternalError(ctx, "setWindowSize() expects 2 arguments");
            return JS_EXCEPTION;
        }

        int width, height;
        JS_ToInt32(ctx, &width, argv[0]);
        JS_ToInt32(ctx, &height, argv[1]);
        glfwSetWindowSize(get_window(), width, height);

        return JS_UNDEFINED;
    }, "setWindowSize", 2));
}

/**
 * Entry-Point of project
 * @return Exit-Code
 */
int main(int argc, char* argv[]) {
    // chibi-ui <entrypoint.js>
    if (argc < 2) {
        printf("usage: chibi-ui <entrypoint.js>\n");
        return -1;
    }

    window = glfw_init(1280, 720, "chibi-ui");
    if (!window) {
        printf("something went wrong... exiting");
        return -1;
    }

    const fs::path entrypoint_file = fs::path(argc == 1 ? "./entrypoint.js" : argv[1]);
    const auto entrypoint = read_file_full(entrypoint_file.string());
    fs::current_path(entrypoint_file.parent_path());

    if (entrypoint.empty()) {
        printf("entrypoint.js not found... exiting");
        return -1;
    }

    JSRuntime* runtime  = JS_NewRuntime();
    JS_SetModuleLoaderFunc(runtime, nullptr, js_module_loader, nullptr);

    JSContext* ctx      = JS_NewContext(runtime);
    JSValue global      = JS_GetGlobalObject(ctx);

    js_imgui_init(ctx);
    js_glfw_init(ctx);
    JS_SetProperty(ctx, global, JS_NewAtom(ctx, "print"), JS_NewCFunction(ctx, js_print, "print", 0));

    JSValue res = JS_Eval(ctx, entrypoint.c_str(), entrypoint.length(), entrypoint_file.filename().c_str(), JS_EVAL_TYPE_MODULE | JS_EVAL_TYPE_GLOBAL);
    if (!JS_IsFunction(ctx, res)) {
        res = JS_GetPropertyStr(ctx, global, "tick");
        if (!JS_IsFunction(ctx, res)) {
            printf("tick() function could not be located... exiting");
            return -1;
        }
    }

    /*
     * Variables
     */
    float delta_time = 0.0f;
    float last_frame = 0.0f;
    JSValue js_delta_time = JS_NewFloat64(ctx, 0);
    JSAtom js_delta_time_atom = JS_NewAtom(ctx, "dt");

    /*
     * Main Loop
     */
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        float currentFrame = glfwGetTime();
        delta_time = currentFrame - last_frame;
        last_frame = currentFrame;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        {
            // Pass delta time
            {
                const JSValue js_new_delta_value = JS_NewFloat64(ctx, delta_time);
                JS_SetProperty(ctx, global, js_delta_time_atom, js_new_delta_value);
                JS_FreeValue(ctx, js_delta_time);
                js_delta_time = js_new_delta_value;
            }

            // Execute pending jobs
            if (JS_IsJobPending(runtime)) {
                JSContext *ctx1;
                JS_ExecutePendingJob(runtime, &ctx1);
            }

            // Run tick function
            JSValue ret = JS_Call(ctx, res, global, 0, nullptr);
            if (JS_IsException(ret)) {
                dump_error(ctx);
                break;
            }
        }
        ImGui::EndFrame();
        ImGui::Render();

        /*
         * OpenGL Rendering
         */
        int display_w, display_h;
        glfwMakeContextCurrent(window);
        glfwGetFramebufferSize(window, &display_w, &display_h);

        const ImVec4* color = get_background_color();

        glViewport(0, 0, display_w, display_h);
        glClearColor(color->x, color->y, color->z, color->w);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwMakeContextCurrent(window);
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    JS_FreeContext(ctx);
    JS_FreeRuntime(runtime);

    return 0;
}