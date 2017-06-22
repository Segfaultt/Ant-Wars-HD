#include <vector>

struct neuron {
	neuron();
	double get_value();
	void compute_value();
	void add_synapse(neuron *other_neuron, double weight);

	std::vector<neuron *> synapses;
	std::vector<double> weights;
	std::vector<int> innovation_numbers;
	double bias;
	double value;
	bool computed;
};

class neat_ant : public ant {
	public:
	neat_ant(ant_type type_, int starting_x, int starting_y);
	void tick();
	double get_fitness();
	void add_result(double damage_given_in_match, double damage_taken_in_match);

	private:
	neuron output_layer[7];
	std::vector<neuron *> hidden_neurons;
	neuron input_neurons[11];
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
};

#include "neat_ants.cpp"
