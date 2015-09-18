
#ifndef TEST2_H
#define TEST2_H

class Test2{
	public:
		void UserInput();

	private:
		bool state;
		int lineSize;
		List* line;
		enum AppClerkState{FREE, BUSY, FINISHED, BREAK };
		bool invalid;

};
#endif