texture_wrapper::texture_wrapper()
{
	texture = NULL;
	width = -1;
	height = -1;
}

texture_wrapper::~texture_wrapper()
{
	free();
}

bool texture_wrapper::load_texture(std::string path)
{
	this->free();
	SDL_Texture* new_texture = NULL;

	SDL_Surface* new_surface = IMG_Load(path.c_str());
	if (new_surface == NULL) {
		std::cout << "could not load image: " << SDL_GetError() << std::endl;
	} else {
		SDL_SetColorKey(new_surface, SDL_TRUE, SDL_MapRGB(new_surface->format, 0xff, 0x00, 0xff));
		new_texture = SDL_CreateTextureFromSurface(renderer, new_surface);
		if (new_texture == NULL) {
			std::cout << "could not create texture: " << SDL_GetError() << std::endl;
		} else {
			width = new_surface->w;
			height = new_surface->h;
		}
		SDL_FreeSurface(new_surface);
	}

	texture = new_texture;
	return texture != NULL;
}

void texture_wrapper::free()
{
	if (texture != NULL) {
		SDL_DestroyTexture(texture);
		texture = NULL;
		width = -1;
		height = -1;
	}
}

int texture_wrapper::get_height()
{
	return height;
}

int texture_wrapper::get_width()
{
	return width;
}

void texture_wrapper::render(int x, int y)
{
	SDL_Rect render_quad = {x, y, width, height};
	SDL_RenderCopy(renderer, texture, NULL, &render_quad);
}

void texture_wrapper::render(int x, int y, double angle)
{
	SDL_Rect render_quad = {x, y, width, height};
	SDL_Point centre = {x + width/2, y + height/2};
	SDL_RenderCopyEx(renderer, texture, NULL, &render_quad, angle, NULL, SDL_FLIP_NONE);
}

bool texture_wrapper::load_text(std::string text, SDL_Color colour, std::string font_path, int size)
{
	bool success = true;

	TTF_Font *font = TTF_OpenFont(font_path.c_str(), size);
	SDL_Surface *message_as_surface = TTF_RenderText_Solid(font, text.c_str(), colour);
	if (message_as_surface == NULL) {
		std::cerr << "Could not render text to surface: " << SDL_GetError() << std::endl;
		success = false;
	} else { 
		texture = SDL_CreateTextureFromSurface(renderer, message_as_surface);
		if (texture == NULL) {
			std::cout << "Unable to create texture of text: " << SDL_GetError() << std::endl;
			success = false;
		} else {
			width = message_as_surface->w;
			height = message_as_surface->h;
		}
		SDL_FreeSurface(message_as_surface);
	}

	return success;
}
