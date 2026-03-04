#pragma once
#include <vector>
#include <cstdint>
#include <functional>
#include <chrono>

namespace dae
{
    class CacheTrashWindow final
    {
    public:
        void DrawImGui(); 

    private:
        // UI state
        int m_samples{ 100 };
        int m_bufferSize{ 200000 }; 
     
        template <typename Fn>
        void RunGeneric(std::vector<float>& out, Fn&& runOnce);

        std::vector<int> m_steps;

        // Timings in microseconds (averaged per step)
        std::vector<float> m_ex1_us;        // ints
        std::vector<float> m_ex2_us;        // GameObject3D
        std::vector<float> m_ex2_alt_us;    // GameObject3DAlt

        void RunExercise1();
        void RunExercise2();
        void RunExercise2Alt();

        bool m_hasEx1{ false };
        bool m_hasEx2{ false };
        bool m_hasEx2Alt{ false };

        static float RunExercise1Once(int bufferSize, int step);
        static float RunExercise2Once(int bufferSize, int step);
        static float RunExercise2AltOnce(int bufferSize, int step);
        static std::vector<int> MakeSteps()
        {
            std::vector<int> s;
            for (int step = 1; step <= 1024; step *= 2)
                s.push_back(step);
            return s;
        }

        static float AverageNoOutliers(std::vector<float>& samples);

        // Combined plot (2+ series) using ImDrawList
        static void PlotCombined(const char* label,
            const std::vector<int>& steps,
            const std::vector<float>& a,
            const std::vector<float>& b,
            const char* aName,
            const char* bName,
            float height = 150.0f);
    };


    template <typename Fn>
    void dae::CacheTrashWindow::RunGeneric(std::vector<float>& out, Fn&& runOnce)
    {
        m_steps = MakeSteps();
        out.assign(m_steps.size(), 0.0f);

        for (size_t si = 0; si < m_steps.size(); ++si)
        {
            const int step = m_steps[si];

            std::vector<float> samples;
            samples.reserve(m_samples);

            for (int k = 0; k < m_samples; ++k)
                samples.push_back(runOnce(m_bufferSize, step));

            out[si] = AverageNoOutliers(samples);
        }
    }

    template<typename T, typename Accessor>
    float RunBenchmark(int bufferSize, int step, Accessor access)
    {
        auto arr = std::make_unique<T[]>(bufferSize);

        const auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < bufferSize; i += step)
            access(arr[i]) *= 2;

        const auto end = std::chrono::high_resolution_clock::now();

        const auto us =
            std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        return static_cast<float>(us);
    }
}