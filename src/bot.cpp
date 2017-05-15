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

	//apply behaviour
	switch (state) {
		case AGGRESSIVE:
			//turning
			if (angle_difference < -5)
				base->move(LEFT);
			if (angle_difference > 5)
				base->move(RIGHT);

			//move forwards
			if (abs(angle_difference) < 30 && distance > 60)
				base->move(FORWARDS);

			if (abs(angle_difference) < 80 && distance < 70)
				base->nip();
			break;
	}
}

ant* bot::get_base()
{
	return base;
}
