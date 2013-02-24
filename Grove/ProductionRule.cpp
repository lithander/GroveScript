#include "Weirwood.h"
#include "ProductionRule.h"

using namespace Weirwood;

ProductionRule::~ProductionRule(void)
{
}

void ProductionRule::SetCondition(const Expression& exp)
{
	mCondition = exp;
}

void ProductionRule::AddTag(const std::string& tag)
{
	if(!HasTag(tag))
		mTags.push_back(tag);
}

bool ProductionRule::HasTag(const std::string& tag)
{
	return std::find(mTags.begin(), mTags.end(), tag) != mTags.end();
}

bool ProductionRule::Match(SymbolList& symbols, SymbolList::iterator& current)
{
	if(!mCondition.IsEmpty() && mCondition.AsBool() == false)
		return false;

	SymbolList::const_iterator symIt = current;
	//verify production pattern matches
	for(SymbolList::const_iterator it = mPredecessor.begin(); it != mPredecessor.end(); it++, symIt++)
	{
		if(symIt == symbols.end() || (*it) != (*symIt))
			return false;
	}	
	//production pattern matches: 

	//1.) remove predecessor
	current = symbols.erase(current, symIt); 
	//2.) add successor
	symbols.insert(current, mSuccessor.begin(), mSuccessor.end());
	return true;
}
