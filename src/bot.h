enum ai_state {
	AGGRESSIVE,
	FLEE,
	EVADE,
	MALFUNCTION
};

class bot {
	public:
		bot(int x, int y, ant* new_target);
		void tick();
		ant* get_base();
	private:
		ant *base;
		ant *target;
		ai_state state;
};

#include "bot.cpp"
