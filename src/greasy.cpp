grease_trap::grease_trap(int x_, int y_)
{
	sprite.load_texture("res/default/grease.png");
	x = x_;
	y = y_;
	alive = true;
	ticks_until_death = TICKS_PER_FRAME*30;
};

bool grease_trap::tick(int target_x, int target_y)
{
	int x_component, y_component;
	if (alive) {
		sprite.render(x - sprite.get_width()/2, y - sprite.get_height()/2);

		x_component = target_x - x;
		y_component = target_y - y;

		if (ticks_until_death-- <= 0) {
			alive = false;
		}
	}

	//if within greasy elipse
	return alive && (2.2096 * pow(y_component, 2) + pow(x_component, 2)) <= 48400;
}
