void bar::render(int x, int y, int value_as_percentage)
{
	SDL_Rect foreground_rect = {x , y, (value_as_percentage * length)/100, height};

	int red = 1 - ((double)value_as_percentage/100 * 0xff);
	int blue = (double)value_as_percentage/100 * 0xff;
	SDL_SetRenderDrawColor(renderer, red, blue, 0x11, 0x55);
	SDL_RenderFillRect(renderer, &foreground_rect);

	SDL_Rect background_rect = {x + (value_as_percentage * length)/100 , y, length - (value_as_percentage * length)/100, height};
	SDL_SetRenderDrawColor(renderer, 0x90, 0x90, 0x90, 0xff);
	SDL_RenderDrawRect(renderer, &background_rect);
}
