#pragma once
#include "Weirwood.h"
#include "Expression.h"

namespace Weirwood
{
	class ProductionRule
	{
	public:
		ProductionRule() : Active(true), mCondition(NULL) {};
		~ProductionRule(void);
		void SetCondition(const Expression& exp); 
		void AddTag(const std::string& tag);
		bool HasTag(const std::string& tag);
		inline SymbolList& Predecessor() { return mPredecessor; }
		inline SymbolList& Successor() { return mSuccessor; }
		bool Match(SymbolList& symbols, SymbolList::iterator& current);
		bool Active;
	private:
		Expression mCondition;
		StringList mTags;
		SymbolList mPredecessor;
		SymbolList mSuccessor;
	};
}
