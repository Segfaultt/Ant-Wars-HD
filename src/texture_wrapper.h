#pragma once
#include <string.h>

class texture_wrapper {
	public:
	texture_wrapper();
	~texture_wrapper();

	//getters
	int get_height();
	int get_width();

	//other functions
	void free();
	bool load_texture(std::string path);
	void render(int x, int y);

	private:
	int width, height;
	SDL_Texture* texture;
};

#include "texture_wrapper.cpp"
