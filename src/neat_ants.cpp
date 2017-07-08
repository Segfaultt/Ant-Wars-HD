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
		if (enabled[i])
			z += synapses[i]->get_value() * weights[i];
	}

	value = (1/(1 + exp(-z)));
	computed = true;
}

void neuron::add_synapse(neuron *other_neuron, double weight)//add an entirely new synapse
{	//stop infinite recusion and duplicate synapses
	bool is_valid_synapse = true;

	neuron *self = this;
	if (other_neuron == self)
		is_valid_synapse = false;
	for (neuron *i : other_neuron->synapses)
		if (i->leads_to(self))
			is_valid_synapse = false;
	for (neuron *i : synapses)
		if (i == other_neuron)
			is_valid_synapse = false;

	if (is_valid_synapse) {
		synapses.push_back(other_neuron);
		weights.push_back(weight);
		innovation_numbers.push_back(innovation_number++);
		enabled.push_back(true);
	}
}

void neuron::add_synapse(neuron *other_neuron, double weight, int innovation_number_, bool enabled_)//add an already existing synapse
{	//stop infinite recusion and duplicate synapses
	bool is_valid_synapse = true;

	neuron *self = this;
	if (other_neuron == self)
		is_valid_synapse = false;
	for (neuron *i : other_neuron->synapses)
		if (i->leads_to(self))
			is_valid_synapse = false;
	for (neuron *i : synapses)
		if (i == other_neuron)
			is_valid_synapse = false;

	if (is_valid_synapse) {
		synapses.push_back(other_neuron);
		weights.push_back(weight);
		innovation_numbers.push_back(innovation_number_);
		enabled.push_back(enabled_);
	}
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

		thickLineRGBA(brain_renderer, x, y, synapses[i]->x, synapses[i]->y, round(abs(weights[i])) + 1, lum, lum, lum, 0xff);

		//tree recursion
		synapses[i]->display_synapses(brain_renderer);
	}
}

bool neuron::leads_to(neuron *other)
{
	bool leads_to = false;

	for (neuron *i : synapses)
		if (i == other | i->leads_to(other)) {
			leads_to = true;
			break;
		}

	return leads_to;
}

//=====Ants=====
neat_ant::neat_ant(ant_type type_, int starting_x, int starting_y) : ant(type_, starting_x, starting_y)
{
	damage_given = 50;
	damage_taken = 50;//no undefined or unreasonably high fitnesses from low damage_taken
	window_open = false;
	brain_window = NULL;
	brain_renderer = NULL;
	mutability = 6;
	fights = 0;
	id = ant_id++;
	fitness_known = false;
	final_distance_sum = 0;

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
	
	for (auto *i : hidden_neurons)
		delete i;
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

	name.render(x, y - 10);
}

double neat_ant::get_fitness()
{
	double average_distance = final_distance_sum/fights;
	return (100 * pow(damage_given, 1.5)/(damage_taken + average_distance/3));
}

int neat_ant::get_fights() 
{
	return fights;
}

void neat_ant::add_result(double damage_given_in_match, double damage_taken_in_match, double final_distance)
{
	damage_given += damage_given_in_match;
	damage_taken += damage_taken_in_match;
	final_distance_sum += final_distance;
	fights++;
	name.load_text("ID" + std::to_string(id) + " H" + std::to_string(hidden_neurons.size()) + " S" + std::to_string(no_of_synapses) + " G" + std::to_string(fights)+ " F" + std::to_string((int)get_fitness()), {0xff, 0xff, 0xff, 0xff}, "res/default/Cousine-Regular.ttf", 20);
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
	mutability = 2;
	for (int i = 0; i < 7; i++)
		for (int j = 0; j < 13; j++)
			output_layer[i].add_synapse(&input_neurons[j], 0);

	/*hidden_neurons.push_back(new neuron);
	  hidden_neurons[0]->set_id();
	  hidden_neurons.push_back(new neuron);
	  hidden_neurons[1]->set_id();
	  output_layer[0].add_synapse(hidden_neurons[0], 2);
	  output_layer[0].add_synapse(&input_neurons[0], 2.1);
	  hidden_neurons[0]->add_synapse(hidden_neurons[1], 5);
	  hidden_neurons[1]->add_synapse(&input_neurons[1], 5);*/
}

neat_ant& cross_over(neat_ant &mother, neat_ant &father)//passing by value messes SDL up
{
	int daughter_mutability = (mother.mutability + father.mutability)/ 2;
	srand(seed++);
	daughter_mutability += rand()%3 - 1;
	if (daughter_mutability <= 0)
		daughter_mutability = 1;

	//pick daughter type
	ant_type daughter_type;
	srand(seed++);
	if (rand()%(4 * daughter_mutability) == 0) {
		srand(seed++);
		daughter_type = ant_type(rand()%(NO_OF_ANT_TYPE-1));
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
		i->enabled.clear();
		i->computed = false;
		i->value = 0.5;
		i->bias = 0;
	}
	for (int i = 0; i < 7; i++) {
		daughter->output_layer[i].synapses.clear();
		daughter->output_layer[i].weights.clear();
		daughter->output_layer[i].innovation_numbers.clear();
		daughter->output_layer[i].enabled.clear();
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
			i->bias += ((double)(rand()%6))/10.0 - 0.3;

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
			if (daughter->output_layer[i].bias > 10) daughter->output_layer[i].bias = 10; else if (daughter->output_layer[i].bias < -10)
				daughter->output_layer[i].bias = -10;

		}
	}

	for (int i = 0; i < 7; i++) {
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

			//is it disabled
			bool is_enabled;
			if (common_gene) {
				if (fitter_parent->output_layer[i].enabled[n] + not_fitter_parent->output_layer[i].enabled[n] == 2)
					is_enabled = true;
				else
					is_enabled = (rand()%4 != 0);
			} else {
				if (fitter_parent->output_layer[i].enabled[n])
					is_enabled = true;
				else
					is_enabled = (rand()%4 != 0);

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

				if (weighting > 10)
					weighting = 10;
				else if (weighting < -10)
					weighting = -10;
			}

			daughter->output_layer[i].add_synapse(target_neuron, weighting, target_innovation_number, is_enabled);
		}
	}

	//cross over hidden synapses
	for (int i = 0; i < fitter_parent->hidden_neurons.size(); i++) {
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

			//is it disabled
			bool is_enabled;
			if (common_gene) {
				if (fitter_parent->hidden_neurons[i]->enabled[n] + not_fitter_parent->hidden_neurons[i]->enabled[n] == 2)
					is_enabled = true;
				else
					is_enabled = (rand()%4 != 0);
			} else {
				if (fitter_parent->hidden_neurons[i]->enabled[n])
					is_enabled = true;
				else
					is_enabled = (rand()%4 != 0);

			}

			//mutate weight
			srand(seed++);
			if (rand()%daughter_mutability == 0) {
				srand(seed++);
				weighting += ((double)(rand()%10))/10.0 - 0.5;

				if (weighting > 10)
					weighting = 10;
				else if (weighting < -10)
					weighting = -10;
			}

			/*daughter->hidden_neurons[i]->synapses.push_back(target_neuron);
			  daughter->hidden_neurons[i]->weights.push_back(weighting);
			  daughter->hidden_neurons[i]->innovation_numbers.push_back(target_innovation_number);
			  daughter->hidden_neurons[i]->enabled.push_back(is_enabled);*/
			daughter->hidden_neurons[i]->add_synapse(target_neuron, weighting, target_innovation_number, is_enabled);
		}
	}

	//mutate new synapses
	srand(seed++);
	if (rand()%(4 * daughter_mutability) == 0) {
		int origin_index,
		    end_index;
		neuron *origin = NULL,
		       *end = NULL;
		srand(seed++);
		origin_index = rand()%(7 + daughter->hidden_neurons.size());
		end_index = rand()%(13 + daughter->hidden_neurons.size());

		if (origin_index < 7)
			origin = &daughter->output_layer[origin_index];
		else
			origin = daughter->hidden_neurons[origin_index - 7];
		if (end_index < 13)
			end = &daughter->input_neurons[end_index];
		else
			end = daughter->hidden_neurons[end_index - 13];

		origin->add_synapse(end, 0);
	}

	//mutate new neurons inside of synapses
	struct gene {
		neuron *origin = NULL;
		neuron *end = NULL;
		int innovation_number;
		double weight;
	};

	//collect synapses
	std::vector<gene> genes;
	for (int i = 0; i < 7; i++) {//output neurons
		gene transfer;
		transfer.origin = &daughter->output_layer[i];
		for (int j = 0; j < daughter->output_layer[i].synapses.size(); j++) {
			if (daughter->output_layer[i].enabled[j]) {
				transfer.end = daughter->output_layer[i].synapses[j];
				transfer.innovation_number = daughter->output_layer[i].innovation_numbers[j];
				transfer.weight = daughter->output_layer[i].weights[j];
				genes.push_back(transfer);
			}
		}
	}
	for (int i = 0; i < daughter->hidden_neurons.size(); i++) {//hidden neurons
		gene transfer;
		transfer.origin = daughter->hidden_neurons[i];
		for (int j = 0; j < daughter->hidden_neurons[i]->synapses.size(); j++) {
			if (daughter->hidden_neurons[i]->enabled[j]) {
				transfer.end = daughter->hidden_neurons[i]->synapses[j];
				transfer.innovation_number = daughter->hidden_neurons[i]->innovation_numbers[j];
				transfer.weight = daughter->hidden_neurons[i]->weights[j];
				genes.push_back(transfer);
			}
		}
	}
	srand(seed++);
	if (rand()%(10*daughter_mutability) == 0) {//needs time to optimise already existing biases and mutations

		//pick synapse to split
		srand(seed++);
		gene split = genes[rand()%genes.size()];

		//add two newborn synapses
		daughter->hidden_neurons.push_back(new neuron);
		daughter->hidden_neurons[daughter->hidden_neurons.size()-1]->set_id();
		split.origin->add_synapse(daughter->hidden_neurons[daughter->hidden_neurons.size()-1], 1);
		daughter->hidden_neurons[daughter->hidden_neurons.size()-1]->add_synapse(split.end, split.weight);
	}

	daughter->no_of_synapses = genes.size();
	daughter->name.load_text("ID" + std::to_string(daughter->get_id()) + " H" + std::to_string(daughter->hidden_neurons.size()) + " S" + std::to_string(genes.size()) + " G0 F0", {0xff, 0xff, 0xff, 0xff}, "res/default/Cousine-Regular.ttf", 20);

	return *daughter;
}

double compatibility_distance(neat_ant &ant1, neat_ant &ant2)
{
	int disjoint = 0;//excess and disjoint count
	double weight_difference = 0;//weights and biases

	//maximum total neurons and synapses
	int N = std::max(ant1.no_of_synapses + ant1.hidden_neurons.size(), ant2.no_of_synapses + ant2.hidden_neurons.size());
	//maximum total of synapses
	int Nsynapses = std::max(ant1.no_of_synapses, ant2.no_of_synapses);

	//sum output bias difference
	for (int i = 0; i < 7; i++) {
		weight_difference += abs(ant1.output_layer[i].bias - ant2.output_layer[i].bias);
	}

	//sum output synapse weight
	for (int i = 0; i < 7; i++) {
		double sum1 = 0,
		       sum2 = 0;
		for (int k = 0; k < ant1.output_layer[i].synapses.size(); k++) {
			sum1 += ant1.output_layer[i].weights[k];
		}
		for (int k = 0; k < ant2.output_layer[i].synapses.size(); k++) {
			sum2 += ant2.output_layer[i].weights[k];
		}

		weight_difference += abs(sum1 - sum2);
	}

	//hidden layer disjoint and excess neurons
	disjoint += abs(ant1.hidden_neurons.size() - ant2.hidden_neurons.size());

	//hidden layer weighting differences
	{
		std::sort(ant1.hidden_neurons.begin(), ant1.hidden_neurons.end(), compare_neurons);
		std::sort(ant2.hidden_neurons.begin(), ant2.hidden_neurons.end(), compare_neurons);

		int iterator1 = 0,
		    iterator2 = 0;
		while (iterator1 < ant1.hidden_neurons.size() && iterator2 < ant2.hidden_neurons.size()) {
			if (ant1.hidden_neurons[iterator1]->id == ant2.hidden_neurons[iterator2]->id) {
				double sum1 = 0,
				       sum2 = 0;

				for (int k = 0; k < ant1.hidden_neurons[iterator1]->synapses.size(); k++)
					sum1 += ant1.hidden_neurons[iterator1]->weights[k];
				for (int k = 0; k < ant1.hidden_neurons[iterator1]->synapses.size(); k++)
					sum2 += ant2.hidden_neurons[iterator2]->weights[k];

				weight_difference += abs(sum1 - sum2) + abs(ant1.hidden_neurons[iterator1]->bias - ant2.hidden_neurons[iterator2]->bias);
				iterator1++;
				iterator2++;
			} else {
				if (ant1.hidden_neurons[iterator1]->id > ant2.hidden_neurons[iterator2]->id)
					iterator2++;
				else
					iterator1++;
			}
		}
	}

	return 200*disjoint/N + 50*weight_difference/Nsynapses + 4*(ant1.type != ant2.type);
}

bool same_species(neat_ant *a, neat_ant *b)
{
	const double threshold = 5;
	return compatibility_distance(*a, *b) < threshold;
}

double neat_ant::get_adjusted_fittness(std::vector<neat_ant *> population)
{
	if (!fitness_known) {
		int denominator = 1;
		for (neat_ant *i : population) {
			denominator += same_species(this, i);
		}

		adjusted_fitness = get_fitness()/denominator;
		fitness_known = true;
	}

	return 	adjusted_fitness;
}

bool compare_ants(neat_ant *first, neat_ant *second, std::vector<neat_ant *> population)
{
	return first->get_adjusted_fittness(population) > second->get_adjusted_fittness(population);
}

bool compare_ants_raw(neat_ant *first, neat_ant *second)
{
	return first->get_fitness() > second->get_fitness();
}

int neat_ant::get_id()
{
	return id;
}

int neat_ant::get_no_hidden_neurons()
{
	return hidden_neurons.size();
}

int neat_ant::get_mutability()
{
	return mutability;
}
