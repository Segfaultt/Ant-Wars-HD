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
	void render();

	//getters
	int get_x();
	int get_y();
	double get_health();

	private:
	double speed;
	double turn_speed;
	double health;
	double stamina;
	double mass;
	double angle;
	int x, y;
	texture_wrapper sprite;
};

#include "ants.cpp"
