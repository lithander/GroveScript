#pragma once
#include "Weirwood.h"

namespace Weirwood
{
	class ProductionRule
	{
	public:
		ProductionRule() {};
		~ProductionRule(void);
		inline SymbolList& Predecessor() { return mPredecessor; }
		inline SymbolList& Successor() { return mSuccessor; }
		bool Match(SymbolList& symbols, SymbolList::iterator& current);
	private:
		SymbolList mPredecessor;
		SymbolList mSuccessor;
	};
}
