#pragma once
#include <vector>

class black_hole {
	public:
	black_hole(int x_coord, int y_coord);
	void pull_ants(int target_x, int target_y, double target_mass, double &x_component, double &y_component);
	void render();
	bool is_alive();

	private:
	texture_wrapper sprite;
	int x, y;
	double angle;
};

#include "black_hole.cpp"
