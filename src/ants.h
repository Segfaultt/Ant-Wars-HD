enum direction {
	FORWARDS,
	BACKWARDS,
	LEFT,
	RIGHT
};

class ant {
	public:
	ant(int starting_x, int starting_y);

	//actions
	void nip();
	void move(direction);
	void apply_force(double x_component, double y_component);

	//getters
	int get_x();
	int get_y();
	double get_health();

	protected:
	double speed,
	       health,
	       stamina,
	       mass;
	int x, y;
};

#include "ants.cpp"
