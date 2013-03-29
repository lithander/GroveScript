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
	swap(first.mParamGenerator, second.mParamGenerator); 
}

ProductionRule::ProductionRule(IExpressionContext* pContext) : Active(true), mContextPtr(pContext), mCondition(this), mParamGenerator(this), mLeftContext(0), mRightContext(0)
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
	mParams = other.mParams;
	mCondition = other.mCondition;
	mParamGenerator = other.mParamGenerator;
	mTags = other.mTags;
	mPredecessor = other.mPredecessor;
	mSuccessor = other.mSuccessor;
	mParamGenerator = other.mParamGenerator;
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

void ProductionRule::SetContext(int left, int right)
{
	mLeftContext = left;
	mRightContext = right;
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
	SymbolList::const_iterator symIt = current;
	//step back to include left context
	for(int i = mLeftContext; i > 0; i--)
		if(symIt == symbols.begin())
			return false;
		else
			symIt--;

	mParams.clear();
	//verify production pattern matches and gather param values
	for(SymbolList::const_iterator it = mPredecessor.begin(); it != mPredecessor.end(); it++, symIt++)
	{
		if(symIt == symbols.end() || (symIt->Type != symIt->Type) || (it->Index != symIt->Index)) //TODO compare params too?
			return false;
		//gather param values to be used by condition & param-generation & processor when executing attached code
		mParams.insert(mParams.end(), symIt->Params.begin(), symIt->Params.end());
	}
	
	//verify condition is met
	if(!mCondition.IsEmpty() && mCondition.AsBool() == false)
		return false;

	//generate successors params
	if(!mParamGenerator.IsEmpty())
		mParamGenerator.ResolveParams(mSuccessor);

	//step back so right context doesn't get erased
	for(int i = mRightContext; i > 0; i--)
		symIt--;

	//replace predecessor with successor
	current = symbols.erase(current, symIt); 
	symbols.insert(current, mSuccessor.begin(), mSuccessor.end());
	return true;
}


//IExecutionContext
double ProductionRule::GetVar(int i) 
{ 
	return mContextPtr->GetVar(i);
}

double ProductionRule::GetParam(int i) 
{ 
	if(mParams.size() > i)
		return mParams.at(i); 
	else
		return 0.0;
}
		
double ProductionRule::GetTime()
{
	return mContextPtr->GetTime();
}

void ProductionRule::Log(const std::string& msg)
{
	return mContextPtr->Log(msg);
}

void ProductionRule::Abort(const std::string& msg)
{
	return mContextPtr->Abort(msg);
}