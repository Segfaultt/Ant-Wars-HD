#include <vector>

struct neuron {
	std::vector<neuron *> synapses;
	std::vector<double> weights;
	std::vector<int> innovation_numbers;

	neuron();
	double get_value();
	void compute_value();
	void add_synapse(neuron *other_neuron, double weight);
	bool operator<(const neuron& other);

	double bias;
	double value;
	bool computed;
	int id;

	//brain window stuff
	int x, y;
	void display_synapses(SDL_Renderer* &brain_renderer);
};

class neat_ant : public ant {
	public:
		neat_ant(ant_type type_, int starting_x, int starting_y);
		~neat_ant();
		void tick();
		double get_fitness();
		void add_result(double damage_given_in_match, double damage_taken_in_match);
		void display_brain();
		friend neat_ant& cross_over(neat_ant mother, neat_ant father);

	private:
		neuron output_layer[7];
		std::vector<neuron *> hidden_neurons;
		neuron input_neurons[13];
		/*
		 * Input neurons index contents:
		 * 0	|	x coord on screen as a fraction 
		 * 1	|	y coord on screen as a fraction 
		 * 2	|	angle of ant as a fraction
		 * 3	|	angle to opponent ant as a fraction
		 * 4	|	health
		 * 5	|	opponent health
		 * 6	|	stamina
		 * 7	|	opponent stamina
		 * 8	|	distance to other ant
		 * 9	|	x velocity
		 * 10	|	y velocity
		 * 11	|	spin
		 * 12	|	opponents angle
		 *
		 * Output neurons index contents:
		 * 0	|	turn left
		 * 1	|	turn right
		 * 2	|	nip
		 * 3	|	move forwards
		 * 4	|	move backwards
		 * 5	|	flip
		 * 6	|	ability
		 */
		double damage_given, damage_taken;
		int mutability;//Pr(mutation) = 1/mutability
		bool window_open;
		SDL_Window *brain_window;
		SDL_Renderer *brain_renderer;
};

#include "neat_ants.cpp"
