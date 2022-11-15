#include <StaticThreadController.h>
#include <Thread.h>
#include <ThreadController.h>


class HumThread: public Thread {

public:
	int pin;
	bool state;
	
	LedThread(int _pin): Thread() {
		pin = _pin;
		state = true;
		pinMode(pin, OUTPUT);
	}
	
	bool shouldRun(unsigned long time){
		return Thread::shouldRun(time);
	}
	
	void run(){
		Thread::run();
		digitalWrite(pin, state ? HIGH : LOW);
		state = !state;
	}
};

