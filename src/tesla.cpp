electric_bolt::electric_bolt(int starting_x, int starting_y) : x(starting_x), y(starting_y)
{
	alive = true;
	ball.load_texture((std::string)"res/" + (std::string)RES_PACK + (std::string)"/bolt.png");
}

void electric_bolt::tick(int target_x, int target_y)
{
	if (alive) {
		const int variance = 44;
		const double speed = 15;
		double distance = sqrt(pow(x - target_x, 2) + pow(y - target_y, 2));
		srand(seed++);
		x -= (speed * (x - target_x) / distance) + rand()%variance - variance/2;
		srand(seed++);
		y -= (speed * (y - target_y) / distance) + rand()%variance - variance/2;

		SDL_SetRenderDrawColor(renderer, 0xc1, 0x4f, 0xc8, 0xff);
		//SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
		for (int i = 1; i < last_x.size(); i++) {
			thickLineRGBA(renderer, last_x[i - 1], last_y[i - 1], last_x[i], last_y[i], 2, 0xd1, 0xbf, 0xd8, 0xff);
		}

		ball.render(x - ball.get_width()/2, y - ball.get_height()/2);

		last_x.push_back(x);
		last_y.push_back(y);
		if (distance < 20) {
			alive = false;
		}
	}
}

bool electric_bolt::is_alive()
{
	return alive;
}
