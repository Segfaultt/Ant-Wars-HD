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

//#define JOYSTICK

//screen dimensions
#define SCREEN_WIDTH 1366
#define SCREEN_HEIGHT 768
#define FPS 60
#define TICKS_PER_FRAME 1000/FPS
#define PI 3.1415927

SDL_Renderer* renderer = NULL;
SDL_Window* window = NULL;

int seed = time(NULL);
int innovation_number;
int neuron_id;
int ant_id;
const int no_of_matches = 2;

bool quit = false; //looping flag

//other files
#include "texture_wrapper.h"
#include "ants.h"
#include "neat_ants.h"
#include "bot.h"
#include "timer.h"

enum ui {
	MENU,
	ONE_PLAYER_GAME,
	TWO_PLAYER_GAME,
	GAME_OVER,
	NEAT_MENU,
	NEAT_GAME,
	NEAT_AI_GAME
};

void add_new_score(unsigned int score, std::fstream& file, ant_type type)
{ file.seekg(0, std::ios_base::end);

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
		case GREASY_BOY:
			type_name = "Italian";
			break;
		case WEEB:
			type_name = "Weeb";
			break;
		case MATT:
			type_name= "Matt the fidget spinner";
			break;
		default:
			type_name = "Unknown";
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
	//clear renderer
	//load background
	ui ui_state = MENU;
	background_texture background;
	background.load_texture((std::string)"res/" + (std::string)RES_PACK + (std::string)"/bg.jpg");

	//load title
	texture_wrapper title;
	title.load_texture((std::string)"res/" + (std::string)RES_PACK + (std::string)"/title.png");

	//load options
	SDL_Color options_colour = {0xf6, 0x0a, 0x06, 0xff};
	const int options_size = 20;
	texture_wrapper option2, option1, option3;
	option3.load_text("Press 3 to start a NEAT ant simulation", options_colour, "res/default/Cousine-Regular.ttf", options_size);
	option2.load_text("Press 2 to start a two player game", options_colour, "res/default/Cousine-Regular.ttf", options_size);
	option1.load_text("Press 1 to start a single player game", options_colour, "res/default/Cousine-Regular.ttf", options_size);
	texture_wrapper neat_option1;
	neat_option1.load_text("Press 1 to start a new simulation", options_colour, "res/default/Cousine-Regular.ttf", options_size);

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
					arc,
					greasy_boy,
					weeb,
					matt,
					antdo,
					squid,
					queen;
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
				greasy_boy.load_text("GREASY BOY", {0x70, 0xd0, 0x70}, "res/default/Cousine-Regular.ttf", 30);
				weeb.load_text("weeb", {0xb0, 0x90, 0x90}, "res/default/Cousine-Regular.ttf", 30);
				matt.load_text("Fidget spinner", {0xb0, 0x90, 0x90}, "res/default/Cousine-Regular.ttf", 30);
				antdo.load_text("antdo", {0xff, 0xff, 0x00}, "res/default/Cousine-Regular.ttf", 30);
				queen.load_text("The ant queen", {0xa0, 0x1f, 0xb0}, "res/default/Cousine-Regular.ttf", 30);
				squid.load_text("squid ant", {0x80, 0x8f, 0xf0}, "res/default/Cousine-Regular.ttf", 30);
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
					case GREASY_BOY:
						greasy_boy.render(x - greasy_boy.get_width()/2, y);
						break;
					case WEEB:
						weeb.render(x - weeb.get_width()/2, y);
						break;
					case MATT:
						matt.render(x - matt.get_width()/2, y);
						break;

					case ANTDO:
						antdo.render(x - antdo.get_width()/2, y);
						break;

					case QUEEN:
						queen.render(x - queen.get_width()/2, y);
						break;

					case SQUID:
						squid.render(x - squid.get_width()/2, y);
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

	//neat set up
	neat_ant *gladiator1 = NULL, *gladiator2 = NULL;
	int generation, match_of_generation;
	texture_wrapper generation_counter;
	int ticks_left, matches_to_do = 0;
	std::vector<neat_ant *> population;
	std::ofstream f_average_complexity,
		f_highest_fittness,
		f_types,
		f_average_mutability;


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
				right_ant = new ant(right_ant_type, SCREEN_WIDTH-150, SCREEN_HEIGHT/2);

				left_ant->set_other_ants({right_ant});
				right_ant->set_other_ants({left_ant});
			} else if (ui_state == MENU && e.key.keysym.sym == SDLK_3) {
				ui_state = NEAT_MENU;
				neuron_id = 20;
				innovation_number = 0;
				ant_id = 0;
				generation = 1;
				match_of_generation = -1;
				population.clear();
				neat_ant *first_ancestor = new neat_ant(ARC, 0, 0);
				first_ancestor->set_as_starter();
				time_t raw_time;
				time(&raw_time);
				f_highest_fittness.open("log/" + std::to_string(raw_time) + "_max_fittness.log", std::ios_base::trunc);
				f_average_complexity.open("log/" + std::to_string(raw_time) + "_mean_complexity.log", std::ios_base::trunc);
				f_average_mutability.open("log/" + std::to_string(raw_time) + "_mean_mutability.log", std::ios_base::trunc);
				f_types.open("log/" + std::to_string(raw_time) + "_types.log", std::ios_base::trunc);
				f_types << "YA-BOY LUCA CSS-BAD HIPSTER BOT MOONBOY ARC GREASY-BOY WEEB FIDGET-SPINNER SQUID ANTDO QUEEN" << std::endl;
				for (int i = 0; i < 100; i++)
					population.push_back(&cross_over(*first_ancestor, *first_ancestor));
				delete first_ancestor;
			} else if (ui_state == MENU && e.key.keysym.sym == SDLK_o) {
				kill_count = 0;
				kill_count_texture.load_text(std::to_string(kill_count), {0xff, 0x22, 0x22}, "res/default/Cousine-Regular.ttf", 40);

				SDL_SetWindowSize(window, 1, 1);
				ui_state = NEAT_AI_GAME;
				neuron_id = 20;
				innovation_number = 0;
				ant_id = 0;
				generation = 1;
				match_of_generation = -1;
				population.clear();
				neat_ant *first_ancestor = new neat_ant(ARC, 0, 0);
				first_ancestor->set_as_starter();
				for (int i = 0; i < 100; i++)
					population.push_back(&cross_over(*first_ancestor, *first_ancestor));
				delete first_ancestor;
				gladiator1 = population[0];
				bots.push_back(new bot(SCREEN_WIDTH/2, SCREEN_HEIGHT/2, gladiator1));
				gladiator1->set_other_ants({bots[0]->get_base()});
				gladiator1->reset();
				gladiator1->set_position(100, SCREEN_HEIGHT/2);
				ticks_left = 2000;
			} else if (ui_state == NEAT_MENU && e.key.keysym.sym == SDLK_0) {
				matches_to_do = 999999999;
				SDL_SetWindowSize(window, 1, 1);
			} else if (ui_state == NEAT_MENU && e.key.keysym.sym == SDLK_9) {
				matches_to_do = 10000;
				SDL_SetWindowSize(window, 1, 1);
			} else if (ui_state == NEAT_MENU && e.key.keysym.sym == SDLK_8) {
				matches_to_do = 5000;
				SDL_SetWindowSize(window, 1, 1);
			} else if (ui_state == NEAT_MENU && e.key.keysym.sym == SDLK_7) {
				matches_to_do = 1000;
				SDL_SetWindowSize(window, 1, 1);
			} else if (ui_state == NEAT_MENU && e.key.keysym.sym == SDLK_6) {
				matches_to_do = 500;
				SDL_SetWindowSize(window, 1, 1);
			} else if (ui_state == NEAT_MENU && e.key.keysym.sym == SDLK_5) {
				matches_to_do = 100;
				SDL_SetWindowSize(window, 1, 1);
			} else if (ui_state == NEAT_MENU && e.key.keysym.sym == SDLK_4) {
				matches_to_do = 50;
				SDL_SetWindowSize(window, 1, 1);
			} else if (ui_state == NEAT_MENU && e.key.keysym.sym == SDLK_3) {
				//matches_to_do = 10;
			} else if (ui_state == NEAT_MENU && e.key.keysym.sym == SDLK_2) {
				matches_to_do = 5;
			} else if (ui_state == NEAT_MENU && e.key.keysym.sym == SDLK_1) {
				matches_to_do = 1;
			} else if (ui_state == NEAT_GAME && e.key.keysym.sym == SDLK_l) {
				gladiator1->display_brain();
			} else if (ui_state == NEAT_GAME && e.key.keysym.sym == SDLK_r) {
				gladiator2->display_brain();
#ifdef JOYSTICK
			} else if (ui_state == MENU && e.key.keysym.sym == SDLK_e) {
#else
			} else if (ui_state == MENU && e.key.keysym.sym == SDLK_0) {
#endif
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

						case ARC:
							right_ant_type = GREASY_BOY;
							break;

						case GREASY_BOY:
							right_ant_type = WEEB;
							break;

						case WEEB:
							right_ant_type = MATT;
							break;

						case MATT:
							right_ant_type = ANTDO;
							break;

						case ANTDO:
							right_ant_type = QUEEN;
							break;

						case QUEEN:
							right_ant_type = SQUID;
							break;

						default:
							right_ant_type = YA_BOY;
							break;
					}
					right_ant_type_timer = TICKS_PER_FRAME/2;
				}
#ifdef JOYSTICK
			} else if (ui_state == MENU && e.key.keysym.sym == SDLK_z) {
#else
			} else if (ui_state == MENU && e.key.keysym.sym == SDLK_9) {
#endif
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

						case ARC:
							left_ant_type = GREASY_BOY;
							break;

						case GREASY_BOY:
							left_ant_type = WEEB;
							break;

						case WEEB:
							left_ant_type = MATT;
							break;

						case MATT:
							left_ant_type = ANTDO;
							break;

						case ANTDO:
							left_ant_type = QUEEN;
							break;

						case QUEEN:
							left_ant_type = SQUID;
							break;

						default:
							left_ant_type = YA_BOY;
							break;
					}
					left_ant_type_timer = TICKS_PER_FRAME/2;
				}
#ifdef JOYSTICK
			} else if (right_ant != NULL && e.key.keysym.sym == SDLK_6) {
				right_ant->ability();
			} else if (left_ant != NULL && e.key.keysym.sym == SDLK_5) {
				left_ant->ability();
			} else if (right_ant != NULL && e.key.keysym.sym == SDLK_RIGHTBRACKET) {
				right_ant->nip();
			} else if (left_ant != NULL && e.key.keysym.sym == SDLK_c) {
				left_ant->nip();
#else
			} else if (right_ant != NULL && e.key.keysym.sym == SDLK_j) {
				right_ant->flip();
			} else if (left_ant != NULL && e.key.keysym.sym == SDLK_b) {
				left_ant->flip();
			} else if (right_ant != NULL && e.key.keysym.sym == SDLK_k) {
				right_ant->ability();
			} else if (left_ant != NULL && e.key.keysym.sym == SDLK_v) {
				left_ant->ability();
			} else if (right_ant != NULL && e.key.keysym.sym == SDLK_l) {
				right_ant->nip();
			} else if (left_ant != NULL && e.key.keysym.sym == SDLK_c) {
				left_ant->nip();
#endif
			} else if (ui_state == GAME_OVER && e.key.keysym.sym == SDLK_SPACE) {
				ui_state = MENU;
			}
		}
		const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);

		if (ui_state == TWO_PLAYER_GAME) {
#ifdef JOYSTICK
			//right ant control
			if (currentKeyStates[SDL_SCANCODE_D])
				right_ant->move(LEFT);
			if (currentKeyStates[SDL_SCANCODE_R])
				right_ant->move(FORWARDS);
			if (currentKeyStates[SDL_SCANCODE_G])
				right_ant->move(RIGHT);
			if (currentKeyStates[SDL_SCANCODE_F])
				right_ant->move(BACKWARDS);

			//left ant control
			if (currentKeyStates[92])
				left_ant->move(LEFT);
			if (currentKeyStates[96])
				left_ant->move(FORWARDS);
			if (currentKeyStates[94])
				left_ant->move(RIGHT);
			if (currentKeyStates[90])
				left_ant->move(BACKWARDS);
#else
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
#endif
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

			for (int i = 0; i < bots.size(); i++) {
				ant *base_ant = bots[i]->get_base();
				if (base_ant != NULL) {
					base_ant->check_edge();
					if (!base_ant->is_alive()) {
						bots.erase(bots.begin() + i);
						i = 0;

						//replacement ants
						bots.push_back(new bot(0, SCREEN_HEIGHT, right_ant));
						bots.push_back(new bot(SCREEN_WIDTH, SCREEN_HEIGHT, right_ant));
						right_ant->set_other_ants({bots[bots.size()-1]->get_base(), bots[bots.size()-2]->get_base()});

						//kill count
						kill_count++;
						kill_count_texture.load_text(std::to_string(kill_count), {0xff, 0x22, 0x22}, "res/default/Cousine-Regular.ttf", 40);
					}
				}
			}
		}
		//start new match
		if (ui_state == NEAT_MENU && matches_to_do > 0) {
			match_of_generation++;
			if (match_of_generation >= no_of_matches * population.size()) {//1000 matches per generation at least 10 matches per ant. 20 matches average
				//store stats
				double highest_fitness = 0;
				for (neat_ant *i : population)
					if (i->get_fitness() > highest_fitness)
						highest_fitness = i->get_fitness();
				f_highest_fittness << highest_fitness << std::endl;
				int total_hidden_neurons = 0;
				int total_mutability = 0;
				int type_frequency[NO_OF_ANT_TYPE];
				for (int i = 0; i < NO_OF_ANT_TYPE; i++)
					type_frequency[i] = 0;
				for (neat_ant *i : population) {
					total_hidden_neurons += i->get_no_hidden_neurons();
					total_mutability += i->get_mutability();
					type_frequency[i->get_type()]++;
				}
				f_average_complexity << (float)total_hidden_neurons/population.size() << std::endl;
				f_average_mutability << (float)total_mutability/population.size() << std::endl;
				for (int i = 0; i < NO_OF_ANT_TYPE; i++)
					f_types << type_frequency[i] << ' ';
				f_types << std::endl;

				//prepare for next generation
				generation++;
				match_of_generation = 0;
				//sort based on fittness (fitter first)
				std::sort(population.begin(), population.end(), [population](neat_ant *a, neat_ant *b){return compare_ants(a,b,population);});
				//std::sort(population.begin(), population.end(), compare_ants_raw);

				//get survivors
				const int divisor = 4;
				std::vector<neat_ant *>::const_iterator last = population.begin() + population.size()/divisor;
				std::vector<neat_ant *>::const_iterator first = population.begin();
				std::vector<neat_ant *> survivors(first, last);

				//breed survivors
				std::vector<neat_ant *> new_population;
				for (neat_ant *father : survivors) {
					int count = divisor;
					for (int i = 0; i < population.size() && count > 0; i++) {
						if (same_species(population[i], father)) {
							count--;
							new_population.push_back(&cross_over(*father, *population[i]));
						}
					}

					//small species have to asexually reproduce
					while (count > 0) {
						count--;
						new_population.push_back(&cross_over(*father, *father));
					}
				}
				std::cout << population.size() << '\t' << new_population.size();
				//kill old ants
				survivors.clear();
				for (neat_ant *i : population)
					delete i;
				population = new_population;
				new_population.clear();
			}

			generation_counter.load_text("Generation: " + std::to_string(generation) + "-" + std::to_string(match_of_generation), {0xff, 0xff, 0xff}, "res/default/Cousine-Regular.ttf", 20);
			matches_to_do--;
			ticks_left = 720;//~2.5s real time 12s @ 60fps
			ui_state = NEAT_GAME;

			gladiator1 = population[floor(match_of_generation/no_of_matches)];
			do {
				srand(seed++);
				gladiator2 = population[rand()%population.size()];
			} while (gladiator2 == gladiator1);

			if (generation > 500) {//randomize y-position
				srand(seed++);
				gladiator1->set_position(50, (rand()%(SCREEN_HEIGHT-200))+50);
				srand(seed++);
				gladiator2->set_position(SCREEN_WIDTH - 150, (rand()%(SCREEN_HEIGHT-10))+50);
			} else {
				gladiator1->set_position(50, SCREEN_HEIGHT/2);
				gladiator2->set_position(SCREEN_WIDTH - 150, SCREEN_HEIGHT/2);
			}
			gladiator1->set_other_ants({gladiator2});
			gladiator2->set_other_ants({gladiator1});
			gladiator1->flipped = true;
			gladiator2->flipped = false;
			gladiator1->reset();
			gladiator2->reset();
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
			option1.render((SCREEN_WIDTH - option1.get_width())/2, SCREEN_HEIGHT/2);
			option2.render((SCREEN_WIDTH - option2.get_width())/2, SCREEN_HEIGHT/2 + option1.get_height());
			option3.render((SCREEN_WIDTH - option3.get_width())/2, SCREEN_HEIGHT/2 + option1.get_height() + option2.get_height());
			right_ant_chooser.render(right_ant_type);
			left_ant_chooser.render(left_ant_type);

			if (left_ant != NULL) {
				delete left_ant;
				left_ant = NULL;
			}
			if (right_ant != NULL) {
				delete right_ant;
				right_ant = NULL;
			}
			for (bot *i : bots)
				delete i;
			bots.clear();
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
			left_ant->apply_physics();
			right_ant->render();
			left_ant->render();
		} else if (ui_state == NEAT_MENU) {
			neat_option1.render((SCREEN_WIDTH - option1.get_width())/2, SCREEN_HEIGHT/2);
		} else if (ui_state == GAME_OVER) {
			game_over.render(SCREEN_WIDTH/2 - 300, SCREEN_HEIGHT/4);
			if (left_ant != NULL && right_ant != NULL) {
				if (right_ant->is_alive()) {
					right_ant_win.render(SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/4);
				} else {
					left_ant_win.render(SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/4);
				}
			}
		} else if (ui_state == NEAT_GAME) {
			gladiator1->tick();
			gladiator2->tick();
			generation_counter.render((SCREEN_WIDTH - generation_counter.get_width())/2, 50);

			if (ticks_left-- <= 0 || !(gladiator1->is_alive() && gladiator2->is_alive())) {
				gladiator1->add_result(gladiator2->get_damaged(), 100 - gladiator1->get_health(), PYTHAG(gladiator1->get_x() - gladiator2->get_x(), gladiator1->get_y() - gladiator2->get_y()));
				gladiator2->add_result(gladiator1->get_damaged(), 100 - gladiator2->get_health(), PYTHAG(gladiator1->get_x() - gladiator2->get_x(), gladiator1->get_y() - gladiator2->get_y()));
				gladiator1->close_display();
				gladiator2->close_display();
				ui_state = NEAT_MENU;
				gladiator1 = NULL;
				gladiator2 = NULL;

				if (matches_to_do <= 0) {
					SDL_SetWindowSize(window, SCREEN_WIDTH, SCREEN_HEIGHT);
				}
			}
		} else if (ui_state == NEAT_AI_GAME) {
			gladiator1->tick();
			if (gladiator1->get_health() <= 99.9)
				gladiator1->damage(-0.04);
			for (bot *i : bots) {
				i->tick();
			}
			kill_count_texture.render((SCREEN_WIDTH - kill_count_texture.get_width())/2, SCREEN_HEIGHT - kill_count_texture.get_height() - 5);
			generation_counter.render((SCREEN_WIDTH - generation_counter.get_width())/2, 50);

			for (int i = 0; i < bots.size(); i++) {
				ant *base_ant = bots[i]->get_base();
				if (base_ant != NULL) {
					base_ant->check_edge();
					if (!base_ant->is_alive()) {
						bots.erase(bots.begin() + i);
						i = 0;

						//replacement ants
						bots.push_back(new bot(0, SCREEN_HEIGHT, gladiator1));
						bots.push_back(new bot(SCREEN_WIDTH, SCREEN_HEIGHT, gladiator1));
						gladiator1->set_other_ants([bots](){std::vector<ant *> bases; for (bot *i : bots) bases.push_back(i->get_base()); return bases;}());

						//kill count
						kill_count++;
						//kill_count_texture.load_text(std::to_string(kill_count), {0xff, 0x22, 0x22}, "res/default/Cousine-Regular.ttf", 40);
					}
				}
			}
			if (ticks_left-- <= 0 || !gladiator1->is_alive()) {
				//sum alive bots damage
				double damage_sum = 0;
				for (bot *i : bots)
					damage_sum += 50-i->get_base()->get_health();
				gladiator1->add_result(50*kill_count + damage_sum, 100 - gladiator1->get_health(), 0);
				gladiator1->close_display();
				gladiator1 = NULL;
				match_of_generation++;
				if (match_of_generation >= no_of_matches * population.size()) {//1000 matches per generation at least 10 matches per ant. 20 matches average
					//prepare for next generation
					generation++;
					match_of_generation = 0;
					//sort based on fittness (fitter first)
					std::sort(population.begin(), population.end(), [population](neat_ant *a, neat_ant *b){return compare_ants(a,b,population);});

					//get survivors
					const int divisor = 4;
					std::vector<neat_ant *>::const_iterator last = population.begin() + population.size()/divisor;
					std::vector<neat_ant *>::const_iterator first = population.begin();
					std::vector<neat_ant *> survivors(first, last);

					//breed survivors
					std::vector<neat_ant *> new_population;
					for (neat_ant *father : survivors) {
						int count = divisor;
						for (int i = 0; i < population.size() && count > 0; i++) {
							if (same_species(population[i], father)) {
								count--;
								new_population.push_back(&cross_over(*father, *population[i]));
							}
						}

						//small species have to asexually reproduce
						while (count > 0) {
							count--;
							new_population.push_back(&cross_over(*father, *father));
						}
					}
					std::cout << population.size() << '\t' << new_population.size();
					//kill old ants
					survivors.clear();
					for (neat_ant *i : population)
						delete i;
					population = new_population;
					new_population.clear();
				}

				generation_counter.load_text("Generation: " + std::to_string(generation) + "-" + std::to_string(match_of_generation), {0xff, 0xff, 0xff}, "res/default/Cousine-Regular.ttf", 20);
				matches_to_do--;

				for (bot *i : bots)
					delete i;
				bots.clear();

				gladiator1 = population[floor(match_of_generation/no_of_matches)];
				gladiator1->set_position(100, SCREEN_HEIGHT/2);
				bots.push_back(new bot(SCREEN_WIDTH/2, SCREEN_HEIGHT/2, gladiator1));
				gladiator1->set_other_ants({bots[0]->get_base()});
				gladiator1->reset();
				kill_count = 0;
				ticks_left = 2000;
			}
		}


		//frame cap
		if (ui_state != NEAT_GAME && ui_state != NEAT_AI_GAME) {
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
		}

		//render
		SDL_RenderPresent(renderer);
	}

	//free memory
	if (left_ant != NULL) {
		delete left_ant;
		left_ant = NULL;
	}
	if (right_ant != NULL) {
		delete right_ant;
		right_ant = NULL;
	}
	if (gladiator1 != NULL) {
		delete gladiator1;
		gladiator1 = NULL;
	}
	if (gladiator2 != NULL) {
		delete gladiator2;
		gladiator2 = NULL;
	}
	for (bot *i : bots)
		delete i;
	bots.clear();
	for (neat_ant *i : population)
		delete i;
	population.clear();

	single_player_scores.close();
	close();

	return 0;
}
