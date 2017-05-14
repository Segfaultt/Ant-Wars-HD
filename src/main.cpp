#include <iostream>
#include <thread>
#include <string>
#include <vector>

#ifdef _WIN32
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_gfx.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#endif

//screen dimensions
#define SCREEN_WIDTH 1366
#define SCREEN_HEIGHT 768
#define FPS 60
#define TICKS_PER_FRAME 1000/FPS

SDL_Renderer* renderer = NULL;
SDL_Window* window = NULL;

int seed = time(NULL);

bool quit = false; //looping flag

//other files
#include "texture_wrapper.h"
#include "ants.h"
#include "timer.h"

enum ui {
	MENU,
	TWO_PLAYER_GAME,
	GAME_OVER
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
		}  //Initialize SDL_ttf
		if( TTF_Init() == -1 ) {
			std::cerr << "Could not initialise SDL_ttf: " << SDL_GetError() << std::endl;
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

	//load choosers
	class ant_type_chooser {
		private:
			texture_wrapper ya_boy,
					luca,
					jeff;
			int x,y;

		public:
			ant_type_chooser(int x_, int y_)
			{
				ya_boy.load_text("Our boy walace", {30,40,200}, "res/default/Cousine-Regular.ttf", 40);
				luca.load_text("not even smol", {30,40,200}, "res/default/Cousine-Regular.ttf", 40);
				luca.load_text("not even smol", {30,40,200}, "res/default/Cousine-Regular.ttf", 40);
				x = x_;
				y = y_;
			}
			void render(ant_type type) {
				SDL_Rect bg_rect = {x - 5, y - 5, 500, 50};
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xff);
				SDL_RenderFillRect(renderer, &bg_rect);
				switch(type) {
					case YA_BOY:
						ya_boy.render(x, y);
						break;

					case LUCA:
						luca.render(x, y);
						break;

					case CSS_BAD:
						jeff.render(x,y);
				}
			}
	};


	ant_type_chooser right_ant_chooser(SCREEN_WIDTH*3/4, SCREEN_HEIGHT*3/4);

	//load end screen
	texture_wrapper game_over;
	game_over.load_texture((std::string)"res/" + (std::string)RES_PACK + (std::string)"/game_over.png");
	texture_wrapper right_ant_win;
	right_ant_win.load_texture((std::string)"res/" + (std::string)RES_PACK + (std::string)"/right_ant_win.png");
	texture_wrapper left_ant_win;
	left_ant_win.load_texture((std::string)"res/" + (std::string)RES_PACK + (std::string)"/left_ant_win.png");

	ant *right_ant = NULL, *left_ant = NULL;
	ant_type right_ant_type = YA_BOY, left_ant_type = YA_BOY;
	int right_ant_type_timer = 0, left_ant_type_timer = 0;

	//=====main loop=====
	bool quit = false;
	SDL_Event e;
	timer cap_timer, fps_timer;
	int frames = 0;
	float fps;
	fps_timer.start();
	while (!quit) {
		cap_timer.start();
		//=====input events=====
		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_QUIT)
				quit = true;
			if (ui_state == MENU && e.key.keysym.sym == SDLK_2) {
				ui_state = TWO_PLAYER_GAME;
				if (left_ant != NULL)
					delete left_ant;
				if (right_ant != NULL)
					delete right_ant;

				left_ant = new ant(YA_BOY, 50, SCREEN_HEIGHT/2);
				right_ant = new ant(right_ant_type, SCREEN_WIDTH-100, SCREEN_HEIGHT/2);

				left_ant->set_other_ants({right_ant});
				right_ant->set_other_ants({left_ant});
			} else if (ui_state == MENU && e.key.keysym.sym == SDLK_0) {
				if (right_ant_type_timer <= 0) {
					switch (right_ant_type) {
						case YA_BOY:
							right_ant_type = LUCA;
							break;

						case LUCA:
							right_ant_type = CSS_BAD;
							break;

						default:
							right_ant_type = YA_BOY;
							break;
					}
					right_ant_type_timer = TICKS_PER_FRAME/2;
				}
			} else if (ui_state == TWO_PLAYER_GAME && e.key.keysym.sym == SDLK_k) {
				right_ant->ability();
			} else if (ui_state == TWO_PLAYER_GAME && e.key.keysym.sym == SDLK_c) {
				left_ant->ability();
			} else if (ui_state == TWO_PLAYER_GAME && e.key.keysym.sym == SDLK_l) {
				right_ant->nip();
			} else if (ui_state == TWO_PLAYER_GAME && e.key.keysym.sym == SDLK_v) {
				left_ant->nip();
			} else if (ui_state == GAME_OVER && e.key.keysym.sym == SDLK_SPACE) {
				ui_state = MENU;
			}
		}
		const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);

		if (ui_state == TWO_PLAYER_GAME) {
			//right ant control
			if (currentKeyStates[SDL_SCANCODE_LEFT])
				right_ant->move(LEFT);
			if (currentKeyStates[SDL_SCANCODE_UP])
				right_ant->move(FORWARDS);
			if (currentKeyStates[SDL_SCANCODE_RIGHT])
				right_ant->move(RIGHT);
			if (currentKeyStates[SDL_SCANCODE_DOWN])
				right_ant->move(BACKWARDS);

			//left ant control
			if (currentKeyStates[SDL_SCANCODE_A])
				left_ant->move(LEFT);
			if (currentKeyStates[SDL_SCANCODE_W])
				left_ant->move(FORWARDS);
			if (currentKeyStates[SDL_SCANCODE_D])
				left_ant->move(RIGHT);
			if (currentKeyStates[SDL_SCANCODE_S])
				left_ant->move(BACKWARDS);
		}

		//=====life checks=====
		if (ui_state == TWO_PLAYER_GAME) {
			right_ant->check_edge();
			left_ant->check_edge();
			if (right_ant->is_alive() + left_ant->is_alive() != 2) {
				ui_state = GAME_OVER;
			}
		}

		//=====rendering=====
		//render background
		background.render();

		if (ui_state == MENU) {//render menu
			if (left_ant_type_timer > 0)
				left_ant_type_timer--;
			if (right_ant_type_timer > 0)
				right_ant_type_timer--;
			title.render(SCREEN_WIDTH/2 - 250, 0);
			options.render(SCREEN_WIDTH/2 - 220, SCREEN_HEIGHT/2);
			right_ant_chooser.render(right_ant_type);
		} else if (ui_state == TWO_PLAYER_GAME) {//render game with two ants
			right_ant->apply_physics();
			right_ant->render();
			left_ant->apply_physics();
			left_ant->render();
		} else if (ui_state == GAME_OVER) {
			game_over.render(SCREEN_WIDTH/2 - 300, SCREEN_HEIGHT/4);
			if (right_ant->is_alive()) {
				right_ant_win.render(SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/4);
			} else {
				left_ant_win.render(SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/4);
			}
		}

		//render
		SDL_RenderPresent(renderer);

		//frame cap
		if (fps_timer.get_time() > 1000) {
			fps = frames/(fps_timer.get_time() / 1000);
			std::cout << fps << std::endl;
		}
		if (TICKS_PER_FRAME > cap_timer.get_time()) {
			SDL_Delay(TICKS_PER_FRAME - cap_timer.get_time());
		}
		frames++;
	}

	close();
	return 0;
}
