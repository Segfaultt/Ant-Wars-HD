#include <math.h>

//=====ANT=====
ant::ant(ant_type type, int starting_x, int starting_y)
{
	mass = 100;
	speed = 4;
	turn_speed = 5;
	health = 100;
	stamina = 100;
	x = starting_x;
	y = starting_y;
	if (x > SCREEN_WIDTH/2) {
		angle = 270;
	} else {
		angle = 90;
	}

	switch (type) {
		case YA_BOY:
		sprite.load_texture((std::string)"res/" + (std::string)RES_PACK + (std::string)"/ya_boy.png");
		break;
	};
}

void ant::move(direction dir)
{
	switch (dir) {
		case FORWARDS:
			x += speed * acos(dir);
			y += speed * asin(dir);
			break;

		case LEFT:
			angle -= turn_speed;
			break;

		case RIGHT:
			angle += turn_speed;
			break;
	}
}

void ant::render()
{
	sprite.render(x, y);
}
