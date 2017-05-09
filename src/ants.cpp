#include <math.h>

//=====ANT=====
ant::ant(ant_type type, int starting_x, int starting_y)
{
	mass = 100;
	speed = 10;
	turn_speed = 5;
	health = 100;
	stamina = 100;
	x = starting_x;
	y = starting_y;
	if (x > SCREEN_WIDTH/2) {
		bearing = 270;
		angle = 180;
	} else {
		bearing = 90;
		angle = 0;
	}

	switch (type) {
		case YA_BOY:
		sprite.load_texture((std::string)"res/" + (std::string)RES_PACK + (std::string)"/ya_boy.png");
		break;
	};
}

void ant::move(direction dir)
{
	#define PI_OVER_180 0.017453293
	switch (dir) {
		case FORWARDS:
			x += speed * cos(angle * PI_OVER_180);
			y -= speed * sin(angle * PI_OVER_180);
			//std::cout << speed * cos(angle * PI_OVER_180) << '\t' << angle << '\t' << speed * sin(angle * PI_OVER_180) << '\t' << x << '\t' << y << '\n';
			break;

		case BACKWARDS:
			x -= speed * cos(angle * PI_OVER_180);
			y += speed * sin(angle * PI_OVER_180);
			break;


		case LEFT:
			bearing -= turn_speed;
			if (bearing < 0)
				bearing += 360;
			angle = 450 - bearing;
			if (angle > 360)
				angle -= 360;
			break;

		case RIGHT:
			bearing += turn_speed;
			if (bearing > 360)
				bearing -= 360;
			angle = 450 - bearing;
			break;
	}
}

void ant::render()
{
	sprite.render(x, y, bearing);
}
