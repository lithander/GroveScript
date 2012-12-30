#pragma once
#include "Weirwood.h"

namespace Weirwood
{
	class Processor;

	class ScriptReader
	{
	public:
		ScriptReader(void);
		~ScriptReader(void);
		bool Parse(std::istream& input, Processor* procPtr);
	};
}
