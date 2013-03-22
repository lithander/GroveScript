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
		bool Read(std::istream& input, Processor* procPtr);
	private:
		void Throw(std::string error) const;
		std::istream* mStreamPtr;
		Processor* mProcPtr;
		//State
		bool mCommentFlag;
		int mBlockDepth;
		int mLineNumber;
		int mPos;
		std::string mMeta;
		std::string mLine;
		std::string mToken;
		std::string mParamToken;
		ProductionRule* mRulePtr;
		//Meta as SequenceHeader
		std::string mSequenceId;
		IndexTable mSequenceParams;
		//Helpers
		bool ReadLine();
		bool ReadToken();
		bool ReadParam();
		std::string mStoredLine;
		void RestoreLine() { mStoredLine = mLine; };
		//Parsing
		void ParseLine();
		void ParseMeta();
		void ParseProductionRule();
		void ParseCommand();
		bool ParseParam(const std::string& token, int& out_pos, std::string& out);
		void ParseExpression(const std::string& token, Expression* out);
		//Macro (Code-gen)
		void ReadBlock(int depth);
		void GenerateRepeat();
		void GenerateWhile();
		void GenerateUntil();
		void GenerateIf();
		void GenerateElse();
		void GenerateMod(char mod);
		bool mGeneratingCode;
		Instruction* GenerateCommand(InstructionSet op, int depth);
		void GenerateCommand(InstructionSet op, const std::string& params, int depth);
	};
}
