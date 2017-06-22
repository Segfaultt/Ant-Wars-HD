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
	if (synapses.size() != weights.size()) {
		std::cout << "error: size of weights and size of synapses do not match\n";
		value = 0;
	} else {
		double z = bias;

		for (int i = 0; i < synapses.size(); i++)
			z += synapses[i]->get_value() * weights[i];

		value = (1/(1 + exp(-z)));
		computed = true;
	}
}

void neuron::add_synapse(neuron *other_neuron, double weight)
{
	synapses.push_back(other_neuron);
	weights.push_back(weight);
	innovation_numbers.push_back(innovation_number++);
}

//=====Ants=====
neat_ant::neat_ant(ant_type type_, int starting_x, int starting_y) : ant(type_, starting_x, starting_y)
{
	damage_given = 0;
	damage_taken = 1;//no undefined or unreasonably high fitnesses from low damage_taken

	//set all nodes to default
	for (int i = 0; i < 7; i++) {
		output_layer[i].bias = 0;
	}
	for (int i = 0; i < 11; i++) {
		input_neurons[i].bias = 0;
		input_neurons[i].computed = true;
	}

	hidden_neurons.push_back(new neuron());
	hidden_neurons.push_back(new neuron());
	hidden_neurons.push_back(new neuron());

	hidden_neurons[0]->bias = 2;
	hidden_neurons[1]->bias = -1;
	hidden_neurons[2]->bias = -1;

	hidden_neurons[0]->add_synapse(hidden_neurons[1],-2);
	hidden_neurons[0]->add_synapse(hidden_neurons[2],-2);
	hidden_neurons[1]->add_synapse(&input_neurons[0],2);
	hidden_neurons[1]->add_synapse(&input_neurons[2],4);
	hidden_neurons[2]->add_synapse(&input_neurons[0],1);
	hidden_neurons[2]->add_synapse(&input_neurons[2],2);

	output_layer[4].bias = -1;
	output_layer[3].bias = 1;
	output_layer[4].add_synapse(hidden_neurons[0], 2);
	output_layer[3].add_synapse(hidden_neurons[0], -2);
}

void neat_ant::tick()
{
	apply_physics();
	render();
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
	input_neurons[1].value = y/SCREEN_HEIGHT;
	input_neurons[2].value = angle/360;
	input_neurons[3].value = angle_to_target/(2*PI);
	input_neurons[4].value = health/100;
	input_neurons[5].value = target->get_health()/100;
	input_neurons[6].value = stamina/100;
	input_neurons[7].value = target->get_stamina()/100;
	input_neurons[8].value = PYTHAG(x_component,y_component)/PYTHAG(SCREEN_WIDTH, SCREEN_HEIGHT);
	input_neurons[9].value = velocity[0];
	input_neurons[10].value = velocity[1];

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

	std::cout <<angle<<'\t'<< input_neurons[2].get_value() << std::endl;
	srand(seed++);
	if (rand()%20==0)
			move(LEFT);
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
