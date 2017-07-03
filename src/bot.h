class bot {
	public:
		bot(int x, int y, ant* new_target);
		void tick();
		ant* get_base();
	private:
		ant *base;
		ant *target;
		ai_state state,
			 past_state;
		int speed_talent,
		       inteligence,
		       right_bias;
};

#include "bot.cpp"
