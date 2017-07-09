#include <math.h>
#include "black_hole.h"

#define PI_OVER_180 0.017453293
#define ONE_EIGHTY_OVER_PI 57.29578
#define PYTHAG(a, b) sqrt(pow(a, 2) + pow(b, 2))
#define ANT_REPEL_FORCE 1

//=====ANT=====
ant::ant(ant_type type_, int starting_x, int starting_y)
{
	x = starting_x;
	y = starting_y;
	type = type_;
	bar_health = NULL;
	bar_stamina = NULL;

	reset();

	//ai stuff
	state = AGGRESSIVE;
	srand(seed++);
	inteligence = rand()%15 + 1;
	srand(seed++);
	right_bias = rand()%24-12;
	srand(seed++);


	if (x > SCREEN_WIDTH/2) {
		bearing = 270;
		angle = 180;
	} else {
		bearing = 90;
		angle = 0;
	}
}

ant::~ant()
{
	for (auto *i : holes)
		delete i;
	for (auto *i : grease)
		delete i;
	for (auto *i : child)
		delete i;
	if (tesla_bolt != NULL) {
		delete tesla_bolt;
		tesla_bolt = NULL;
	}
	if (bar_health != NULL) {
		delete bar_health;
		bar_health = NULL;
	}
	if (bar_stamina != NULL) {
		delete bar_stamina;
		bar_stamina = NULL;
	}
}

void ant::set_other_ants(std::vector<ant *> other_ants_)
{
	other_ants = other_ants_;
}

void ant::move(direction dir)
{
	switch (dir) {
		case FORWARDS:
			if (guitar > 0) {
				x += grease_effect * speed * 0.5 * cos(angle * PI_OVER_180);
				y -= grease_effect * speed * 0.5 * sin(angle * PI_OVER_180);
			} else {
				x += grease_effect * speed * cos(angle * PI_OVER_180);
				y -= grease_effect * speed * sin(angle * PI_OVER_180);
			}
			break;

		case BACKWARDS:
			if (guitar > 0) {
				x -= grease_effect * speed * 0.5 * cos(angle * PI_OVER_180);
				y += grease_effect * speed * 0.5 * sin(angle * PI_OVER_180);
			} else {
				x -= grease_effect * speed * cos(angle * PI_OVER_180);
				y += grease_effect * speed * sin(angle * PI_OVER_180);
			}
			break;


		case LEFT:
			if (type == MATT && angular_momentum < 10) {
				angular_momentum += 1.5;
			} else {
				bearing -= turn_speed;
				if (bearing < 0)
					bearing += 360;
				angle = 450 - bearing;
				if (angle >= 360)
					angle -= 360;
			}
			break;

		case RIGHT:
			if (type == MATT && angular_momentum > -10) {
				angular_momentum -= 1.5;
			} else {
				bearing += turn_speed;
				if (bearing >= 360)
					bearing -= 360;
				angle = 450 - bearing;
				if (angle >= 360)
					angle -= 360;
			}
			break;
	}
}

void ant::render()
{

	bar_health->render(x + 50, y - 32, health);
	bar_stamina->render(x + 80, y - 20, stamina);

	if (flip_timer > 0)
		flip_timer--;
	if (nip_out_timer > 0) {
		nip_texture.render(45 * cos(angle * PI_OVER_180) + x + 25, -45 * sin(angle * PI_OVER_180) + y + 25, bearing);
	}
	if (nip_out_timer >= -TICKS_PER_FRAME/2)
		nip_out_timer--;

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
			std::vector<int> p = {i->get_x() - x, y - i->get_y()};

			double lambda = p[0]*cos(angle * PI_OVER_180) + p[1]*sin(angle * PI_OVER_180);
			if (lambda >= 0 && 2500 >= pow(p[0], 2) + pow(p[1], 2) - pow(lambda, 2)) {
				i->damage(5);

				//push targets
				double magnitude = PYTHAG(x - i->get_x(), y - i->get_y());
				double x_component_unit_vector = (x - i->get_x()) / magnitude;
				double y_component_unit_vector = (y - i->get_y()) / magnitude;
				const double push_force = -3;
				i->apply_force(push_force * x_component_unit_vector, push_force * y_component_unit_vector);
			}
			laser_on--;
		}
	}

	if (type == WEEB && tenticles_out > 0) {
		tenticle_texture.render(80 * cos(angle * PI_OVER_180) + x, -80 * sin(angle * PI_OVER_180) + y, bearing);
		double angle_to_target, angle_difference, y_component, x_component;
		for (ant *i : other_ants) {
			x_component = x - i->get_x();
			y_component = -1 * (y - i->get_y());
			angle_to_target = atan(y_component/x_component) * ONE_EIGHTY_OVER_PI;	
			if (x_component < 0)
				angle_to_target += 180;
			if (x_component > 0 && y_component < 0)
				angle_to_target += 360;

			angle_difference = angle - angle_to_target + 180;
			if (angle_difference > 180)
				angle_difference = angle_difference - 360;

			if (abs(angle_difference) < 40 && PYTHAG(x_component, y_component) < 155) {
				i->damage(10);

				//push targets
				double magnitude = PYTHAG(x - i->get_x(), y - i->get_y());
				double x_component_unit_vector = (x - i->get_x()) / magnitude;
				double y_component_unit_vector = (y - i->get_y()) / magnitude;
				const double push_force = -7;
				i->apply_force(push_force * x_component_unit_vector, push_force * y_component_unit_vector);
			}

		}

		tenticles_out--;
	}
	if (type == SQUID && squid_tenticles_out > 0) {
		tenticle_texture.render(-80 * cos(angle * PI_OVER_180) + x, 80 * sin(angle * PI_OVER_180) + y, bearing + 180);
		double angle_to_target, angle_difference, y_component, x_component;
		for (ant *i : other_ants) {
			x_component = x - i->get_x();
			y_component = -1 * (y - i->get_y());
			angle_to_target = atan(y_component/x_component) * ONE_EIGHTY_OVER_PI;	
			if (x_component < 0)
				angle_to_target += 180;
			if (x_component > 0 && y_component < 0)
				angle_to_target += 360;

			angle_difference = angle - angle_to_target;
			if (angle_difference > 180)
				angle_difference = angle_difference - 360;

			if (abs(angle_difference) < 40 && PYTHAG(x_component, y_component) < 155) {
				i->damage(10);

				//push targets
				double magnitude = PYTHAG(x - i->get_x(), y - i->get_y());
				double x_component_unit_vector = (x - i->get_x()) / magnitude;
				double y_component_unit_vector = (y - i->get_y()) / magnitude;
				const double push_force = -7;
				i->apply_force(push_force * x_component_unit_vector, push_force * y_component_unit_vector);
			}
		}
		apply_force(7 * cos(angle * PI_OVER_180), -7 * sin(angle * PI_OVER_180));
	}
	if (squid_tenticles_out > -20)
		squid_tenticles_out--;
	sprite.render(x, y, bearing);

	if (type == HIPSTER) {
		if (guitar > 0 && stamina > 0 && health <= 100) {
			guitar--;
			guitar_texture.render(45 * cos(angle * PI_OVER_180) + x + 25, -45 * sin(angle * PI_OVER_180) + y + 25, bearing);
			damage(-1);
			stamina -= 2;
		} else if (guitar > 0) {
			guitar = 0;
		}
	}

	if (type == ARC && arc_turn > 0) {
		if (arc_left) {
			move(LEFT);
			move(LEFT);
			move(LEFT);
		} else {
			move(RIGHT);
			move(RIGHT);
			move(RIGHT);
		}

		move(FORWARDS);
		move(FORWARDS);
		arc_turn--;
	}

	if (type == QUEEN) {
		int index = 0;
		for (ant *i : child) {
			ant *target = NULL;
			double smallest_distance = 12345;
			for (ant *ants : other_ants)
				if (smallest_distance > PYTHAG(ants->get_x() - x, ants->get_y() - y)) {
					smallest_distance = PYTHAG(ants->get_x() - x, ants->get_y() - y);
					target = ants;
				}


			if (smallest_distance != 12345) {
				i->ai(target);
				i->render();
				i->apply_physics();
			}

			//kill dead children
			if (!i->is_alive()) {
				for (ant *j : other_ants) {
					j->remove_other_ants(i);
				}
				child.erase(child.begin() + index);
			}
			index++;
		}
	}
}

void ant::apply_force(double x_component, double y_component)
{
	velocity[0] += x_component/mass;
	velocity[1] += y_component/mass;
}

void ant::apply_physics()
{
	//cap velocity
	/*const double velocity_cap = 50;
	  if (velocity[0] > velocity_cap)
	  velocity[0] = velocity_cap;
	  if (velocity[0] < -velocity_cap)
	  velocity[0] = -velocity_cap;
	  if (velocity[1] > velocity_cap)
	  velocity[1] = velocity_cap;
	  if (velocity[1] < -velocity_cap)
	  velocity[1] = -velocity_cap;*/

	//apply velocity
	x += velocity[0];
	y += velocity[1];

	//wrap position
	const int out_of_bounds_border = -30;
	/*if (x + 50 > SCREEN_WIDTH + out_of_bounds_border)
	  x = -out_of_bounds_border - 50;
	  if (x + 50 < -out_of_bounds_border)
	  x = SCREEN_WIDTH + out_of_bounds_border - 50;
	  if (y + 50 < -out_of_bounds_border)
	  y = SCREEN_HEIGHT + out_of_bounds_border - 50;
	  if (y + 50 > SCREEN_HEIGHT + out_of_bounds_border)
	  y = -out_of_bounds_border - 50;

	//cap position
	if (x + 50 > SCREEN_WIDTH + out_of_bounds_border)
	x = SCREEN_WIDTH + out_of_bounds_border - 50;
	if (x + 50 < -out_of_bounds_border)
	x = -out_of_bounds_border - 50;
	if (y + 50 < -out_of_bounds_border)
	y = -out_of_bounds_border - 50;
	if (y + 50 > SCREEN_HEIGHT + out_of_bounds_border)
	y = SCREEN_HEIGHT + out_of_bounds_border - 50;
	*/

	//apply angular momentum
	bearing -= angular_momentum;
	if (bearing < 0)
		bearing += 360;
	angle = 450 - bearing;
	if (angle >= 360)
		angle -= 360;



	//friction/air resistance
	velocity[0] += abs(velocity[0])/velocity[0] * -0.5 / mass;
	velocity[1] += abs(velocity[1])/velocity[1] * -0.5 / mass;
	angular_momentum += abs(angular_momentum)/angular_momentum * -0.6 / mass;
	if (abs(velocity[0]) < 0.0001)
		velocity[0] = 0;
	if (abs(velocity[1]) < 0.0001)
		velocity[1] = 0; 
	if (abs(angular_momentum) < 0.0001)
		angular_momentum = 0;

	//ants repel
	double distance;
	for (ant *i : other_ants) {
		distance = sqrt(pow(i->get_x() - x, 2) + pow(i->get_y() - y, 2));
		if (distance < 50) {
			velocity[0] -= ANT_REPEL_FORCE * (i->get_x() - x)/distance;
			velocity[1] -= ANT_REPEL_FORCE * (i->get_y() - y)/distance;
		}
	}

	if (stamina <= 100 - stamina_regen) {//stamina regen cap
		stamina += stamina_regen * grease_effect;
	}

	//black hole child physics
	double x_component, y_component, rotation;
	for (black_hole *i : holes) {
		//pull other ants
		for (ant *each_ant : other_ants) {
			x_component = 0;//force from black hole passed by reference
			y_component = 0;
			i->pull_ants(each_ant->get_x(), each_ant->get_y(), each_ant->get_mass(), x_component, y_component, rotation);
			each_ant->apply_force(x_component, y_component);
			each_ant->apply_rotational_force(rotation);
		}
	}

	//apply grease
	if (type == GREASY_BOY) {
		for (ant *target_ant : other_ants) {
			bool in_grease = false;
			for (grease_trap *i : grease)
				if (i->tick(target_ant->get_x() + 50, target_ant->get_y() + 50))
					in_grease = true;
			target_ant->set_grease_effect(in_grease);
		}

		bool in_grease = false;
		for (grease_trap *i : grease)
			if (i->tick(x + 50, y + 50))
				in_grease = true;
		if (in_grease) {
			grease_effect = 1.5;
			damage(-0.4);
		} else {
			grease_effect = 1;
		}
	}

	//slowly die
	if (type == ANTDO) {
		damage(0.1);
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
	if (damage > 0) {
		if (type == YA_BOY)
			damage *= 1.5;
		if (type == MOONBOY)
			damage *= 0.5;
		if (type == WEEB)
			damage *= 1.6;
	}
	if (damage > 0 | health - damage <= 100) {
		health -= damage;
		if (type != MATT && type != SQUID)
			mass -= damage/150;
	}
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
			if (stamina > 75) {
				black_hole *hole = new black_hole(x, y, angle);
				holes.push_back(hole);
				stamina -= 75;
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
			if (stamina > 0 && guitar == 0) {
				guitar = TICKS_PER_FRAME;
			}
			break;

		case ARC:
			if (stamina >= 5 && arc_turn == 0) {
				stamina -= 5;
				arc_turn = 180/(turn_speed*3);
				srand(seed++);
				arc_left = rand()%2;
			}
			break;
		case GREASY_BOY:
			if (stamina >= 70) {
				stamina -= 70;
				grease.push_back(new grease_trap(x + sprite.get_width()/2, y + sprite.get_height()/2));
			}
			break;

		case WEEB:
			if (stamina >= 60) {
				stamina -= 60;
				tenticles_out = TICKS_PER_FRAME/2;
			}
			break;

		case ANTDO:
			if (stamina >= 70) {
				stamina -= 70;

				srand(seed++);
				ant *switcher_ant = other_ants[0];
				double transfer_health = switcher_ant->get_health();
				//go through the standard damage funtion for mass
				switcher_ant->damage(transfer_health - health);
				damage(health - transfer_health);
			}
			break;
		case QUEEN:
			if (stamina >= 53& health > 20 && child.size() < 3) {
				damage(20);
				stamina -= 53;

				srand(seed++);
				child.push_back(new ant(BOT, 45 * cos(angle * PI_OVER_180) + x + 25, -45 * sin(angle * PI_OVER_180) + y + 25));
				child[child.size() - 1]->set_other_ants(other_ants);
				for (ant *i : other_ants) {
					i->add_other_ants(child[child.size() - 1]);
				}
			}
			break;

		case SQUID:
			if (stamina >= 15 && squid_tenticles_out <= -10) {
				stamina -= 15;
				squid_tenticles_out = TICKS_PER_FRAME/6;
			}
			break;

	}
}

void ant::nip()
{
	const double stamina_take = 20;
	if (stamina >= stamina_take && nip_out_timer < -TICKS_PER_FRAME/2) {
		stamina -= stamina_take;
		int nip_pos[2] = {45 * cos(angle * PI_OVER_180) + x + 25, -45 * sin(angle * PI_OVER_180) + y + 25};
		double distance = 0;
		nip_out_timer = TICKS_PER_FRAME/2;//0.5 seconds
		for (ant *i : other_ants) {
			distance = sqrt(pow(nip_pos[0] - i->get_x() - 25, 2) + pow(nip_pos[1] - i->get_y() - 25, 2));
			if (distance < 50) {
				i->damage(nip_damage * grease_effect);

				//push targets
				double magnitude = PYTHAG(x - i->get_x(), y - i->get_y());
				double x_component_unit_vector = (x - i->get_x()) / magnitude;
				double y_component_unit_vector = (y - i->get_y()) / magnitude;
				const double push_force = -10;
				i->apply_force(push_force * x_component_unit_vector, push_force * y_component_unit_vector);

			}
		}
	}
}

void ant::tesla()
{
	const int cost_coefficient = 10;
	tesla_target = NULL;
	double shortest_distance = 999999;
	for (ant *i : other_ants) {
		double distance = sqrt(pow(x - i->get_x(), 2) + pow(y - i->get_y(), 2));
		if (distance < shortest_distance) {
			shortest_distance = distance;
			tesla_target = i;
		}
	}

	if (stamina >= shortest_distance/cost_coefficient + 20 && tesla_bolt == NULL) {
		stamina -= shortest_distance/cost_coefficient + 20;
		delete tesla_bolt;
		tesla_bolt = new electric_bolt(x + 50, y + 50);
		//apply attraction
		double magnitude = PYTHAG(x - tesla_target->get_x(), y - tesla_target->get_y());
		double x_component_unit_vector = (x - tesla_target->get_x()) / magnitude;
		double y_component_unit_vector = (y - tesla_target->get_y()) / magnitude;
		const double pull_force = 8;

		tesla_target->apply_force(pull_force * x_component_unit_vector, pull_force * y_component_unit_vector);
		apply_force(-pull_force * x_component_unit_vector, -pull_force * y_component_unit_vector);


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

double ant::get_stamina()
{
	return stamina;
}

void ant::flip()
{
	if (stamina >= 30 && flip_timer == 0) {
		stamina -= 30;
		bearing += 180;
		if (bearing > 360)
			bearing -= 360;
		angle = 450 - bearing;
		flip_timer = TICKS_PER_FRAME/2;
	}
}

void ant::set_grease_effect(bool on)
{
	if (type != GREASY_BOY) {
		if (on) {
			grease_effect = 0.5;
			damage(0.2);
		} else {
			grease_effect = 1;
		}
	}
}

void ant::change_speed(double value)
{
	speed += value;
}

void ant::apply_rotational_force(double angular_force)
{
	angular_momentum += angular_force;
}

void ant::set_position(int new_x, int new_y)
{
	x = new_x;
	y = new_y;

	if (x > SCREEN_WIDTH/2) {
		bearing = 270;
		angle = 180;
	} else {
		bearing = 90;
		angle = 0;
	}
}

void ant::add_other_ants(ant *other_ants_)
{
	other_ants.push_back(other_ants_);
}

void ant::remove_other_ants(ant *other_ants_)
{
	for (int i = 0; i < other_ants.size(); i++) {
		if (other_ants[i] == other_ants_)
			other_ants.erase(other_ants.begin() + i);
	}
}

void ant::ai(ant *target)
{
	//maths
	double x_component = target->get_x() - get_x();
	double y_component = -1 * (target->get_y() - get_y());
	double angle_to_target = atan(y_component/x_component) * ONE_EIGHTY_OVER_PI;
	if (x_component < 0)
		angle_to_target += 180;
	if (x_component > 0 && y_component < 0)
		angle_to_target += 360;
	double angle_difference = get_angle() - angle_to_target;
	if (angle_difference > 180)
		angle_difference = angle_difference - 360;
	double distance = sqrt(pow(x_component, 2) + pow(y_component, 2));
	double stamina_ratio = get_stamina() * 100 / target->get_stamina();

	//apply behaviour
	switch (state) {
		case AGGRESSIVE:
			//turning
			if (angle_difference < -15 + right_bias)
				move(LEFT);
			if (angle_difference > 15 + right_bias)
				move(RIGHT);

			//move forwards
			if (abs(angle_difference) < 30 && distance > 60)
				move(FORWARDS);

			srand(seed++);
			if (abs(angle_difference) < 80 && distance < 70 && rand()%9 == 0)
				nip();
			break;
		case MALFUNCTION:
			{
				srand(seed++);
				int turn = rand()%3;
				if (turn == 0)
					move(LEFT);
				if (turn == 1)
					move(RIGHT);
				srand(seed++);
				if (rand()%3 != 1)
					move(FORWARDS);
				srand(seed++);
				if (rand()%20 == 0)
					nip();
				srand(seed++);
				if (rand()%15 == 0)
					state = past_state;
				break;
			}
		case FLEE:
			if (distance < 500) {
				if (angle_difference > -5)
					move(LEFT);
				if (angle_difference < 5)
					move(RIGHT);
				move(FORWARDS);
			}
			break;
		case OFF_SCREEN:
			{
				//maths
				double x_component = SCREEN_WIDTH/2 - get_x();
				double y_component = -1 * (SCREEN_HEIGHT/2 - get_y());
				double angle_to_target = atan(y_component/x_component) * ONE_EIGHTY_OVER_PI;
				if (x_component < 0)
					angle_to_target += 180;
				if (x_component > 0 && y_component < 0)
					angle_to_target += 360;
				double angle_difference = get_angle() - angle_to_target;
				if (angle_difference > 180)
					angle_difference = angle_difference - 360;

				//turning
				if (angle_difference < -5)
					move(LEFT);
				if (angle_difference > 5)
					move(RIGHT);

				//move forwards
				if (abs(angle_difference) < 30)
					move(FORWARDS);

				if (abs(get_x() - SCREEN_WIDTH/2) < SCREEN_WIDTH/2 - 100 && abs(get_y() - SCREEN_HEIGHT/2) < SCREEN_HEIGHT/2 - 100)
					state = past_state;
			}

	}

	//behaviour checks
	srand(seed++);
	if (get_x() - 50 < 0 | get_x() + 50 > SCREEN_WIDTH | get_y() - 50 < 0 | get_y() + 50 > SCREEN_HEIGHT) {
		past_state = state;
		state = OFF_SCREEN;
	} else if (rand()%inteligence == 0) {
		past_state = state;
		state = MALFUNCTION;
	} else if (stamina_ratio < 20)
		state = FLEE;
	else
		state = AGGRESSIVE;
}

void ant::reset()
{
	angular_momentum = 0;
	tenticles_out = 0;
	grease_effect = 1;
	for (auto *i : grease)
		delete i;
	grease.clear();
	for (auto *i : holes)
		delete i;
	holes.clear();
	arc_turn = 0;
	flip_timer = 0;
	nip_out_timer = 0;
	nip_damage = 40;
	laser_on = 0;
	guitar = 0;
	alive = true;
	mass = 1;
	velocity[0] = 0;
	velocity[1] = 0;
	speed = 8;
	turn_speed = 5;
	health = 100;
	stamina = 0;
	stamina_regen = 0.22;
	nip_texture.load_texture((std::string)"res/" + (std::string)RES_PACK + (std::string)"/nip.png");
	tesla_bolt = NULL;
	tesla_target = NULL;
	squid_tenticles_out = 0;

	if (bar_health != NULL) {
		delete bar_health;
		bar_health = NULL;
	}
	if (bar_stamina != NULL) {
		delete bar_stamina;
		bar_stamina = NULL;
	}
	bar_health = new bar(90, 10);
	bar_stamina = new bar(60, 7);


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
			guitar_texture.load_texture((std::string)"res/" + (std::string)RES_PACK + (std::string)"/guitar.png");
			sprite.load_texture((std::string)"res/" + (std::string)RES_PACK + (std::string)"/hipster.png");
			speed *= 1.2;
			mass *= 0.6;
			break;
		case BOT:
			sprite.load_texture((std::string)"res/" + (std::string)RES_PACK + (std::string)"/bot.png");
			speed *= 0.8;
			turn_speed *= 0.8;
			health *= 0.5;
			nip_damage *= 0.8;
			stamina_regen *= 0.5;
			break;

		case MOONBOY:
			sprite.load_texture((std::string)"res/" + (std::string)RES_PACK + (std::string)"/moonboy.png");
			nip_damage *= 1.5;
			mass *= 2;
			break;

		case ARC:
			sprite.load_texture((std::string)"res/" + (std::string)RES_PACK + (std::string)"/the_arc.png");
			speed *= 1.5;
			break;

		case GREASY_BOY:
			sprite.load_texture((std::string)"res/" + (std::string)RES_PACK + (std::string)"/greasy_boy.png");
			speed *= 0.7;
			nip_damage *= 0.8;
			mass *= 0.8;
			break;

		case WEEB:
			sprite.load_texture((std::string)"res/" + (std::string)RES_PACK + (std::string)"/weeb.png");
			tenticle_texture.load_texture((std::string)"res/" + (std::string)RES_PACK + (std::string)"/tenticles.png");
			speed *= 1.2;
			turn_speed *= 1.5;
			break;

		case MATT:
			sprite.load_texture((std::string)"res/" + (std::string)RES_PACK + (std::string)"/fidget_spinner.png");
			speed *= 1.4;
			break;

		case ANTDO:
			sprite.load_texture((std::string)"res/" + (std::string)RES_PACK + (std::string)"/antdo.png");
			break;

		case SQUID:
			sprite.load_texture((std::string)"res/" + (std::string)RES_PACK + (std::string)"/squid.png");
			tenticle_texture.load_texture((std::string)"res/" + (std::string)RES_PACK + (std::string)"/tenticles.png");
			speed = 0;
			stamina_regen *= 3;
			break;

		case QUEEN:
			sprite.load_texture((std::string)"res/" + (std::string)RES_PACK + (std::string)"/queen.png");
			nip_damage *= 0.6;
			break;
	};
}

ant_type ant::get_type()
{
	return type;
}
