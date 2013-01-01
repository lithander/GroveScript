#pragma once
#include "Weirwood.h"
#include "Processor.h"

namespace Weirwood
{
	class ScriptReader
	{
	public:
		ScriptReader(void);
		~ScriptReader(void);
		bool Parse(std::istream& input, Processor* procPtr);
	private:
		Processor* mProcPtr;
		void ParseProductionRule(const std::string& line, ProductionRule* out);
		void ParseCommand(const std::string& line, int lineNr, Processor::Command* out);
		void ParseExpression(const std::string& line, int lineNumber, Expression* out);
	};
}
