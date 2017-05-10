#include <math.h>

black_hole::black_hole(int x_coord, int y_coord)
{
	x = x_coord;
	y = y_coord;
	
	sprite.load_texture((std::string)"res/" + (std::string)RES_PACK + (std::string)"/black_hole.png");
}

void black_hole::render()
{
	sprite.render(x, y, angle);
	angle += 10;
}

void black_hole::pull_ants(int target_x, int target_y, double target_mass, double &x_component, double &y_component)
{
	const double G = 100;
	double distance;
	distance = sqrt(pow(x - target_x, 2) + pow(y - target_y, 2));
	x_component = (G * target_mass * (x - target_x))/pow(distance, 3);
	y_component = (G * target_mass * (y - target_y))/pow(distance, 3);
}
