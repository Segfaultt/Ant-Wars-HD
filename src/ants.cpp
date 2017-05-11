#include <math.h>
#include "black_hole.h"

//=====ANT=====
ant::ant(ant_type type_, int starting_x, int starting_y, std::vector<ant *> other_ants_)
{
	type = type_;
	alive = true;
	mass = 100;
	velocity[0] = 0;
	velocity[1] = 0;
	speed = 8;
	turn_speed = 5;
	health = 100;
	stamina = 100;
	x = starting_x;
	y = starting_y;
	other_ants = other_ants_;
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
		case LUCA:
		sprite.load_texture((std::string)"res/" + (std::string)RES_PACK + (std::string)"/luca.png");
		break;
	};
}

ant::~ant()
{
	for (black_hole *i : holes)
		i->~black_hole();
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
	if (type == LUCA) {
		for (black_hole *i : holes) {
			int pos = 0;
			i->render();
			if (!i->is_alive()) {
				i->~black_hole();
				holes.erase(holes.begin() + pos);
			}
			pos++;
		}
	}
}

void ant::apply_force(double x_component, double y_component)
{
	velocity[0] += x_component;
	velocity[1] += y_component;
}

void ant::apply_physics()
{
	x += velocity[0];
	y += velocity[1];

	velocity[0] *= 0.9;
	velocity[1] *= 0.9;

	stamina += 1;

	double x_component, y_component;
	for (black_hole *i : holes)
		for (ant *each_ant : other_ants) {
			x_component = 0;
			y_component = 0;
			i->pull_ants(each_ant->get_x(), each_ant->get_y(), each_ant->get_mass(), x_component, y_component);
			each_ant->apply_force(x_component, y_component);
		}
}

int ant::get_x()
{
	return x;
}

int ant::get_y()
{
	return y;
}

double ant::get_mass()
{
	return mass;
}

void ant::damage(double damage)
{
	health -= damage;
	if (health < 0) {
		alive = false;
	}
}

bool ant::is_alive()
{
	return alive;
}

void ant::check_edge()
{
	if (x > SCREEN_WIDTH | x < 0 | y > SCREEN_HEIGHT | y < 0) {
		damage(100);
	}
}

void ant::ability()
{
	switch (type) {
		case LUCA:
		if (stamina > 50) {
			black_hole *hole = new black_hole(x, y);
			holes.push_back(hole);
			stamina -= 50;
		}
	}
}
