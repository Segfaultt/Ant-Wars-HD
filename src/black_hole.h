#include <vector>
class black_hole {
	public:
	black_hole(int x_coord, int y_coord, std::vector<ant *> target);
	void pull_ants();
	void render();

	private:
	texture_wrapper sprite;
	int x, y;
	double angle;
	std::vector<ant *> targets;
};

#include "black_hole.cpp"
