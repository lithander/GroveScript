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
		IndexTable mActiveParams;
		//Helpers
		bool ReadLine();
		bool ReadToken();
		bool ReadParam();
		std::string mStoredLine;
		void RestoreLine() { mStoredLine = mLine; };
		std::string RemoveContextMarkers(std::string line, int& out_leftContext, int& out_rightContext);
		//Parsing
		void ParseLine();
		void ParseMeta();
		void ParseParamNames(const std::string& token, IndexTable& out);
		void ParseProductionRule();
		void ParseCommand();
		bool ParseParam(const std::string& token, int& out_pos, std::string& out);
		void ParseExpression(const std::string& token, Expression* out, bool checkVarNames = true);
		//Macro (Code-gen)
		void ReadBlock(int depth);
		void GenerateRepeat();
		void GenerateWhile();
		void GenerateUntil();
		void GenerateIf();
		void GenerateElse();
		void GenerateMod(char mod);
		Instruction* GenerateCommand(InstructionSet op, int depth);
		void GenerateCommand(InstructionSet op, const std::string& params, int depth);
	};
}
