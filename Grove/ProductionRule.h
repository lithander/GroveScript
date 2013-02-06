#pragma once
#include "Weirwood.h"

namespace Weirwood
{
	class ProductionRule
	{
	public:
		ProductionRule() : Active(true) {};
		~ProductionRule(void);
		void AddTag(const std::string& tag);
		bool HasTag(const std::string& tag);
		inline SymbolList& Predecessor() { return mPredecessor; }
		inline SymbolList& Successor() { return mSuccessor; }
		bool Match(SymbolList& symbols, SymbolList::iterator& current);
		bool Active;
	private:
		StringList mTags;
		SymbolList mPredecessor;
		SymbolList mSuccessor;
	};
}
