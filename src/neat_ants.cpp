#include <algorithm>

#define ANT_BRAIN_WINDOW_HEIGHT 500
#define ANT_BRAIN_WINDOW_WIDTH 600
#define NEURON_RADIUS 15

//=====Neuron=====
neuron::neuron()
{

}

double neuron::get_value()
{
	if (!computed)
		compute_value();
	return value;
}

//apply sigmoidal activation function
void neuron::compute_value()
{
	double z = bias;

	for (int i = 0; i < synapses.size(); i++) {
		z += synapses[i]->get_value() * weights[i];
	}

	value = (1/(1 + exp(-z)));
	computed = true;
}

void neuron::add_synapse(neuron *other_neuron, double weight)//add an entirely new synapse
{
	synapses.push_back(other_neuron);
	weights.push_back(weight);
	innovation_numbers.push_back(innovation_number++);
}

bool neuron::operator<(const neuron& other)
{
	return id < other.id;
}

void neuron::display_synapses(SDL_Renderer* &brain_renderer)
{
	for (int i = 0; i < synapses.size(); i++) {
		int lum = (bias + 2) * 63.75;
		if (lum > 0xff)
			lum = 0xff;
		if (lum < 0)
			lum = 0;

		SDL_SetRenderDrawColor(brain_renderer, lum, lum, lum, 0xff);
		SDL_RenderDrawLine(brain_renderer, x, y, synapses[i]->x, synapses[i]->y);
		
		//tree recursion
		synapses[i]->display_synapses(brain_renderer);
	}
}

//=====Ants=====
neat_ant::neat_ant(ant_type type_, int starting_x, int starting_y) : ant(type_, starting_x, starting_y)
{
	damage_given = 0;
	damage_taken = 1;//no undefined or unreasonably high fitnesses from low damage_taken
	window_open = false;
	brain_window = NULL;
	brain_renderer = NULL;

	//set all nodes to default
	for (int i = 0; i < 7; i++) {
		output_layer[i].id = i;
		output_layer[i].bias = 0;
		output_layer[i].x = i * ANT_BRAIN_WINDOW_WIDTH/7 + ANT_BRAIN_WINDOW_WIDTH/14;
		output_layer[i].y = NEURON_RADIUS + 5;
	}
	for (int i = 0; i < 13; i++) {
		input_neurons[i].bias = 0;
		input_neurons[i].computed = true;
		input_neurons[i].id = i + 7;
		input_neurons[i].x = i * ANT_BRAIN_WINDOW_WIDTH/13 + ANT_BRAIN_WINDOW_WIDTH/26;
		input_neurons[i].y = ANT_BRAIN_WINDOW_HEIGHT - NEURON_RADIUS - 5;
	}

	//test brain
	output_layer[4].bias = -1;
	output_layer[4].add_synapse(&input_neurons[0], 2);
	output_layer[3].bias = 1;
	output_layer[3].add_synapse(&input_neurons[0], -2);
}

neat_ant::~neat_ant()
{
	if (window_open) {
		SDL_DestroyWindow(brain_window);
		SDL_DestroyRenderer(brain_renderer);
		brain_window = NULL;
		brain_renderer = NULL;
		window_open = false;
	}
}

void neat_ant::tick()
{
	SDL_Delay(100);
	apply_physics();
	render();
	check_edge();

	for (neuron *i : hidden_neurons)
		i->computed = false;
	for (int i = 0; i < 7; i++) {
		output_layer[i].computed = false;
	}

	ant *target = other_ants[0];
	double x_component = target->get_x() - x;
	double y_component = -1 * (target->get_y() - y);
	double angle_to_target = atan(y_component/x_component);
	if (x_component < 0)
		angle_to_target += PI;
	if (x_component > 0 && y_component < 0)
		angle_to_target += 2*PI;

	//get input neuron values
	input_neurons[0].value = x/(double)SCREEN_WIDTH;
	input_neurons[1].value = y/(double)SCREEN_HEIGHT;
	input_neurons[2].value = angle/360;
	input_neurons[3].value = angle_to_target/(2*PI);
	input_neurons[4].value = health/100;
	input_neurons[5].value = target->get_health()/100;
	input_neurons[6].value = stamina/100;
	input_neurons[7].value = target->get_stamina()/100;
	input_neurons[8].value = PYTHAG(x_component,y_component)/100;
	input_neurons[9].value = velocity[0]/100;
	input_neurons[10].value = velocity[1]/100;
	input_neurons[11].value = angular_momentum/100;
	input_neurons[12].value = target->get_angle();

	//get and apply outputs
	if (output_layer[0].get_value() > 0.5)
		move(LEFT);
	if (output_layer[1].get_value() > 0.5)
		move(RIGHT);
	if (output_layer[2].get_value() > 0.5)
		nip();
	if (output_layer[3].get_value() > 0.5)
		move(FORWARDS);
	if (output_layer[4].get_value() > 0.5)
		move(BACKWARDS);
	if (output_layer[5].get_value() > 0.5)
		flip();
	if (output_layer[6].get_value() > 0.5)
		ability();

	//brain display
	//brighter indicates it has a higher current value
	if (window_open) {
		//clear renderer
		SDL_SetRenderDrawColor(brain_renderer, 0x80, 0x80, 0x80, 0xff);
		SDL_RenderClear(brain_renderer);

		for (int i = 0; i < 13; i++) {//input nodes
			int lum = 0xff * input_neurons[i].get_value();
			if (lum > 0xff)
				lum = 0xff;
			if (lum < 0)
				lum = 0;
			filledCircleRGBA(brain_renderer, (i * ANT_BRAIN_WINDOW_WIDTH/13 + ANT_BRAIN_WINDOW_WIDTH/26), ANT_BRAIN_WINDOW_HEIGHT - NEURON_RADIUS - 5, NEURON_RADIUS, lum, lum, lum, 0xff);
			circleRGBA(brain_renderer, (i * ANT_BRAIN_WINDOW_WIDTH/13 + ANT_BRAIN_WINDOW_WIDTH/26), ANT_BRAIN_WINDOW_HEIGHT - NEURON_RADIUS - 5, NEURON_RADIUS, 0, 0, 0, 0xff);
		}

		for (int i = 0; i < 7; i++) {//output nodes
			int lum = output_layer[i].get_value() * 0xff;
			if (lum > 0xff)
				lum = 0xff;
			if (lum < 0)
				lum = 0;

			filledCircleRGBA(brain_renderer, (i * ANT_BRAIN_WINDOW_WIDTH/7 + ANT_BRAIN_WINDOW_WIDTH/14), NEURON_RADIUS + 5, NEURON_RADIUS, lum, lum, lum, 0xff);
			circleRGBA(brain_renderer, (i * ANT_BRAIN_WINDOW_WIDTH/7 + ANT_BRAIN_WINDOW_WIDTH/14), NEURON_RADIUS + 5, NEURON_RADIUS, 0, 0, 0, 0xff);

			//connections
			output_layer[i].display_synapses(brain_renderer);
		}

		SDL_RenderPresent(brain_renderer);
	}
}

double neat_ant::get_fitness()
{
	return damage_given/damage_taken;
}

void neat_ant::add_result(double damage_given_in_match, double damage_taken_in_match)
{
	damage_given += damage_given;
	damage_taken += damage_taken;
}

void neat_ant::display_brain()
{
	brain_window = SDL_CreateWindow("ant brain", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, ANT_BRAIN_WINDOW_WIDTH, ANT_BRAIN_WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
	brain_renderer = SDL_CreateRenderer(brain_window, -1, SDL_RENDERER_ACCELERATED);
	window_open = true;
}

neat_ant& cross_over(neat_ant mother, neat_ant father)
{
	int daughter_mutability = (mother.mutability + father.mutability)/ 2;
	srand(seed++);
	daughter_mutability += rand()%8 - 4;

	//pick daughter type
	ant_type daughter_type;
	srand(seed++);
	if (rand()%(4 * daughter_mutability) == 0) {//check for mutation major mutations are 4 times less likely
		srand(seed++);
		daughter_type = ant_type(rand()%NO_OF_ANT_TYPE);
	} else {
		srand(seed++);
		if (rand()%2 == 0)
			daughter_type = father.type;
		else
			daughter_type = mother.type;
	}
	neat_ant *daughter = new neat_ant(daughter_type, 0, 0);

	//cross over neural net based on innovation numbers

	struct gene {//useful for crossover
		int owner_id, target_id, innovation_number;
		double weight;
	};
	//to take disjoint and excess genes the fitter parent is needed
	neat_ant *fitter_parent = NULL, *not_fitter_parent = NULL;
	if (mother.get_fitness() > father.get_fitness()) {
		fitter_parent = &mother;
		not_fitter_parent = &father;
	} else {
		fitter_parent = &father;
		not_fitter_parent = &mother;
	}

	//give daughter fitter_parent's neurons
	daughter->hidden_neurons = fitter_parent->hidden_neurons;
	//assign biases
	for (neuron *i : daughter->hidden_neurons) {
		int fitter_bias_index = -1,
		    not_fitter_bias_index = -1;

		for (int j = 0; j < fitter_parent->hidden_neurons.size() && fitter_bias_index == -1; j++)
			if (i->id == fitter_parent->hidden_neurons[j]->id)
				fitter_bias_index = j;

		for (int j = 0; j < not_fitter_parent->hidden_neurons.size() && not_fitter_bias_index == -1; j++)
			if (i->id == not_fitter_parent->hidden_neurons[j]->id)
				not_fitter_bias_index = j;

		srand(seed++);
		if (not_fitter_bias_index != -1 && rand()%2 == 0)
			i->bias = not_fitter_parent->hidden_neurons[not_fitter_bias_index]->bias;
		else
			i->bias = not_fitter_parent->hidden_neurons[not_fitter_bias_index]->bias;

		srand(seed++);
		if (rand()%daughter_mutability == 0) {//mutate bias
			srand(seed++);
			i->bias *= (rand()%20)/10;
		}

	}

	//cross over output neuron biases
	for (int i = 0; i < 7; i++) {
		//pick gene owner
		neat_ant *gene_owner = NULL;
		srand(seed++);
		if (rand()%2 == 0)
			gene_owner = &mother;
		else
			gene_owner = &father;
		daughter->output_layer[i].bias = gene_owner->output_layer[i].bias;//give bias

		srand(seed++);
		if (rand()%daughter_mutability == 0) {//mutate bias
			srand(seed++);
			daughter->output_layer[i].bias *= (rand()%20)/10;
		}
	}

	return *daughter;
}
