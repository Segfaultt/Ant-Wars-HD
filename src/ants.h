#include "black_hole.h"
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
	MOONBOY
};

class ant {
	public:
	ant(ant_type type_, int starting_x, int starting_y);
	void set_other_ants(std::vector<ant *> other_ants_);
	~ant();

	//actions
	void nip();
	void ability();
	void tesla();
	void move(direction);
	void apply_force(double x_component, double y_component);
	void apply_physics();
	void render();
	void damage(double damage);
	void check_edge();
	void flip();

	//getters
	int get_x();
	int get_y();
	double get_mass();
	double get_health();
	double get_stmaina();
	double get_angle();
	bool is_alive();

	private:
	double speed,
	       turn_speed,
	       health,
	       stamina,
	       mass,
	       velocity[2],
	       bearing,
	       angle,
	       nip_damage;
	std::vector<black_hole *> holes;
	std::vector<ant *> other_ants;
	electric_bolt *tesla_bolt;
	ant *tesla_target;
	bar *bar_health, *bar_stamina;
	ant_type type;
	int x, y;
	bool alive;
	texture_wrapper sprite;
	texture_wrapper nip_texture;
	texture_wrapper guitar_texture;
	int nip_out_timer, laser_on, guitar, flip_timer;
};

#include "ants.cpp"
