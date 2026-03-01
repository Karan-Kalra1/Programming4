#include "CacheTrashWindow.h"

#include <algorithm>
#include <chrono>
#include <numeric>
#include <memory>

// Dear ImGui
#include <imgui.h>


struct Transform
{
    float matrix[16] = {
        1,0,0,0,
        0,1,0,0,
        0,0,1,0,
        0,0,0,1
    };
};

class GameObject3D
{
public:
    Transform transform;
    int ID{};
};


struct GameObject3DAlt
{
    std::unique_ptr<Transform[]> transforms;
    std::unique_ptr<int[]> ids;

    explicit GameObject3DAlt(int count)
        : transforms(std::make_unique<Transform[]>(count))
        , ids(std::make_unique<int[]>(count))
    {
    }
};

// ------------------ Helpers ------------------
static std::vector<int> MakeSteps()
{
    std::vector<int> s;
    for (int step = 1; step <= 1024; step *= 2)
        s.push_back(step);
    return s;
}

float dae::CacheTrashWindow::AverageNoOutliers(std::vector<float>& samples)
{
    if (samples.empty()) return 0.0f;
    if (samples.size() <= 2)
    {
        float sum = std::accumulate(samples.begin(), samples.end(), 0.0f);
        return sum / static_cast<float>(samples.size());
    }

    std::sort(samples.begin(), samples.end());
    // drop min and max
    float sum = 0.0f;
    for (size_t i = 1; i + 1 < samples.size(); ++i)
        sum += samples[i];
    return sum / static_cast<float>(samples.size() - 2);
}

float dae::CacheTrashWindow::RunExercise1Once(int bufferSize, int step)
{
    // ints
    auto arr = std::make_unique<int[]>(bufferSize);

    const auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < bufferSize; i += step)
        arr[i] *= 2;
    const auto end = std::chrono::high_resolution_clock::now();

    const auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    return static_cast<float>(us);
}

float dae::CacheTrashWindow::RunExercise2Once(int bufferSize, int step)
{
    auto arr = std::make_unique<GameObject3D[]>(bufferSize);

    const auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < bufferSize; i += step)
        arr[i].ID *= 2;
    const auto end = std::chrono::high_resolution_clock::now();

    const auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    return static_cast<float>(us);
}

float dae::CacheTrashWindow::RunExercise2AltOnce(int bufferSize, int step)
{
    GameObject3DAlt data(bufferSize);

    const auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < bufferSize; i += step)
        data.ids[i] *= 2;
    const auto end = std::chrono::high_resolution_clock::now();

    const auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    return static_cast<float>(us);
}

// Combined plot using ImDrawList
void dae::CacheTrashWindow::PlotCombined(const char* label,
    const std::vector<int>& steps,
    const std::vector<float>& a,
    const std::vector<float>& b,
    const char* aName,
    const char* bName,
    float height)
{
    if (steps.empty() || a.size() != steps.size() || b.size() != steps.size())
        return;

    ImGui::TextUnformatted(label);

    ImVec2 p0 = ImGui::GetCursorScreenPos();
    ImVec2 size(ImGui::GetContentRegionAvail().x, height);
    ImVec2 p1(p0.x + size.x, p0.y + size.y);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(p0, p1, IM_COL32(20, 30, 45, 255), 6.0f);
    dl->AddRect(p0, p1, IM_COL32(80, 90, 110, 255), 6.0f);

    // Compute min/max for normalization
    float minV = std::min(*std::min_element(a.begin(), a.end()), *std::min_element(b.begin(), b.end()));
    float maxV = std::max(*std::max_element(a.begin(), a.end()), *std::max_element(b.begin(), b.end()));
    if (maxV <= minV) maxV = minV + 1.0f;

    auto toPoint = [&](int idx, float v)
        {
            float tX = (steps.size() == 1) ? 0.0f : (float)idx / (float)(steps.size() - 1);
            float tY = (v - minV) / (maxV - minV);
            // padding
            float pad = 10.0f;
            float x = p0.x + pad + tX * (size.x - 2 * pad);
            float y = p1.y - pad - tY * (size.y - 2 * pad);
            return ImVec2(x, y);
        };

    // Choose two distinct default ImGui colors
    ImU32 colA = ImGui::GetColorU32(ImGuiCol_PlotLines);
    ImU32 colB = ImGui::GetColorU32(ImGuiCol_PlotHistogram);

    // Draw polylines
    for (size_t i = 1; i < steps.size(); ++i)
    {
        dl->AddLine(toPoint((int)i - 1, a[i - 1]), toPoint((int)i, a[i]), colA, 2.0f);
        dl->AddLine(toPoint((int)i - 1, b[i - 1]), toPoint((int)i, b[i]), colB, 2.0f);
    }

    // Legend
    dl->AddText(ImVec2(p0.x + 12, p0.y + 8), colA, aName);
    dl->AddText(ImVec2(p0.x + 12, p0.y + 26), colB, bName);

    ImGui::Dummy(size); // reserve space
}

void dae::CacheTrashWindow::RunExercise1()
{
    m_steps = MakeSteps();
    m_ex1_us.assign(m_steps.size(), 0.0f);

    for (size_t si = 0; si < m_steps.size(); ++si)
    {
        const int step = m_steps[si];

        std::vector<float> samples;
        samples.reserve(m_samples);

        for (int k = 0; k < m_samples; ++k)
            samples.push_back(RunExercise1Once(m_bufferSize, step));

        m_ex1_us[si] = AverageNoOutliers(samples);
    }

    m_hasEx1 = true;
}

void dae::CacheTrashWindow::RunExercise2()
{
    m_steps = MakeSteps();
    m_ex2_us.assign(m_steps.size(), 0.0f);

    for (size_t si = 0; si < m_steps.size(); ++si)
    {
        const int step = m_steps[si];

        std::vector<float> samples;
        samples.reserve(m_samples);

        for (int k = 0; k < m_samples; ++k)
            samples.push_back(RunExercise2Once(m_bufferSize, step));

        m_ex2_us[si] = AverageNoOutliers(samples);
    }

    m_hasEx2 = true;
}

void dae::CacheTrashWindow::RunExercise2Alt()
{
    m_steps = MakeSteps();
    m_ex2_alt_us.assign(m_steps.size(), 0.0f);

    for (size_t si = 0; si < m_steps.size(); ++si)
    {
        const int step = m_steps[si];

        std::vector<float> samples;
        samples.reserve(m_samples);

        for (int k = 0; k < m_samples; ++k)
            samples.push_back(RunExercise2AltOnce(m_bufferSize, step));

        m_ex2_alt_us[si] = AverageNoOutliers(samples);
    }

    m_hasEx2Alt = true;
}



void dae::CacheTrashWindow::DrawImGui()
{
    //Exercise 1
    ImGui::Begin("Exercise 1");

    ImGui::InputInt("bufferSize", &m_bufferSize);
    m_bufferSize = std::max(1024, m_bufferSize);

    ImGui::InputInt("# samples", &m_samples);
    m_samples = std::clamp(m_samples, 3, 5000);

    if (ImGui::Button("Thrash the cache"))
        RunExercise1();

    if (m_hasEx1)
    {
        ImGui::SeparatorText("Thrash the cache");
        ImGui::PlotLines("ints (us)", m_ex1_us.data(), (int)m_ex1_us.size(), 0, nullptr, 0.0f, FLT_MAX, ImVec2(0, 160));
    }

    ImGui::End();

    // Exercise 2
    ImGui::Begin("Exercise 2");

    ImGui::InputInt("# samples##2", &m_samples);
    m_samples = std::clamp(m_samples, 3, 5000);

    if (ImGui::Button("Thrash the cache with GameObject3D"))
        RunExercise2();

    if (ImGui::Button("Thrash the cache with GameObject3DAlt"))
        RunExercise2Alt();

    if (m_hasEx2)
    {
        ImGui::SeparatorText("Thrash the cache with GameObject3D");
        ImGui::PlotLines("GameObject3D (us)", m_ex2_us.data(), (int)m_ex2_us.size(), 0, nullptr, 0.0f, FLT_MAX, ImVec2(0, 140));
    }

    if (m_hasEx2Alt)
    {
        ImGui::SeparatorText("Thrash the cache with GameObject3DAlt");
        ImGui::PlotLines("GameObject3DAlt (us)", m_ex2_alt_us.data(), (int)m_ex2_alt_us.size(), 0, nullptr, 0.0f, FLT_MAX, ImVec2(0, 140));
    }

    // Combined only if both exist
    if (m_hasEx2 && m_hasEx2Alt)
    {
        ImGui::SeparatorText("Combined");
        PlotCombined("Combined:", m_steps, m_ex2_us, m_ex2_alt_us, "GameObject3D", "GameObject3DAlt", 160.0f);
    }
    else
    {
        ImGui::TextDisabled("Run both GameObject3D and GameObject3DAlt to see the combined plot.");
    }

    ImGui::End();
}