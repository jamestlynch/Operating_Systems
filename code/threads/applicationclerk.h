
#ifndef applicationclerk_H
#define applicationclerk_H

class applicationclerk{
	public:

	private:
		bool state;
		int lineSize;
		List* line;
		enum AppClerkState{FREE=0, BUSY=1, FINISHED=2, BREAK=3 };

};