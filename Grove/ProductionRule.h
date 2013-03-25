#pragma once
#include "Weirwood.h"
#include "Expression.h"

namespace Weirwood
{
	class ProductionRule : public IExpressionContext
	{
		friend void swap(ProductionRule& first, ProductionRule& second);
	public:
		ProductionRule(IExpressionContext* pContext);
		ProductionRule(const ProductionRule& other);
		ProductionRule& operator=(ProductionRule other);
		~ProductionRule(void);
		void AddTag(const std::string& tag);
		bool HasTag(const std::string& tag);
		Instruction* AppendCommand(InstructionSet type, int blockDepth);
		inline CommandList& Commands() { return mCommands; }
		inline SymbolList& Predecessor() { return mPredecessor; }
		inline SymbolList& Successor() { return mSuccessor; }
		inline Expression& ParamGenerator() { return mParamGenerator; }
		inline Expression& Condition() { return mCondition; }
		inline Variables& Params() { return mParams; }
		bool Match(SymbolList& symbols, SymbolList::iterator& current);
		//IExpressionContext (use Processor to resolve everything except parameters)
		virtual double GetVar(int index);
		virtual double GetParam(int index);
		virtual double GetTime();
		virtual void Log(const std::string& msg);
		virtual void Abort(const std::string& msg);
		//can be used by client's to activate/deactive rules without much overhead. never set or read by this class.
		bool Active;
	private:
		IExpressionContext* mContextPtr;
		Variables mParams; //predeccessor names/defines params. usable in condition and form successor param generation
		Expression mCondition;
		Expression mParamGenerator;
		StringList mTags;
		SymbolList mPredecessor;
		SymbolList mSuccessor;
		CommandList mCommands;
	};
}
