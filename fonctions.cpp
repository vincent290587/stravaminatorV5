#include "fonctions.h"
#include "Global.h"

using namespace mvc;


int percentageBatt (float tensionValue) {

	float fp_ = 0.;

	if (tensionValue > 3.78) {
		fp_ = 536.24*tensionValue*tensionValue*tensionValue-6723.8*tensionValue*tensionValue;
		fp_ += 28186*tensionValue-39402;

		if (fp_ > 100.) fp_ = 100.;

	} else if (tensionValue > 2.) {
		fp_ = pow(10, -11.4)*pow(tensionValue, 22.315);
	} else {
		fp_ = 0;
	}

	return (int)fp_;
}


uint32_t myFreeRam(){ // for Teensy 3.0
	uint32_t stackTop;
	uint32_t heapTop;

	// current position of the stack.
	stackTop = (uint32_t) &stackTop;

	// current position of heap.
	void* hTop = malloc(1);
	heapTop = (uint32_t) hTop;
	free(hTop);

	// The difference is the free, available ram.
	return stackTop - heapTop;
}


float get_sec_jour()
{
	static float res = 0;
	static unsigned long sec_, csec_;
	static unsigned long hour_, min_;

	hour_ = gps.time.hour(); // GMT
	min_ = gps.time.minute();
	sec_ = gps.time.second();
	csec_ = gps.time.centisecond();
	res = 3600. * hour_ + 60. * min_ + sec_ + (float)csec_ / 100.;

	return res;
}

void resetdelay_() {
	start = millis();
}




