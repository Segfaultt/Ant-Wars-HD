#include <vector>

class grease_trap {
	public:
	grease_trap(int x_, int y_);
	bool tick(int target_x, int target_y);

	private:
	int x, y, ticks_until_death;
	bool alive;
	texture_wrapper sprite;
};

#include "greasy.cpp"
