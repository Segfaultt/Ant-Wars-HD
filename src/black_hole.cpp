#include <math.h>

black_hole::black_hole(int x_coord, int y_coord, std::vector<ant *> target)
{
	x = x_coord;
	y = y_coord;
	
	targets = target;
	sprite.load_texture((std::string)"res/" + (std::string)RES_PACK + (std::string)"/black_hole.png");
}

void black_hole::render()
{
	sprite.render(x, y, angle);
	angle += 10;
}

void black_hole::pull_ants()
{
	double distance;
	for (ant *ant_iterator : targets) {
		distance = sqrt(pow(x - ant_iterator->get_x(), 2) + pow(y - ant_iterator->get_y(), 2));
		ant_iterator->apply_force(distance/(x - ant_iterator->get_x()), distance/(y - ant_iterator->get_y()));
		std::cout << (distance/(x - ant_iterator->get_x())) << '\t' <<  (distance/(y - ant_iterator->get_y())) << '\n';
	}
}
