#include "Weirwood.h"
#include "ProductionRule.h"
#include "Command.h"

using namespace Weirwood;

void Weirwood::swap(ProductionRule& first, ProductionRule& second)
{
	using std::swap; 
	// by swapping the members two objects are effectively swapped
	swap(first.Active, second.Active); 
	swap(first.mCondition, second.mCondition); 
	swap(first.mTags, second.mTags);
	swap(first.mPredecessor, second.mPredecessor);
	swap(first.mSuccessor, second.mSuccessor);
	swap(first.mCommands, second.mCommands);
}

ProductionRule::ProductionRule() : Active(true), mCondition(NULL) 
{
};

ProductionRule::~ProductionRule(void)
{
	for(CommandList::iterator it = mCommands.begin(); it != mCommands.end(); it++)
		delete (*it);
	mCommands.clear();
}

ProductionRule::ProductionRule(const ProductionRule& other)
{
	Active = other.Active;
	mCondition = other.mCondition;
	mTags = other.mTags;
	mPredecessor = other.mPredecessor;
	mSuccessor = other.mSuccessor;
	for(CommandList::const_iterator it = other.mCommands.begin(); it != other.mCommands.end(); it++)
		mCommands.push_back(new Instruction(**it));
	mCommands.clear();
}

ProductionRule& ProductionRule::operator=(ProductionRule other)
{
	//COPY&SWAP IDIOM
	swap(*this, other);
    return *this;
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

Instruction* ProductionRule::AppendCommand(InstructionSet type, int blockDepth)
{
	Instruction* pCmd = new Instruction(type, blockDepth);
	mCommands.push_back(pCmd);
	return pCmd;
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
