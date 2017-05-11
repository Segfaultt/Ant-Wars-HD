#include "black_hole.h"
#include "bars.h"

enum direction {
	FORWARDS,
	BACKWARDS,
	LEFT,
	RIGHT
};

enum ant_type {
	YA_BOY,
	LUCA
};

class ant {
	public:
	ant(ant_type type_, int starting_x, int starting_y);
	void set_other_ants(std::vector<ant *> other_ants_);
	~ant();

	//actions
	void nip();
	void ability();
	void move(direction);
	void apply_force(double x_component, double y_component);
	void apply_physics();
	void render();
	void damage(double damage);
	void check_edge();

	//getters
	int get_x();
	int get_y();
	double get_mass();
	double get_health();
	bool is_alive();

	private:
	double speed,
	       turn_speed,
	       health,
	       stamina,
	       mass,
	       velocity[2],
	       bearing,
	       angle;
	std::vector<black_hole *> holes;
	std::vector<ant *> other_ants;
	bar *bar_health, *bar_stamina;
	ant_type type;
	int x, y;
	bool alive;
	texture_wrapper sprite;
	texture_wrapper nip_texture;
	int nip_out_timer;
};

#include "ants.cpp"
