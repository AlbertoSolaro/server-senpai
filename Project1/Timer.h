#pragma once

class Timer {
	bool clear = false;

public:
	void setTimeout(void function(void), int delay);

	void setInterval(void function(void), int interval);

	void stop();
};