#pragma once
#include "Weirwood.h"
#include "Sprout.h"
#include "ProductionRule.h"
#include "Command.h"

namespace Weirwood
{
	class Processor : public IExpressionContext
	{
	public:
		typedef std::map<std::string, int> IndexTable;
		typedef std::list<std::string> LogMessageList;
		typedef Command<InstructionSet> Command;

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

		Command* AppendCommand(const std::string& seqId, InstructionSet type, int blockDepth);
		ProductionRule* AppendProduction();

		//IExpressionIndex
		int GetVarIndex(const std::string& name);
		virtual Variables* GetVars() { return &mVars; }
		virtual double GetTime();
		virtual void Log(const std::string& msg);
		virtual void Abort(const std::string& msg);
		//SymbolList creation
		void FillSymbolList(const std::string& line, SymbolList& out_symbols);
	private:
		void ExecuteSymbols(SymbolList& symbols);
		void ExecuteSequence(CommandList& sequence);
		void ExecuteCommand(Command* cmd);

		//Commands
		void SetVariable(const std::string& name, double value);
		void PushState(const std::string& stackId);
		void PopState(const std::string& stackId);
		void ClearStacks();
		void Seed(const std::string& structure, const std::string& axiom);
		void Grow(const std::string& structure, const std::string& ruleSet);
		void Execute(const std::string& structOrSeqName);
		void Gate(bool condition, int depth);
		void Break(int depth);
		void Repeat(int depth);
		void Print(Command* pCmd);

		//output
		LogMessageList mLog;
		Sprout* mSproutPtr;

		//input&state
		bool mValid;
		CommandList::iterator mNextCommand;
		CommandList::iterator mSequenceBegin;
		CommandList::iterator mSequenceEnd;
		Variables mVars;
		Productions mProductions;
		StateStackTable mStacks;
		StateStack mTrash;
		Sequences mSequences;
		Structures mStructures;

		//index tables
		IndexTable mStructureIndexTable;
		IndexTable mSequenceIndexTable;
		IndexTable mSymbolIndexTable;
		IndexTable mVarIndexTable;

		int GetSequenceIndex(const std::string& name);
		int GetSymbolIndex(const std::string& name);
		int GetStructureIndex(const std::string& name);
						
		//tmp
		std::stack<State*> mStateCache;
		std::stack<SymbolList*> mGrowSymbols;
	};
}

