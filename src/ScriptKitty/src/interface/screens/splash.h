#pragma once
#include "../lib/DevKittyInterface.h"

class SplashScreen: public DevKittyScreen {
	public:
		SplashScreen(unsigned long durationInMs);
		~SplashScreen(){};
		bool draw();
		int update(int);
	private:
		unsigned long endAt;
};