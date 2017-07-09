#include "black_hole.h"
#include "greasy.h"
#include "tesla.h"
#include "bars.h"

enum direction {
	FORWARDS,
	BACKWARDS,
	LEFT,
	RIGHT
};

enum ant_type {
	YA_BOY,
	LUCA,
	CSS_BAD,
	HIPSTER,
	BOT,
	MOONBOY,
	ARC,
	GREASY_BOY,
	WEEB,
	MATT,
	ANTDO,
	SQUID,
	QUEEN,
	NO_OF_ANT_TYPE
};

enum ai_state {
	AGGRESSIVE,
	FLEE,
	EVADE,
	MALFUNCTION,
	OFF_SCREEN
};

class ant {
	public:
		ant(ant_type type_, int starting_x, int starting_y);
		~ant();
		void set_other_ants(std::vector<ant *> other_ants_);
		void add_other_ants(ant *other_ants_);
		void remove_other_ants(ant *other_ants_);

		//actions
		void nip();
		void ability();
		void tesla();
		void move(direction);
		void apply_force(double x_component, double y_component);
		void apply_rotational_force(double angular_force);
		void apply_physics();
		void render();
		void ai(ant *target);
		void damage(double damage);
		void check_edge();
		void flip();

		//setters
		void set_grease_effect(bool on);
		void change_speed(double value);
		void set_position(int new_x, int new_y);
		void reset();

		//getters
		int get_x();
		int get_y();
		double get_mass();
		double get_health();
		double get_stamina();
		double get_angle();
		bool is_alive();
		ant_type get_type();

	protected:
		double speed,
		       turn_speed,
		       health,
		       stamina,
		       stamina_regen,
		       mass,
		       velocity[2],
		       angular_momentum,
		       bearing,
		       angle,
		       nip_damage,
		       grease_effect;
		       std::vector<black_hole *> holes;
		       std::vector<grease_trap *> grease;
		       std::vector<ant *> child;
		       std::vector<ant *> other_ants;
		       electric_bolt *tesla_bolt;
		       ant *tesla_target;
		       bar *bar_health, *bar_stamina;
		       ant_type type;
		       double x, y;
		       bool alive, arc_left;
		       texture_wrapper sprite;
		       texture_wrapper nip_texture;
		       texture_wrapper guitar_texture;
		       texture_wrapper tenticle_texture;
		       int nip_out_timer, laser_on, guitar, tenticles_out, squid_tenticles_out, flip_timer, arc_turn;
		       ai_state state, past_state;
		       int inteligence,
			   right_bias;
};

#include "ants.cpp"
