#include <vector>

class electric_bolt
{
	public:
		electric_bolt(int starting_x, int starting_y);
		bool is_alive();
		void tick(int target_x, int target_y);

	private:
		double x, y;
		std::vector<int> last_x;
		std::vector<int> last_y;
		texture_wrapper ball;
		bool alive;
};

#include "tesla.cpp"
