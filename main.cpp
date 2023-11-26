#include <string>
#include <iostream>
#include <fstream>

#include <GL/glew.h>

#include "./imgui.h"
#include "./imgui_internal.h"
#include "./backends/imgui_impl_glfw.h"
#include "./backends/imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>
#include <quickjs.h>

#include "./js_imgui.h"
#include "./js_util.h"

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

std::string get_entrypoint(const std::string& filename) {
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

/**
 * Entry-Point of project
 * @return Exit-Code
 */
int main() {
    GLFWwindow* window = glfw_init(1280, 720, "chibi-ui");
    if (!window) {
        printf("something went wrong... exiting");
        return -1;
    }

    const auto entrypoint = get_entrypoint("./entrypoint.js");
    if (entrypoint.empty()) {
        printf("entrypoint.js not found... exiting");
        return -1;
    }

    JSRuntime* runtime  = JS_NewRuntime();
    JSContext* ctx      = JS_NewContext(runtime);
    JSValue global      = JS_GetGlobalObject(ctx);

    js_imgui_init(ctx);
    JS_SetProperty(ctx, global, JS_NewAtom(ctx, "print"), JS_NewCFunction(ctx, js_print, "print", 0));

    JSValue res = JS_Eval(ctx, entrypoint.c_str(), entrypoint.length(), "entrypoint.js", JS_EVAL_TYPE_GLOBAL);
    if (!JS_IsFunction(ctx, res)) {
        printf("undefined tick!");
        return -1;
    }

    /*
     * Variables
     */
    float delta_time = 0.0f;
    float last_frame = 0.0f;
    JSValue js_delta_time = JS_NewFloat64(ctx, 0);
    JSAtom js_delta_time_atom = JS_NewAtom(ctx, "dt");
    ImVec4 clear_color = ImColor(0, 0, 0, 255);

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

        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
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