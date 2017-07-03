class bar {
	public:
		bar(int length_, int height_) : length(length_), height(height_) {};
		void render(int x, int y, int value_as_percentage);
	private:
		int length, height;
};

#include "bars.cpp"
