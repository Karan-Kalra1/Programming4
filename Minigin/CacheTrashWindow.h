#pragma once
#include <vector>
#include <cstdint>

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
}