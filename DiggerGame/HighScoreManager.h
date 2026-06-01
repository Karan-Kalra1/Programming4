#pragma once
#include <string>
#include <vector>
#include <filesystem>

namespace digger
{
	struct HighScoreEntry
	{
		std::string name{};
		int score{};
	};

	class HighScoreManager final
	{
	public:
		explicit HighScoreManager(std::filesystem::path filePath);

		void Load();
		void Save() const;

		void AddScore(const std::string& name, int score);

		const std::vector<HighScoreEntry>& GetScores() const { return m_Scores; }

	private:
		std::filesystem::path m_FilePath{};
		std::vector<HighScoreEntry> m_Scores{};

		static constexpr int MaxScores = 10;
	};
}