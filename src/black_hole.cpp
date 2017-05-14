#include <math.h>

black_hole::black_hole(int x_coord, int y_coord, double angle_)
{
	x = x_coord;
	y = y_coord;

	const int launch_force = 30;
	#define PI_OVER_180 0.017453293
	velocity[0] = launch_force * cos(angle_ * PI_OVER_180);
	velocity[1] = -launch_force * sin(angle_ * PI_OVER_180);
	
	sprite.load_texture((std::string)"res/" + (std::string)RES_PACK + (std::string)"/black_hole.png");
}

void black_hole::render()
{
	sprite.render(x, y, angle);
	angle += 10;
}

void black_hole::pull_ants(int target_x, int target_y, double target_mass, double &x_component, double &y_component)
{
	const int G = 200;
	double distance;
	distance = sqrt(pow(x - target_x, 2) + pow(y - target_y, 2));
	x_component = (G * target_mass * (x - target_x))/pow(distance, 2);
	y_component = (G * target_mass * (y - target_y))/pow(distance, 2);

	x += velocity[0];
	y += velocity[1];

	velocity[0] *= 0.95;
	velocity[1] *= 0.95;
	if (abs(velocity[0]) < 0.0001)
		velocity[0] = 0;
	if (abs(velocity[1]) < 0.0001)
		velocity[1] = 0;
}

bool black_hole::is_alive()
{
	return angle < 1500;
}
