#include "Performance.h"

#include ".class/CameraController/CameraController.h"

Performance::Performance() {}

auto Performance::render() -> void {
    static constexpr int SAMPLE_COUNT = 1000;

    static double avg_fps;
    static double frame_ms;
    static float fps;
    static float low_dt_ms;
    static float render_time_;
    static float update_time_;

    static std::vector samples(SAMPLE_COUNT, 0.0);
    static int write_idx = 0;
    static int filled = 0;

    double dt = ImGui::GetIO().DeltaTime;
    if (dt <= 0.0) {
        dt = 0.0;
    }

    samples[write_idx] = dt;
    write_idx = (write_idx + 1) % SAMPLE_COUNT;
    if (filled < SAMPLE_COUNT) {
        ++filled;
    }

    char timestr[64] = "??:??:??";
    const std::time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm local_tm;
    if (localtime_s(&local_tm, &t) == 0) {
        std::strftime(timestr, sizeof(timestr), "%H:%M:%S", &local_tm);
    }

    const ImVec2 screen = ImGui::GetIO().DisplaySize;

    ImGui::SetNextWindowPos(ImVec2(screen.x * 0.5f, 0.0f), ImGuiCond_Always, ImVec2(0.5f, 0.0f));
    ImGui::SetNextWindowBgAlpha(0.5f);

    constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings;

    ImGui::Begin("##topbar_only", nullptr, flags);

    ImGui::AlignTextToFramePadding();
    ImGui::Text(reinterpret_cast<const char*>(u8"%s FPS: %.1f 平均: %.1f 时间: %.3f ms, 0.1%% FPS: %.1f 渲染: %.3f ms 更新: %.3f ms 用时: %.3f ms"), timestr, fps, avg_fps, frame_ms, low_dt_ms, render_time_, update_time_, render_time_ + update_time_);

    ImGui::End();

    using clock = std::chrono::high_resolution_clock;
    static auto last_update_tp = clock::now();
    const auto now = clock::now();
    if (now - last_update_tp < std::chrono::seconds(1)) {
        return;
    }
    last_update_tp = now;

    if (filled > 0) {
        std::vector tmp(samples.begin(), samples.begin() + filled);
        std::ranges::sort(tmp);
        constexpr double p = 0.999;
        const int idx = std::clamp(static_cast<int>(std::ceil(p * filled)) - 1, 0, filled - 1);
        low_dt_ms = tmp[idx] * 1000.0;
    }

    fps = ImGui::GetIO().Framerate;
    frame_ms = dt * 1000.0;

    double avg_dt = 0.0;
    for (int i = 0; i < filled; ++i) {
        avg_dt += samples[i];
    }
    avg_fps = filled > 0 ? filled / avg_dt : 0.0;

    std::shared_lock lock(rw_lock);
    std::shared_lock lock2(CameraController::rw_lock);
    update_time_ = CameraController::update_time;
    render_time_ = render_time;
}

auto Performance::update() -> void {}
