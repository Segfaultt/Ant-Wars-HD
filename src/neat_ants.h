#include <vector>

struct neuron {
	std::vector<neuron *> synapses;
	std::vector<double> weights;
	std::vector<int> innovation_numbers;
	std::vector<bool> enabled;

	neuron();
	double get_value();
	void compute_value();
	double get_value(std::vector<neuron *> parent_chain);
	void compute_value(std::vector<neuron *> parent_chain);
	void add_synapse(neuron *other_neuron, double weight);
	void add_synapse(neuron *other_neuron, double weight, int innovation_number_, bool enabled_);
	void set_id();
	bool operator<(const neuron& other);
	bool leads_to(neuron *other);

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
		//other
		neat_ant(ant_type type_, int starting_x, int starting_y);
		~neat_ant();
		bool flipped;

		//actions
		void tick();
		void set_as_starter();
		void display_brain();
		void close_display();

		//setters/adders
		void add_result(double damage_given_in_match, double damage_taken_in_match, double final_distance);
		//getters
		int get_id();
		int get_no_hidden_neurons();
		double get_adjusted_fittness(std::vector<neat_ant *> population);
		double get_fitness();
		int get_fights();
		int get_mutability();

		//friends
		friend neat_ant& cross_over(neat_ant &mother, neat_ant &father);
		friend double compatibility_distance(neat_ant &ant1, neat_ant &ant2);

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
		 * 0	|	turn right
		 * 1	|	turn left
		 * 2	|	move forwards
		 * 3	|	move backwards
		 * 4	|	nip
		 * 5	|	flip
		 * 6	|	ability
		 */
		double damage_given, damage_taken, final_distance_sum;
		int fights, no_of_synapses, id;
		int mutability;//Pr(mutation) = 1/mutability
		bool window_open,
		     fitness_known;
		double adjusted_fitness;
		int tick_count;
		SDL_Window *brain_window;
		SDL_Renderer *brain_renderer;
		texture_wrapper name;
};

#include "neat_ants.cpp"
