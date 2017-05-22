#include <math.h>
#define PI_OVER_180 0.017453293
#define ONE_EIGHTY_OVER_PI 57.29578

bot::bot(int x, int y, ant* new_target)
{
	base = NULL;
	base = new ant(BOT, x, y);
	target = new_target;
	state = AGGRESSIVE;
	base->set_other_ants({target});

	srand(seed++);
	speed_talent = rand()%16 - 8;
	base->change_speed(speed_talent/10);
	srand(seed++);
	inteligence = rand()%15 + 1;
	srand(seed++);
	right_bias = rand()%24-12;
	srand(seed++);
	base->damage(rand()%50-25);
}

void bot::tick()
{
	//render/physics
	base->apply_physics();
	base->render();

	//maths
	double x_component = target->get_x() - base->get_x();
	double y_component = -1 * (target->get_y() - base->get_y());
	double angle_to_target = atan(y_component/x_component) * ONE_EIGHTY_OVER_PI;
	if (x_component < 0)
		angle_to_target += 180;
	if (x_component > 0 && y_component < 0)
		angle_to_target += 360;
	double angle_difference = base->get_angle() - angle_to_target;
	if (angle_difference > 180)
		angle_difference = angle_difference - 360;
	double distance = sqrt(pow(x_component, 2) + pow(y_component, 2));
	double stamina_ratio = base->get_stmaina() * 100 / target->get_stmaina();

	//apply behaviour
	switch (state) {
		case AGGRESSIVE:
			//turning
			if (angle_difference < -15 + right_bias)
				base->move(LEFT);
			if (angle_difference > 15 + right_bias)
				base->move(RIGHT);

			//move forwards
			if (abs(angle_difference) < 30 && distance > 60)
				base->move(FORWARDS);

			srand(seed++);
			if (abs(angle_difference) < 80 && distance < 70 && rand()%9 == 0)
				base->nip();
			break;
		case MALFUNCTION:
			{
				srand(seed++);
				int turn = rand()%3;
				if (turn == 0)
					base->move(LEFT);
				if (turn == 1)
					base->move(RIGHT);
				srand(seed++);
				if (rand()%3 != 1)
					base->move(FORWARDS);
				srand(seed++);
				if (rand()%20 == 0)
					base->nip();
				srand(seed++);
				if (rand()%15 == 0)
					state = past_state;
				break;
			}
		case FLEE:
			if (distance < 500) {
				if (angle_difference > -5)
					base->move(LEFT);
				if (angle_difference < 5)
					base->move(RIGHT);
				base->move(FORWARDS);
			}
			break;
		case OFF_SCREEN:
			{
				//maths
				double x_component = SCREEN_WIDTH/2 - base->get_x();
				double y_component = -1 * (SCREEN_HEIGHT/2 - base->get_y());
				double angle_to_target = atan(y_component/x_component) * ONE_EIGHTY_OVER_PI;
				if (x_component < 0)
					angle_to_target += 180;
				if (x_component > 0 && y_component < 0)
					angle_to_target += 360;
				double angle_difference = base->get_angle() - angle_to_target;
				if (angle_difference > 180)
					angle_difference = angle_difference - 360;

				//turning
				if (angle_difference < -5)
					base->move(LEFT);
				if (angle_difference > 5)
					base->move(RIGHT);

				//move forwards
				if (abs(angle_difference) < 30)
					base->move(FORWARDS);

				if (abs(base->get_x() - SCREEN_WIDTH/2) < SCREEN_WIDTH/2 - 100 && abs(base->get_y() - SCREEN_HEIGHT/2) < SCREEN_HEIGHT/2 - 100)
					state = past_state;
			}

	}

	//behaviour checks
	srand(seed++);
	if (base->get_x() - 50 < 0 | base->get_x() + 50 > SCREEN_WIDTH | base->get_y() - 50 < 0 | base->get_y() + 50 > SCREEN_HEIGHT) {
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

ant* bot::get_base()
{
	return base;
}
