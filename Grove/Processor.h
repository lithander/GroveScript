#pragma once
#include "Weirwood.h"
#include "Sprout.h"
#include "ProductionRule.h"
#include "Command.h"
#include "Dictionary.h"
#include <set>

namespace Weirwood
{
	class Processor : public IExpressionContext
	{
	public:
		typedef std::list<std::string> LogMessageList;

		//STATE STACK
		struct State
		{
			Variables Vars;
			Sprout::State Sprout;
		};
		typedef std::stack<State*> StateStack;
		typedef std::map<std::string, StateStack> StateStackTable;
	
		//MAIN
		Processor(Sprout* sproutPtr);
		~Processor(void);
		void Reset();
		void Run(const std::string& seqId);
		bool IsValid() { return mValid; }
		void ClearLog() { mLog.clear(); }
		LogMessageList& LogMessages() { return mLog; }

		Instruction* AppendCommand(const std::string& seqId, InstructionSet type, int blockDepth);
		ProductionRule* AppendProduction();

		std::string GetTempVar();
		void ReleaseTempVar();

		int GetVarIndex(const std::string& varName);

		//IExecutionContext
		virtual double GetVar(int i);
		virtual double GetParam(int i);
		virtual double GetTime();
		virtual void Log(const std::string& msg);
		virtual void Abort(const std::string& msg);

		void ParseSymbolList(const std::string& line, SymbolList& out_symbols, bool searchStructures = false);
	private:
		void ExecuteSymbols(SymbolList& symbols);
		void ExecuteSequence(CommandList& sequence, Variables& params);
		void ExecuteCommand(Instruction* cmd);

		//Commands
		void PushState(const std::string& stackId);
		void PopState(const std::string& stackId);
		void ClearStacks();
		void Grow(const std::string& structure, const std::string& ruleSet);
		void Seed(Instruction* pCmd);
		void Run(Instruction* pCmd);
		void Shuffle(int seed);
		//void Execute(const std::string& structOrSeqName);
		//void ExecuteWithParams(Instruction* cmd);
		void Gate(bool condition, int depth);
		void Break(int depth);
		void Repeat(int depth);
		void Print(Instruction* pCmd);

		//output
		LogMessageList mLog;
		Sprout* mSproutPtr;

		//input&state
		bool mValid;
		Dictionary<double> mVars;
		Dictionary<CommandList> mSequences;
		Dictionary<SymbolList> mStructures;
		Productions mProductions;
		StateStackTable mStacks;
		StateStack mTrash;

		//call state
		Variables* mParamsPtr;
		CommandList::iterator mNextCommand;
		CommandList::iterator mSequenceBegin;
		CommandList::iterator mSequenceEnd;

		int mTempVar;
		typedef std::set<int> IndexList;
		IndexList mTempVarIndices;
	};
}

