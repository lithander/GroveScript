#pragma once
#include "Weirwood.h"
#include "Expression.h"

namespace Weirwood
{
	class ProductionRule
	{
		friend void swap(ProductionRule& first, ProductionRule& second);
	public:
		ProductionRule();
		ProductionRule(const ProductionRule& other);
		ProductionRule& operator=(ProductionRule other);
		~ProductionRule(void);
		void SetCondition(const Expression& exp); 
		void AddTag(const std::string& tag);
		bool HasTag(const std::string& tag);
		Instruction* AppendCommand(InstructionSet type, int blockDepth);
		inline CommandList& Commands() { return mCommands; }
		inline SymbolList& Predecessor() { return mPredecessor; }
		inline SymbolList& Successor() { return mSuccessor; }
		bool Match(SymbolList& symbols, SymbolList::iterator& current);
		bool Active;
	private:
		Expression mCondition;
		StringList mTags;
		SymbolList mPredecessor;
		SymbolList mSuccessor;
		CommandList mCommands;
	};
}
