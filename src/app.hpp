#pragma once

#include <iostream>



#include "databases/entry_data.hpp"

class App {
	public:
		void run();

	private:
		entryDB entries;
};
