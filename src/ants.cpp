#include <math.h>
#include "black_hole.h"

#define PI_OVER_180 0.017453293
#define PYTHAG(a, b) sqrt(pow(a, 2) + pow(a, 2))
#define STAMINA_REGEN 0.3
#define ANT_REPEL_FORCE 100

//=====ANT=====
ant::ant(ant_type type_, int starting_x, int starting_y)
{
	type = type_;
	nip_out_timer = 0;
	laser_on = 0;
	guitar = 0;
	alive = true;
	mass = 1;
	velocity[0] = 0;
	velocity[1] = 0;
	speed = 8;
	turn_speed = 5;
	health = 100;
	stamina = 100;
	x = starting_x;
	y = starting_y;
	nip_texture.load_texture((std::string)"res/" + (std::string)RES_PACK + (std::string)"/nip.png");
	guitar_texture.load_texture((std::string)"res/" + (std::string)RES_PACK + (std::string)"/guitar.png");
	tesla_bolt = NULL;
	tesla_target = NULL;

	bar_health = new bar(90, 10);
	bar_stamina = new bar(60, 7);

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
		case LUCA:
			sprite.load_texture((std::string)"res/" + (std::string)RES_PACK + (std::string)"/luca.png");
			break;
		case CSS_BAD:
			sprite.load_texture((std::string)"res/" + (std::string)RES_PACK + (std::string)"/jeff.png");
			break;
		case HIPSTER:
			sprite.load_texture((std::string)"res/" + (std::string)RES_PACK + (std::string)"/hipster.png");
			break;
		case BOT:
			sprite.load_texture((std::string)"res/" + (std::string)RES_PACK + (std::string)"/bot.png");
			speed *= 0.5;
			turn_speed *= 0.8;
			health *= 0.7;
			break;

	};
}

void ant::set_other_ants(std::vector<ant *> other_ants_)
{
	other_ants = other_ants_;
}

ant::~ant()
{
	for (black_hole *i : holes)
		i->~black_hole();
}

void ant::move(direction dir)
{
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

	bar_health->render(x + 50, y - 32, health);
	bar_stamina->render(x + 80, y - 20, stamina);

	if (nip_out_timer > 0) {
		nip_texture.render(45 * cos(angle * PI_OVER_180) + x + 25, -45 * sin(angle * PI_OVER_180) + y + 25, bearing);
		nip_out_timer--;
	}

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

if (type == YA_BOY && tesla_bolt != NULL && tesla_target != NULL) {
		tesla_bolt->tick(tesla_target->get_x() + 50, tesla_target->get_y() + 50);
		if (!tesla_bolt->is_alive()) {
			//apply attraction
			double magnitude = PYTHAG(x - tesla_target->get_x(), y - tesla_target->get_y());
			double x_component_unit_vector = (x - tesla_target->get_x()) / magnitude;
			double y_component_unit_vector = (y - tesla_target->get_y()) / magnitude;
			const double pull_force = 3;

			tesla_target->apply_force(pull_force * x_component_unit_vector, pull_force * y_component_unit_vector);
			apply_force(-pull_force * x_component_unit_vector, -pull_force * y_component_unit_vector);

			//damage
			tesla_target->damage(15);

			//clean up
			delete tesla_bolt;
			tesla_bolt = NULL;
			tesla_target = NULL;

		}
	}

	if (laser_on > 0) {
		double x_gradient = cos(angle * PI_OVER_180);
		double y_gradient = sin(angle * PI_OVER_180);
		thickLineRGBA(renderer, 45 * x_gradient + x + 50, -45 * y_gradient + y + 50, (SCREEN_WIDTH + SCREEN_HEIGHT) * x_gradient + x + 50, -1 *(SCREEN_WIDTH + SCREEN_HEIGHT) * y_gradient + y + 50, 9, 0x97, 0x00, 0x00, 0xff);

		//check if hit
		for (ant *i : other_ants) {
			double y_difference, x_difference;
			if ((bearing - 50 > 180 && x - i->get_x() < 0) || (bearing + 50 <= 180 && x - i->get_x() > 0)) {
				y_difference = SCREEN_HEIGHT;
				x_difference = SCREEN_WIDTH;
			} else {
				y_difference = abs(((i->get_x() - x) * tan(angle * PI_OVER_180) - y + 50) + i->get_y() - 50);
				x_difference = abs(((i->get_y() - y) / tan(angle * PI_OVER_180) - x + 50) + i->get_x() - 50);
			}
			double smallest_difference;
			if (y_difference > x_difference)
				smallest_difference = x_difference;
			else 
				smallest_difference = y_difference;

			if (smallest_difference < 50)
				i->damage(5);
			laser_on--;
		}
	}

	sprite.render(x, y, bearing);

	if (type == HIPSTER && guitar > 0 && stamina > 0 && health < 100) {
		guitar--;
		guitar_texture.render(45 * cos(angle * PI_OVER_180) + x + 25, -45 * sin(angle * PI_OVER_180) + y + 25, bearing);
		damage(-0.5);
		stamina -= 2;
	}
}

void ant::apply_force(double x_component, double y_component)
{
	velocity[0] += x_component/mass;
	velocity[1] += y_component/mass;
}

void ant::apply_physics()
{
	//apply velocity
	x += velocity[0];
	y += velocity[1];

	//friction/air resistance
	velocity[0] *= 0.9;
	velocity[1] *= 0.9;
	if (abs(velocity[0]) < 0.0001)
		velocity[0] = 0;
	if (abs(velocity[1]) < 0.0001)
		velocity[1] = 0; //ants repel
	double distance;
	for (ant *i : other_ants) {
		distance = sqrt(pow(i->get_x() - x, 2) + pow(i->get_y() - y, 2));
		if (distance < 90) {
			velocity[0] -= ANT_REPEL_FORCE * (i->get_x() - x)/pow(distance, 3);
			velocity[1] -= ANT_REPEL_FORCE * (i->get_y() - y)/pow(distance, 3);
		}
	}

	if (stamina <= 100 - STAMINA_REGEN) {//stamina regen cap
		stamina += STAMINA_REGEN;
	}

	//black hole child physics
	double x_component, y_component;
	for (black_hole *i : holes) {
		//pull other ants
		for (ant *each_ant : other_ants) {
			x_component = 0;//force from black hole passed by reference
			y_component = 0;
			i->pull_ants(each_ant->get_x(), each_ant->get_y(), each_ant->get_mass(), x_component, y_component);
			each_ant->apply_force(x_component, y_component);
		}

		//pull self
		x_component = 0;//force from black hole passed by reference
		y_component = 0;
		i->pull_ants(x, y, mass, x_component, y_component);
		apply_force(x_component/2, y_component/2);
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
	if (type == YA_BOY)
		damage *= 1.5;
	health -= damage;
	mass -= damage/150;
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
	if (x + 50 > SCREEN_WIDTH | x + 50< 0 | y + 50 > SCREEN_HEIGHT | y + 50 < 0) {
		damage(0.5);
	}
}

void ant::ability()
{
	switch (type) {
		case LUCA:
			if (stamina > 80) {
				black_hole *hole = new black_hole(x, y, angle);
				holes.push_back(hole);
				stamina -= 80;
			}
			break;

		case YA_BOY:
			tesla();
			break;

		case CSS_BAD:
			if (stamina > 60 && laser_on <= 0) {
				laser_on = TICKS_PER_FRAME/2;
				apply_force(-20 * cos(angle * PI_OVER_180), 20 * sin(angle * PI_OVER_180));
				stamina -= 60;
			}
			break;

		case HIPSTER:
			if (stamina > 0) {
				guitar = TICKS_PER_FRAME;
			}
			break;
	}
}

void ant::nip()
{
	const double stamina_take = 30;
	if (stamina >= stamina_take && nip_out_timer == 0) {
		stamina -= stamina_take;
		int nip_pos[2] = {45 * cos(angle * PI_OVER_180) + x + 25, -45 * sin(angle * PI_OVER_180) + y + 25};
		double distance = 0;
		nip_out_timer = TICKS_PER_FRAME/2;//0.5 seconds
		for (ant *i : other_ants) {
			distance = sqrt(pow(nip_pos[0] - i->get_x() - 25, 2)+pow(nip_pos[1] - i->get_y() - 25, 2));
			if (distance < 50) {
				i->damage(10);
			}
		}
	}
}

void ant::tesla()
{
	const int cost_coefficient = 10;
	for (ant *i : other_ants) {
		double distance = sqrt(pow(x - i->get_x(), 2) + pow(y - i->get_y(), 2));
		if (stamina >= distance/cost_coefficient + 20 && tesla_bolt == NULL) {
			stamina -= distance/cost_coefficient + 20;
			tesla_target = i;
			delete tesla_bolt;
			tesla_bolt = new electric_bolt(x + 50, y + 50);
		}
	}
}

double ant::get_angle()
{
	return angle;
}

double ant::get_health()
{
	return health;
}
