class timer 
{
	public:
	timer();
	void start();
	Uint32 get_time();
 	private:
	Uint32 starting_ticks;
};

timer::timer()
{
	starting_ticks = 0;
}

void timer::start()
{
	starting_ticks = SDL_GetTicks();
}

Uint32 timer::get_time()
{
	return SDL_GetTicks() - starting_ticks;
}
