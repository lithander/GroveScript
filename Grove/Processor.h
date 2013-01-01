#pragma once
#include "Weirwood.h"
#include "Sprout.h"
#include "ProductionRule.h"
#include "Command.h"

namespace Weirwood
{
	class Processor : public ICommandContext<InstructionSet>, public IProductionContext
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

		Command* AppendCommand(const std::string& seqId);
		ProductionRule* AppendProduction();

		//IExpressionIndex
		int GetVarIndex(const std::string& name);
		virtual Variables* GetVars() { return &mVars; }
		virtual double GetTime();
		virtual void Log(const std::string& msg);
		virtual void Abort(const std::string& msg);
		//ICommandContext
		virtual InstructionSet GetOperationType(const std::string& opToken);
		//IProductionContext
		virtual void ParseSymbolList(const std::string& line, SymbolList& out_symbols);
	private:
		void Execute(SymbolList& symbols);
		void Execute(Command* cmd);

		//Commands
		void SetVariable(const std::string& name, double value);
		void PushState(const std::string& stackId);
		void PopState(const std::string& stackId);
		void ClearStacks();
		void Grow(SymbolList& inout_axiom, int iterations);
		void Grow(const std::string& line, int iterations);
		//TODO: Print is too app-specific to fit into Processor
		void Print(const std::string& token);

		//output
		LogMessageList mLog;
		Sprout* mSproutPtr;

		//input&state
		bool mValid;
		Variables mVars;
		Productions mProductions;
		StateStackTable mStacks;
		Sequences mSequences;

		//index tables
		IndexTable mSequenceIndexTable;
		IndexTable mSymbolIndexTable;
		IndexTable mVarIndexTable;

		int GetSequenceIndex(const std::string& name);
		int GetSymbolIndex(const std::string& name);
						
		//tmp
		std::stack<State*> mStateCache;
		std::stack<SymbolList*> mGrowSymbols;
	};
}

