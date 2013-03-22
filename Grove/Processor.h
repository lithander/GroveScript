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

		virtual int GetVarIndex(const std::string& varName) { return mVars.IndexOf(varName); }
		virtual double GetVar(int i) 
		{ 
			return mVars.Retrieve(i); 
		}
		virtual double GetParam(int i) 
		{ 
			if(mParamsPtr->size() > i)
				return mParamsPtr->at(i); 
			else
				return 0.0;
		}
		
		virtual double GetTime();
		virtual void Log(const std::string& msg);
		virtual void Abort(const std::string& msg);
		//SymbolList creation
		void ParseSymbolList(const std::string& line, SymbolList& out_symbols);
	private:
		void ExecuteSymbols(SymbolList& symbols);
		void ExecuteSequence(CommandList& sequence, Variables& params);
		void ExecuteCommand(Instruction* cmd);

		//Commands
		void PushState(const std::string& stackId);
		void PopState(const std::string& stackId);
		void ClearStacks();
		void Seed(const std::string& structure, const std::string& axiom);
		void Grow(const std::string& structure, const std::string& ruleSet);
		void Execute(const std::string& structOrSeqName);
		void ExecuteWithParams(Instruction* cmd);
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
		Variables mNoParams;
		Variables* mParamsPtr;
		CommandList::iterator mNextCommand;
		CommandList::iterator mSequenceBegin;
		CommandList::iterator mSequenceEnd;

		int mTempVar;
		typedef std::set<int> IndexList;
		IndexList mTempVarIndices;

		IndexTable mSymbolIndexTable;
		int GetSymbolIndex(const std::string& name);
	};
}

