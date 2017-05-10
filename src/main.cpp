#include <iostream>
#include <thread>
#include <string>
#include <vector>

#ifdef _WIN32
#include <SDL.h>
#include <SDL_image.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#endif

//screen dimensions
#define SCREEN_WIDTH 1366
#define SCREEN_HEIGHT 768

SDL_Renderer* renderer = NULL;
SDL_Window* window = NULL;


bool quit = false; //looping flag

//other files
#include "texture_wrapper.h"
#include "ants.h"

enum ui {
	MENU,
	TWO_PLAYER_GAME
};

//ant right_ant(YA_BOY, SCREEN_WIDTH/2, SCREEN_HEIGHT/2);//right ant
bool init()
{
	bool success = true;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		std::cerr << "Could not init SDL: " << SDL_GetError() << std::endl;
		success = false;
	} else {
		window = SDL_CreateWindow("ANT WARS HD", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (window == NULL) {
			std::cerr << "Could not create window: " << SDL_GetError() << std::endl;
			success = false;
		} 
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
		if (renderer == NULL) {
			std::cerr << "Could not create renderer: " << SDL_GetError() << std::endl;
			success = false;
		}
	}

	return success;
}

void close()
{
	SDL_DestroyRenderer(renderer);
	renderer = NULL;
	SDL_DestroyWindow(window);
	window = NULL;
}

int main()
{
	if (!init()) {
		std::cerr << "failed to init\n";
		return 1;
	}

	class background_texture : public texture_wrapper {
		SDL_Rect rect;
		public:
		background_texture()
		{
			rect.x = 0;
			rect.y = 0;
			rect.w = SCREEN_WIDTH;
			rect.h = SCREEN_HEIGHT;
		}
		void render()
		{
			SDL_RenderCopy(renderer, texture, NULL, &rect);
		}
	};

	//=====Menu preparation=====
	//load background
	ui ui_state = MENU;
	background_texture background;
	background.load_texture((std::string)"res/" + (std::string)RES_PACK + (std::string)"/bg.jpg");

	//load title
	texture_wrapper title;
	title.load_texture((std::string)"res/" + (std::string)RES_PACK + (std::string)"/title.png");

	//load options
	texture_wrapper options;
	options.load_texture((std::string)"res/" + (std::string)RES_PACK + (std::string)"/options.png");

	ant right_ant(YA_BOY, SCREEN_WIDTH*3/4, SCREEN_HEIGHT/2);//right ant
	ant left_ant(YA_BOY, SCREEN_WIDTH/4, SCREEN_HEIGHT/2);//right ant
	std::vector<ant *> left_ant_vector;
	left_ant_vector.push_back(&left_ant);
	black_hole hole(50, 50, left_ant_vector);

	//=====main loop=====
	bool quit = false;
	SDL_Event e;
	while (!quit) {
		//=====input events=====
		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_QUIT)
				quit = true;
			if (ui_state == MENU && e.key.keysym.sym == SDLK_SPACE) {
				ui_state = TWO_PLAYER_GAME;
			}
		}
		const Uint8* currentKeyStates = SDL_GetKeyboardState( NULL );

		if (ui_state == TWO_PLAYER_GAME) {
			//right ant control
			if (currentKeyStates[SDL_SCANCODE_LEFT])
				right_ant.move(LEFT);
			if (currentKeyStates[SDL_SCANCODE_UP])
				right_ant.move(FORWARDS);
			if (currentKeyStates[SDL_SCANCODE_RIGHT])
				right_ant.move(RIGHT);
			if (currentKeyStates[SDL_SCANCODE_DOWN])
				right_ant.move(BACKWARDS);
			if (currentKeyStates[SDL_SCANCODE_K])
				left_ant.apply_force(10 / (left_ant.get_x() - right_ant.get_x()), 10 / (left_ant.get_y() - right_ant.get_y()));

			//left ant control
			if (currentKeyStates[SDL_SCANCODE_A])
				left_ant.move(LEFT);
			if (currentKeyStates[SDL_SCANCODE_W])
				left_ant.move(FORWARDS);
			if (currentKeyStates[SDL_SCANCODE_D])
				left_ant.move(RIGHT);
			if (currentKeyStates[SDL_SCANCODE_S])
				left_ant.move(BACKWARDS);
		}

		//=====rendering=====
		//render background
		background.render();

		if (ui_state == MENU) {//render menu
			title.render(SCREEN_WIDTH/2 - 250, 0);
			options.render(SCREEN_WIDTH/2 - 220, SCREEN_HEIGHT/2);
		} else if (ui_state == TWO_PLAYER_GAME) {//render game with two ants
			hole.render();
			hole.pull_ants();
			right_ant.apply_physics();
			right_ant.render();
			left_ant.apply_physics();
			left_ant.render();
		}

		//render
		SDL_RenderPresent(renderer);
	}

	close();
	return 0;
}
