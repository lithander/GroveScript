#include "Weirwood.h"
#include "ProductionRule.h"

using namespace Weirwood;

void ProductionRule::Parse(const std::string& line)
{
	int splitPosition = line.find("=>", 0);
	std::string from = line.substr(0, splitPosition);
	std::string to = line.substr(splitPosition+2);

	mPredecessor.clear();
	mContextPtr->ParseSymbolList(from, mPredecessor);
	mSuccessor.clear();
	mContextPtr->ParseSymbolList(to, mSuccessor);
}

ProductionRule::~ProductionRule(void)
{
}

bool ProductionRule::Match(SymbolList& symbols, SymbolList::iterator& current)
{
	//TODO: verify condition

	SymbolList::const_iterator symIt = current;
	//verify production pattern matches
	for(SymbolList::const_iterator it = mPredecessor.begin(); it != mPredecessor.end(); it++, symIt++)
		if(symIt == symbols.end() || (*it) != (*symIt))
			return false;
		
	//production pattern matches: 

	//1.) remove predecessor
	current = symbols.erase(current, symIt); 

	//2.) add successor
	symbols.insert(current, mSuccessor.begin(), mSuccessor.end());
	return true;
}
