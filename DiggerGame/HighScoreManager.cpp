#include "HighScoreManager.h"

#include <fstream>
#include <algorithm>

digger::HighScoreManager::HighScoreManager(std::filesystem::path filePath)
	: m_FilePath(std::move(filePath))
{
}

void digger::HighScoreManager::Load()
{
	m_Scores.clear();

	std::ifstream file{ m_FilePath };
	if (!file)
		return;

	std::string name{};
	int score{};

	while (file >> name >> score)
	{
		if (name.size() > 3)
			name = name.substr(0, 3);

		m_Scores.push_back({ name, score });
	}

	std::sort(m_Scores.begin(), m_Scores.end(),
		[](const HighScoreEntry& a, const HighScoreEntry& b)
		{
			return a.score > b.score;
		});

	if (m_Scores.size() > MaxScores)
		m_Scores.resize(MaxScores);
}

void digger::HighScoreManager::Save() const
{
	std::ofstream file{ m_FilePath, std::ios::trunc };

	for (const auto& entry : m_Scores)
		file << entry.name << ' ' << entry.score << '\n';
}

void digger::HighScoreManager::AddScore(const std::string& name, int score)
{
	std::string fixedName = name;

	if (fixedName.empty())
		fixedName = "AAA";

	while (fixedName.size() < 3)
		fixedName += 'A';

	if (fixedName.size() > 3)
		fixedName = fixedName.substr(0, 3);

	m_Scores.push_back({ fixedName, score });

	std::sort(m_Scores.begin(), m_Scores.end(),
		[](const HighScoreEntry& a, const HighScoreEntry& b)
		{
			return a.score > b.score;
		});

	if (m_Scores.size() > MaxScores)
		m_Scores.resize(MaxScores);
}