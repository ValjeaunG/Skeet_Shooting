#pragma once

struct Target
{
	float x, y, xvel, yvel, size;
	int r, g, b, tier, health, timer;
	bool destroyed, hit, is_offscreen, missed;
};
