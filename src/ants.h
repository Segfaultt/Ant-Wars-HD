enum direction {
	FORWARDS,
	BACKWARDS,
	LEFT,
	RIGHT
};

enum ant_type {
	YA_BOY
};

class ant {
	public:
	ant(ant_type type, int starting_x, int starting_y);

	//actions
	void nip();
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
	int x, y;
	bool alive;
	texture_wrapper sprite;
};

#include "ants.cpp"
