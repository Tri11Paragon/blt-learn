/*
 *  <Short Description>
 *  Copyright (C) 2025  Brett Terpstra
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <blt/gfx/window.h>
#include "blt/gfx/renderer/resource_manager.h"
#include "blt/gfx/renderer/batch_2d_renderer.h"
#include "blt/gfx/renderer/camera.h"
#include <imgui.h>
#include <filesystem>
#include <fstream>

#include "blt/config.h"

#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#include <cerrno>
#include <cstring>
#endif

const char* runtimes[] = {RUNTIMES};
const char* sources[] = {SOURCES};

blt::gfx::matrix_state_manager global_matrices;
blt::gfx::resource_manager resources;
blt::gfx::batch_renderer_2d renderer_2d(resources, global_matrices);
blt::gfx::first_person_camera camera;

void invoke_runtime(const std::filesystem::path& exec)
{
    std::ofstream output{"runtime.last"};
    output << exec.string();
    output.flush();
    output.close();
#ifdef _WIN32
    // On Windows, _wexecv replaces the current process image on success.
    std::wstring wexe = exec.wstring();
    // argv[0] is conventionally the program path/name
    const wchar_t* wargv[] = {wexe.c_str(), nullptr};
    _wexecv(wexe.c_str(), const_cast<wchar_t* const*>(wargv));

    // If we got here, _wexecv failed.
    // You may want to show a message box or log appropriately.
    // fprintf is fine if you have a console attached.
    // Note: errno is set on failure.
    fprintf(stderr, "Failed to exec '%ls' (errno=%d)\n", wexe.c_str(), errno);
#else
    // On POSIX, execv replaces the current process image on success.
    std::string exe = exec.string();
    char* argv[] = {const_cast<char*>(exe.c_str()), nullptr};
    execv(exe.c_str(), argv);

    // If we got here, execv failed.
    fprintf(stderr, "Failed to exec '%s': %s\n", exe.c_str(), std::strerror(errno));
#endif
}


void init(const blt::gfx::window_data&)
{
    using namespace blt::gfx;


    global_matrices.create_internals();
    resources.load_resources();
    renderer_2d.create();
}

void update(const blt::gfx::window_data& data)
{
    global_matrices.update_perspectives(data.width, data.height, 90, 0.1, 2000);

    camera.update();
    camera.update_view(global_matrices);
    global_matrices.update();

    const std::filesystem::path base_dir{CMAKE_BINARY_DIR};
    std::vector<std::pair<std::filesystem::path, std::filesystem::path>> sorted_dirs;
    for (const auto& [source, runtime] : blt::zip(sources, runtimes))
    {
        std::filesystem::path src{source};
        std::filesystem::path exe{runtime};
        sorted_dirs.emplace_back(src, exe);
    }
    std::sort(sorted_dirs.begin(), sorted_dirs.end(), [](const auto& a, const auto& b)
    {
        static auto get_time = [](const std::filesystem::path& p) {
            std::error_code ec;
            auto t = std::filesystem::last_write_time(p, ec);
            if (ec) {
                // If stat fails, push these to the end by using the minimum time
                return std::filesystem::file_time_type::min();
            }
            return t;
        };

        return get_time(a.first) > get_time(b.first);
    });
    // ImGui::ShowDemoWindow();
    ImGui::SetNextWindowSize({static_cast<float>(data.width), static_cast<float>(data.height)}, ImGuiCond_Always);
    ImGui::SetNextWindowPos({0, 0}, ImGuiCond_Always);
    ImGui::Begin("##Display", nullptr,
                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration);
    ImVec2 outer_size(0, 0);
    ImVec2 btn_size(200.0f, 48.0f);
    const ImGuiStyle& style = ImGui::GetStyle();
    float avail_w = ImGui::GetContentRegionAvail().x;
    float spacing = style.ItemSpacing.x;
    int columns = (int)((avail_w + spacing) / (btn_size.x + spacing));
    if (columns < 1) columns = 1;
    ImGuiTableFlags flags =
            ImGuiTableFlags_SizingFixedFit |
            ImGuiTableFlags_PadOuterX |
            ImGuiTableFlags_NoHostExtendX |
            ImGuiTableFlags_ScrollY;

    if (ImGui::BeginTable("ButtonGrid", columns, flags, outer_size))
    {
        ImGui::TableSetupScrollFreeze(0, 0);
        for (const auto& [source, runtime] : sorted_dirs)
        {
            ImGui::TableNextColumn();
            float cell_w = ImGui::GetColumnWidth();
            float x_offset = (cell_w - btn_size.x) * 0.5f;
            if (x_offset > 0)
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + x_offset);

            std::string display_name = std::filesystem::relative(runtime, base_dir).string();
            if (ImGui::Button(display_name.c_str(), btn_size))
                invoke_runtime(runtime);
        }
        ImGui::EndTable();
    }
    ImGui::End();

    renderer_2d.render(data.width, data.height);
}

void destroy(const blt::gfx::window_data&)
{
    global_matrices.cleanup();
    resources.cleanup();
    renderer_2d.cleanup();
    blt::gfx::cleanup();
}

int main()
{
#ifdef BLT_AUTORELOAD_RUNTIME
    if (std::filesystem::exists("runtime.last"))
    {
        std::ifstream input("runtime.last");
        std::string exec;
        input >> exec;
        invoke_runtime(exec);
    }
#endif
    glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
    blt::gfx::init(blt::gfx::window_data{"Select Runtime", init, update, destroy}.setSyncInterval(1));

}
