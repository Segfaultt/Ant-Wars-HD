void bar::render(int x, int y, int value_as_percentage)
{
	SDL_Rect foreground_rect = {x , y, (value_as_percentage * length)/100, height};
	SDL_SetRenderDrawColor(renderer, 0x11, 0xaa, 0x11, 0x55);
	SDL_RenderFillRect(renderer, &foreground_rect);

	SDL_Rect background_rect = {x + (value_as_percentage * length)/100 , y, length - (value_as_percentage * length)/100, height};
	SDL_SetRenderDrawColor(renderer, 0xaa, 0x00, 0x11, 0x55);
	SDL_RenderFillRect(renderer, &background_rect);
}
