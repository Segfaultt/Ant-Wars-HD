#include <algorithm>

#define ANT_BRAIN_WINDOW_HEIGHT 500
#define ANT_BRAIN_WINDOW_WIDTH 600
#define NEURON_RADIUS 15

//=====Neuron=====
neuron::neuron()
{
	bias = 0;
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

void neuron::set_id()
{
		id = neuron_id++;
}

bool compare_neurons(neuron *first, neuron *second)
{
	return *first < *second;
}

void neuron::display_synapses(SDL_Renderer* &brain_renderer)
{
	for (int i = 0; i < synapses.size(); i++) {
		int lum = weights[i] * 60 + 127.5;
		if (lum > 0xff)
			lum = 0xff;
		if (lum < 0)
			lum = 0;

		thickLineRGBA(brain_renderer, x, y, synapses[i]->x, synapses[i]->y, round(abs(weights[i])), lum, lum, lum, 0xff);

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
	mutability = 6;

	//set all nodes to default
	for (int i = 0; i < 7; i++) {
		output_layer[i].id = i;
		output_layer[i].x = i * ANT_BRAIN_WINDOW_WIDTH/7 + ANT_BRAIN_WINDOW_WIDTH/14;
		output_layer[i].y = NEURON_RADIUS + 5;
	}
	for (int i = 0; i < 13; i++) {
		input_neurons[i].computed = true;
		input_neurons[i].id = i + 7;
		input_neurons[i].x = i * ANT_BRAIN_WINDOW_WIDTH/13 + ANT_BRAIN_WINDOW_WIDTH/26;
		input_neurons[i].y = ANT_BRAIN_WINDOW_HEIGHT - NEURON_RADIUS - 5;
	}
}

neat_ant::~neat_ant()
{
	close_display();
}

double angle_addition(double angle, double addition)
{
	angle += addition;
	if (angle >= 2*PI)
		angle -= 2*PI;

	return angle;
}

void neat_ant::tick()
{
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
	input_neurons[12].value = target->get_angle()/360;

	//make it think it's always the left ant by inversing inputs
	if (flipped) {
		input_neurons[0].value = 1 - input_neurons[0].value;
		input_neurons[1].value = 1 - input_neurons[1].value;
		input_neurons[2].value = angle_addition(angle * PI_OVER_180, PI)/(2*PI);
		input_neurons[3].value = angle_addition(angle_to_target, PI)/(2*PI);
		input_neurons[9].value *= -1;
		input_neurons[10].value *= -1;
		input_neurons[12].value = angle_addition(target->get_angle() * PI_OVER_180, PI)/(2*PI);
	}

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

		for (neuron *i : hidden_neurons) {//hidden neurons
			int lum = i->get_value() * 0xff;

			int bias_lum = i->bias * 60 + 127.5;
			if (bias_lum > 0xff)
				bias_lum = 0xff;
			if (bias_lum < 0)
				bias_lum = 0;

			filledCircleRGBA(brain_renderer, i->x, i->y, NEURON_RADIUS + 1 + round(abs(i->bias)), bias_lum, 0, (1 - bias_lum), 0xff);
			filledCircleRGBA(brain_renderer, i->x, i->y, NEURON_RADIUS, lum, lum, lum, 0xff);
		}

		for (int i = 0; i < 7; i++) {//output nodes
			int lum = output_layer[i].get_value() * 0xff;
			if (lum > 0xff)
				lum = 0xff;
			if (lum < 0)
				lum = 0;
			int bias_lum = output_layer[i].bias * 60 + 127.5;
			if (bias_lum > 0xff)
				bias_lum = 0xff;
			if (bias_lum < 0)
				bias_lum = 0;

			filledCircleRGBA(brain_renderer, (i * ANT_BRAIN_WINDOW_WIDTH/7 + ANT_BRAIN_WINDOW_WIDTH/14), NEURON_RADIUS + 5, NEURON_RADIUS + 1 + round(abs(output_layer[i].bias)), bias_lum, 0, (1 - bias_lum), 0xff);
			filledCircleRGBA(brain_renderer, (i * ANT_BRAIN_WINDOW_WIDTH/7 + ANT_BRAIN_WINDOW_WIDTH/14), NEURON_RADIUS + 5, NEURON_RADIUS, lum, lum, lum, 0xff);

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
	damage_given += damage_given_in_match;
	damage_taken += damage_taken_in_match;
}

void neat_ant::display_brain()
{
	if (!window_open) {
		brain_window = SDL_CreateWindow("ant brain", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, ANT_BRAIN_WINDOW_WIDTH, ANT_BRAIN_WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
		brain_renderer = SDL_CreateRenderer(brain_window, -1, SDL_RENDERER_ACCELERATED);
		window_open = true;

		//hidden node positions
		for (int i = 0; i < hidden_neurons.size(); i++) {
			hidden_neurons[i]->x = i * ANT_BRAIN_WINDOW_WIDTH/hidden_neurons.size() + ANT_BRAIN_WINDOW_WIDTH/(2*hidden_neurons.size());
			hidden_neurons[i]->y = ANT_BRAIN_WINDOW_HEIGHT/2;
		}
	}
}

void neat_ant::close_display()
{
		if (window_open) {
				SDL_DestroyWindow(brain_window);
				SDL_DestroyRenderer(brain_renderer);
				brain_window = NULL;
				brain_renderer = NULL;
				window_open = false;
		}
}

void neat_ant::set_as_starter()
{
	hidden_neurons.push_back(new neuron);
	hidden_neurons[0]->set_id();
	hidden_neurons.push_back(new neuron);
	hidden_neurons[1]->set_id();
	output_layer[0].add_synapse(hidden_neurons[0], 2);
	output_layer[0].add_synapse(&input_neurons[0], 2.1);
	hidden_neurons[0]->add_synapse(hidden_neurons[1], 5);
	hidden_neurons[1]->add_synapse(&input_neurons[1], 5);
}

neat_ant& cross_over(neat_ant &mother, neat_ant &father)//passing by value messes SDL up
{
		int daughter_mutability = (mother.mutability + father.mutability)/ 2;
		srand(seed++);
		daughter_mutability += rand()%4 - 2;
		if (daughter_mutability <= 0)
				daughter_mutability = 1;

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
		daughter->mutability = daughter_mutability;

		//=========cross over neural net based on innovation numbers=========

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
		for (neuron *i : fitter_parent->hidden_neurons) {
			daughter->hidden_neurons.push_back(new neuron(*i));
		}

		//remove synapses and biases
		for (neuron *i : daughter->hidden_neurons) {
				i->synapses.clear();
				i->weights.clear();
				i->innovation_numbers.clear();
				i->computed = false;
				i->value = 0.5;
				i->bias = 0;
		}
		for (int i = 0; i < 7; i++) {
				daughter->output_layer[i].synapses.clear();
				daughter->output_layer[i].weights.clear();
				daughter->output_layer[i].innovation_numbers.clear();
				daughter->output_layer[i].value = 0.5;
				daughter->output_layer[i].bias = 0;
		}
		//assign biases
		for (neuron *i : daughter->hidden_neurons) {
				int fitter_bias_index = -1,
					not_fitter_bias_index = -1;

				for (int j = 0; j < fitter_parent->hidden_neurons.size() && fitter_bias_index == -1; j++)
						if (i->id == fitter_parent->hidden_neurons[j]->id)
								fitter_bias_index = j;

				for (int j = 0; j < not_fitter_parent->hidden_neurons.size() && not_fitter_bias_index == -1; j++) {
						if (i->id == not_fitter_parent->hidden_neurons[j]->id)
								not_fitter_bias_index = j;
				}

				srand(seed++);
				if (not_fitter_bias_index != -1 && rand()%2 == 0)
						i->bias = not_fitter_parent->hidden_neurons[not_fitter_bias_index]->bias;
				else
						i->bias = fitter_parent->hidden_neurons[fitter_bias_index]->bias;

				srand(seed++);
				if (rand()%daughter_mutability == 0) {//mutate bias
						srand(seed++);
						i->bias += ((double)(rand()%10))/10.0 - 0.5;
						srand(seed++);
						double coefficient = ((double)(rand()%10))/10.0 + 0.5;
						i->bias *= coefficient;

						if (i->bias > 10)
								i->bias = 10;
						else if (i->bias < -10)
								i->bias = -10;
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
						daughter->output_layer[i].bias += ((double)(rand()%10))/10.0 - 0.5;
						srand(seed++);
						double coefficient = ((double)(rand()%10))/10.0 + 0.5;
						if (coefficient == 0)
								coefficient = 1;
						daughter->output_layer[i].bias *= coefficient;
						if (daughter->output_layer[i].bias > 10) daughter->output_layer[i].bias = 10; else if (daughter->output_layer[i].bias < -10)
								daughter->output_layer[i].bias = -10;

				}
		}

		//sort hidden neurons by ID
		//std::sort(daughter->hidden_neurons.begin(), daughter->hidden_neurons.end(), compare_neurons);
		//cross over output synapses
		for (int i = 0; i < 7; i++) {
				//sort synapses
				/*std::sort(fitter_parent->output_layer[i].synapses.begin(), fitter_parent->output_layer[i].synapses.end(), compare_neurons);
				  std::sort(not_fitter_parent->output_layer[i].synapses.begin(), not_fitter_parent->output_layer[i].synapses.end(), compare_neurons);*/

				for (int n = 0; n < fitter_parent->output_layer[i].synapses.size(); n++) {
						bool common_gene = false;
						int target_innovation_number = fitter_parent->output_layer[i].innovation_numbers[n];
						for (int unfit_n = 0; unfit_n < not_fitter_parent->output_layer[i].synapses.size() && !common_gene; unfit_n++) {
								if (not_fitter_parent->output_layer[i].innovation_numbers[unfit_n] == target_innovation_number) {
										common_gene = true;
								}
						}

						//find target neuron address
						neuron *target_neuron = NULL;
						int target_id = fitter_parent->output_layer[i].synapses[n]->id;
						if (target_id > 19) {
								for (neuron *iter : daughter->hidden_neurons) {
										if (iter->id == target_id) {
												target_neuron = iter;
												break;
										}
								}
						} else {
								target_neuron = &daughter->input_neurons[target_id - 7];
						}

						//get weight
						double weighting = 1;
						if (common_gene) {
								srand(seed++);
								neat_ant *gene_owner = NULL;
								if (rand()%2 == 0)
										gene_owner = &father;
								else
										gene_owner = &mother;

								weighting = gene_owner->output_layer[i].weights[n];
						} else {
								weighting = fitter_parent->output_layer[i].weights[n];
						}

						//mutate weight
						srand(seed++);
						if (rand()%daughter_mutability == 0) {
								srand(seed++);
								weighting += ((double)(rand()%10))/10.0 - 0.5;
								srand(seed++);
								double coefficient = ((double)(rand()%10))/10.0 + 0.5;
								weighting *= coefficient;

								if (weighting > 10)
										weighting = 10;
								else if (weighting < -10)
										weighting = -10;
						}

						daughter->output_layer[i].add_synapse(target_neuron, weighting);
				}
		}

		//cross over hidden synapses
		for (int i = 0; i < fitter_parent->hidden_neurons.size(); i++) {
				//sort synapses
				//std::sort(fitter_parent->hidden_neurons[i]->synapses.begin(), fitter_parent->hidden_neurons[i]->synapses.end(), compare_neurons);
				//std::sort(not_fitter_parent->hidden_neurons[i]->synapses.begin(), not_fitter_parent->hidden_neurons[i]->synapses.end(), compare_neurons);
				for (int n = 0; n < fitter_parent->hidden_neurons[i]->synapses.size(); n++) {
						bool common_gene = false;
						int target_innovation_number = fitter_parent->hidden_neurons[i]->innovation_numbers[n];

						for (int unfit_n = 0; i < not_fitter_parent->hidden_neurons.size() && unfit_n < not_fitter_parent->hidden_neurons[i]->synapses.size() && !common_gene; unfit_n++) {
								if (not_fitter_parent->hidden_neurons[i]->innovation_numbers[unfit_n] == target_innovation_number) {
										common_gene = true;
								}
						}

						//find target neuron address
						neuron *target_neuron = NULL;
						int target_id = fitter_parent->hidden_neurons[i]->synapses[n]->id;
						if (target_id > 19) {
								for (neuron *iter : daughter->hidden_neurons) {
										if (iter->id == target_id) {
												target_neuron = iter;
												break;
										}
								}
						} else {
								target_neuron = &daughter->input_neurons[target_id - 7];
						}

						//get weight
						double weighting = 1;
						if (common_gene) {
								srand(seed++);
								neat_ant *gene_owner = NULL;
								if (rand()%2 == 0)
										gene_owner = &father;
								else
										gene_owner = &mother;

								weighting = gene_owner->hidden_neurons[i]->weights[n];
						} else {
								weighting = fitter_parent->hidden_neurons[i]->weights[n];
						}

						//mutate weight
						srand(seed++);
						if (rand()%daughter_mutability == 0) {
								srand(seed++);
								weighting += ((double)(rand()%10))/10.0 - 0.5;
								srand(seed++);
								double coefficient = ((double)(rand()%10))/10.0 + 0.5;
								weighting *= coefficient;

								if (weighting > 10)
										weighting = 10;
								else if (weighting < -10)
										weighting = -10;
						}

						daughter->hidden_neurons[i]->synapses.push_back(target_neuron);
						daughter->hidden_neurons[i]->weights.push_back(weighting);
						daughter->hidden_neurons[i]->innovation_numbers.push_back(target_innovation_number);
				}
		}

		return *daughter;
}
