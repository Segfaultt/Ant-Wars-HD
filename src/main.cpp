#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <fstream>

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
#include "bot.h"
#include "timer.h"

enum ui {
	MENU,
	ONE_PLAYER_GAME,
	TWO_PLAYER_GAME,
	GAME_OVER
};

void add_new_score(unsigned int score, std::fstream& file, ant_type type)
{
	file.seekg(0, std::ios_base::end);

	std::string type_name;
	switch (type) {
		case HIPSTER:
			type_name = "Hipster";
			break;

		case YA_BOY:
			type_name = "Ya boy";
			break;
		case CSS_BAD:
			type_name = "C## bad";
			break;
		case ARC:
			type_name = "Mr. V";
			break;
		case LUCA:
			type_name = "Luca";
			break;
		case MOONBOY:
			type_name = "Moonboy";
			break;
		default:
			type_name = "Unkown";
			break;
	}
	time_t raw_time;
	time(&raw_time);
	file << std::endl << type_name << std::endl << "kills: " << score  << std::endl << ctime(&raw_time);
}

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
	options.load_text("Press 2 to start a two player game\n Press 1 to start a signle player game", {0xf0, 0xa0, 0xf0, 0xff}, "res/default/Cousine-Regular.ttf", 30);

	//fps count on screen
	bool show_fps = false;
	texture_wrapper fps_count;

	//load choosers
	class ant_type_chooser {
		private:
			texture_wrapper ya_boy,
					luca,
					jeff,
					hipster,
					moonboy,
					arc;
			int x,y;

		public:
			ant_type_chooser(int x_, int y_)
			{
				ya_boy.load_text("Our boy walace", {0x3b, 0xcd, 0xd1}, "res/default/Cousine-Regular.ttf", 30);
				luca.load_text("not even smol", {250,0,0}, "res/default/Cousine-Regular.ttf", 30);
				jeff.load_text("C## bad", {0xb2, 0x55, 0x00}, "res/default/Cousine-Regular.ttf", 30);
				hipster.load_text("hipster ant", {0x50, 0xd0, 0x50}, "res/default/Cousine-Regular.ttf", 30);
				moonboy.load_text("moonboy", {0x70, 0x70, 0x70}, "res/default/Cousine-Regular.ttf", 30);
				arc.load_text("Mr. V", {0x70, 0x70, 0xb0}, "res/default/Cousine-Regular.ttf", 30);
				x = x_;
				y = y_;
			}
			void render(ant_type type) {
				/*SDL_Rect bg_rect = {x - 5, y - 5, 500, 50};
				  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xff);
				  SDL_RenderFillRect(renderer, &bg_rect);*/
				switch(type) {
					case YA_BOY:
						ya_boy.render(x - ya_boy.get_width()/2, y);
						break;

					case LUCA:
						luca.render(x - luca.get_width()/2, y);
						break;

					case CSS_BAD:
						jeff.render(x - jeff.get_width()/2,y);
						break;

					case HIPSTER:
						hipster.render(x - hipster.get_width()/2, y);
						break;

					case MOONBOY:
						moonboy.render(x - moonboy.get_width()/2, y);
						break;
					case ARC:
						arc.render(x - arc.get_width()/2, y);
						break;
				}
			}
	};



	//load end screen
	texture_wrapper game_over;
	game_over.load_texture((std::string)"res/" + (std::string)RES_PACK + (std::string)"/game_over.png");
	texture_wrapper right_ant_win;
	right_ant_win.load_texture((std::string)"res/" + (std::string)RES_PACK + (std::string)"/right_ant_win.png");
	texture_wrapper left_ant_win;
	left_ant_win.load_texture((std::string)"res/" + (std::string)RES_PACK + (std::string)"/left_ant_win.png");

	//set up ants
	ant *right_ant = NULL, *left_ant = NULL;
	ant_type_chooser right_ant_chooser(SCREEN_WIDTH*3/4, SCREEN_HEIGHT*3/4);
	ant_type_chooser left_ant_chooser(SCREEN_WIDTH/4, SCREEN_HEIGHT*3/4);
	ant_type right_ant_type = YA_BOY, left_ant_type = YA_BOY;
	int right_ant_type_timer = 0, left_ant_type_timer = 0;

	//single player set up
	int kill_count;
	texture_wrapper kill_count_texture;
	std::vector<bot *> bots;
	std::fstream single_player_scores("./high_scores.txt");
	if (!single_player_scores.is_open()) {
		single_player_scores.open("./high_scores.txt", std::ios_base::trunc);
	}

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
			if (e.key.keysym.sym == SDLK_f)
				show_fps = true; //!show_fps;
			if (ui_state == MENU && e.key.keysym.sym == SDLK_1) {
				ui_state = ONE_PLAYER_GAME;
				right_ant = new ant(right_ant_type, SCREEN_WIDTH-100, SCREEN_HEIGHT/2);
				bots.push_back(new bot(SCREEN_WIDTH/2, SCREEN_HEIGHT/2, right_ant));
				right_ant->set_other_ants({bots[0]->get_base()});
				kill_count = 0;
				kill_count_texture.load_text(std::to_string(kill_count), {0xff, 0x22, 0x22}, "res/default/Cousine-Regular.ttf", 40);
			} else if (ui_state == MENU && e.key.keysym.sym == SDLK_2) {
				ui_state = TWO_PLAYER_GAME;
				left_ant = new ant(left_ant_type, 50, SCREEN_HEIGHT/2);
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

						case CSS_BAD:
							right_ant_type = HIPSTER;
							break;

						case HIPSTER:
							right_ant_type = MOONBOY;
							break;

						case MOONBOY:
							right_ant_type = ARC;
							break;

						default:
							right_ant_type = YA_BOY;
							break;
					}
					right_ant_type_timer = TICKS_PER_FRAME/2;
				}
			} else if (ui_state == MENU && e.key.keysym.sym == SDLK_9) {
				if (left_ant_type_timer <= 0) {
					switch (left_ant_type) {
						case YA_BOY:
							left_ant_type = LUCA;
							break;

						case LUCA:
							left_ant_type = CSS_BAD;
							break;

						case CSS_BAD:
							left_ant_type = HIPSTER;
							break;

						case HIPSTER:
							left_ant_type = MOONBOY;
							break;

						case MOONBOY:
							left_ant_type = ARC;
							break;

						default:
							left_ant_type = YA_BOY;
							break;
					}
					left_ant_type_timer = TICKS_PER_FRAME/2;
				}

			} else if (right_ant != NULL && e.key.keysym.sym == SDLK_k) {
				right_ant->ability();
			} else if (left_ant != NULL && e.key.keysym.sym == SDLK_v) {
				left_ant->ability();
			} else if (right_ant != NULL && e.key.keysym.sym == SDLK_l) {
				right_ant->nip();
			} else if (left_ant != NULL && e.key.keysym.sym == SDLK_c) {
				left_ant->nip();
			} else if (right_ant != NULL && e.key.keysym.sym == SDLK_j) {
				right_ant->flip();
			} else if (left_ant != NULL && e.key.keysym.sym == SDLK_b) {
				left_ant->flip();
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
		} else if (ui_state == ONE_PLAYER_GAME) {
			//right ant control
			if (currentKeyStates[SDL_SCANCODE_LEFT])
				right_ant->move(LEFT);
			if (currentKeyStates[SDL_SCANCODE_UP])
				right_ant->move(FORWARDS);
			if (currentKeyStates[SDL_SCANCODE_RIGHT])
				right_ant->move(RIGHT);
			if (currentKeyStates[SDL_SCANCODE_DOWN])
				right_ant->move(BACKWARDS);
		}
		//=====life checks=====
		if (ui_state == TWO_PLAYER_GAME) {//two player
			right_ant->check_edge();
			left_ant->check_edge();
			if (right_ant->is_alive() + left_ant->is_alive() != 2) {
				ui_state = GAME_OVER;
			}
		}
		if (ui_state == ONE_PLAYER_GAME) {//single player
			right_ant->check_edge();
			if (!right_ant->is_alive()) {
				ui_state = GAME_OVER;
				add_new_score(kill_count, single_player_scores, right_ant_type);
			}

			int pos = 0;
			for (bot *i : bots) {
				ant *base_ant = i->get_base();
				if (base_ant != NULL) {
					base_ant->check_edge();
					if (!base_ant->is_alive()) {
						bots.erase(bots.begin() + pos);

						//replacement ants
						bots.push_back(new bot(0, SCREEN_HEIGHT, right_ant));
						bots.push_back(new bot(SCREEN_WIDTH, SCREEN_HEIGHT, right_ant));
						right_ant->set_other_ants({bots[bots.size()-1]->get_base(), bots[bots.size()-2]->get_base()});

						//kill count
						kill_count++;
						kill_count_texture.load_text(std::to_string(kill_count), {0xff, 0x22, 0x22}, "res/default/Cousine-Regular.ttf", 40);
					}
				}
				pos++;
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
			options.render((SCREEN_WIDTH - options.get_width())/2, SCREEN_HEIGHT/2);
			right_ant_chooser.render(right_ant_type);
			left_ant_chooser.render(left_ant_type);
		} else if (ui_state == ONE_PLAYER_GAME) {//render game with one ant
			right_ant->apply_physics();
			right_ant->render();
			if (right_ant->get_health() <= 99.9)
				right_ant->damage(-0.04);
			for (bot *i : bots) {
				i->tick();
			}
			kill_count_texture.render((SCREEN_WIDTH - kill_count_texture.get_width())/2, SCREEN_HEIGHT - kill_count_texture.get_height() - 5);
		} else if (ui_state == TWO_PLAYER_GAME) {//render game with two ants
			right_ant->apply_physics();
			right_ant->render();
			left_ant->apply_physics();
			left_ant->render();
		} else if (ui_state == GAME_OVER) {
			game_over.render(SCREEN_WIDTH/2 - 300, SCREEN_HEIGHT/4);
			if (left_ant != NULL && right_ant != NULL) {
				if (right_ant->is_alive()) {
					right_ant_win.render(SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/4);
				} else {
					left_ant_win.render(SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/4);
				}
			}
			if (left_ant != NULL) {
				delete left_ant;
				left_ant = NULL;
			}
			if (right_ant != NULL) {
				delete right_ant;
				right_ant = NULL;
			}
			bots.clear();
		}

		//frame cap
		if (fps_timer.get_time() > 1000) {
			fps = frames/(fps_timer.get_time() / 1000);
			//fps count
			if (show_fps) {
				fps_count.load_text(std::to_string((int)fps), {0xff, 0xff, 0xff}, "res/default/Cousine-Regular.ttf", 20);
				fps_count.render(0, 0);
			}

		}
		if (TICKS_PER_FRAME > cap_timer.get_time()) {
			SDL_Delay(TICKS_PER_FRAME - cap_timer.get_time());
		}
		frames++;

		//render
		SDL_RenderPresent(renderer);
	}

	single_player_scores.close();
	close();
	return 0;
}
