#pragma once
#include "SoundSystem.h"

namespace digger
{
	enum GameSound : dae::SoundId
	{
		DiamondPickUp = 1,

		BackgroundMusic = 2,

		MoneyBagWiggle = 3,
		MoneyBagFalling = 4,

		PlayerDeathSfx = 5,
		PlayerDeathMusic = 6,

		BulletTravel = 7,
		BulletHit = 8
	};
}