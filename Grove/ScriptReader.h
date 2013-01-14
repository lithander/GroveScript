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
		bool mGeneratingCode;
		int mBlockDepth;
		int mLineNumber;
		std::string mSeqId;
		std::string mLine;
		Processor* mProcPtr;
		void ParseProductionRule(ProductionRule* out);
		bool ParseCommand();
		void GenerateCommand(InstructionSet op, const std::string& params);
		void ParseParams(Processor::Command* out, const std::string& line, int pos = -1);
		void ParseExpression(const std::string& token, Expression* out);
		void ParseMacro();
	};
}
