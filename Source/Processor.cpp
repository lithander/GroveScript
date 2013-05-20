#include "Weirwood.h"
#include "Processor.h"
#include "Sprout.h"
#include "Command.h"
#include "Expression.h"
#include "Keywords.h"
#include <ctime>

using namespace Weirwood;

Processor::Processor(Sprout* sproutPtr) : mTempVar(0), mParamsPtr(NULL), mNextRnd(1)
{
	mSproutPtr = sproutPtr;
	mProductions.reserve(20);
	mVars.Reserve(20);
	mSequences.Reserve(20);
	mStructures.Reserve(10);
}

Processor::~Processor(void)
{
	ClearStacks();
}

void Processor::Reset()
{
	ClearStacks();
	while(!mTrash.empty())
	{
		delete mTrash.top();
		mTrash.pop();
	}

	mNextRnd = 1;
	mParamsPtr = NULL;
	mTempVar = 0;
	mTempVarIndices.clear();
	mValid = true;
	mLog.clear();
	mSproutPtr->Reset();

	//delete Instructions appended to Sequences
	for(std::vector<CommandList>::iterator it = mSequences.Data().begin(); it != mSequences.Data().end(); it++)
		for(CommandList::iterator it2 = it->begin(); it2 != it->end(); it2++)
			delete *it2;

	//delete Productions
	for(Productions::iterator it = mProductions.begin(); it != mProductions.end(); it++)
		delete *it;

	mProductions.clear();
	mVars.Clear();
	mSequences.Clear();
	mStructures.Clear();
}

void Processor::ClearStacks()
{
	for(StateStackTable::iterator it = mStacks.begin(); it != mStacks.end(); it++)
		while(it->second.size() > 0)
		{
			State* pState = it->second.top();
			mTrash.push(pState);
			it->second.pop();	
		}
	mStacks.clear();
}

void Processor::Run(const std::string& seqId)
{
	if(!mValid)
	{
		Abort("Invalid State. Reset required!");
		return;
	}
	try
	{
		mSproutPtr->Reset();
		ClearStacks();

		for(std::vector<double>::iterator it = mVars.Data().begin(); it != mVars.Data().end(); it++)
			(*it) = 0;

		ExecuteSequence(mSequences.Retrieve(seqId), Variables());
		mSproutPtr->Flush();
	}
	catch(Error e)
	{
		Abort(e.desc);
	}
}

void Processor::ExecuteSymbols(SymbolList& symbols)
{
	for(SymbolList::iterator it = symbols.begin(); it != symbols.end(); it++)
	{
		if(it->Type == Symbol::SEQUENCE)
			ExecuteSequence(mSequences.Retrieve(it->Index), it->Params);
		else if(it->Type == Symbol::STRUCTURE)
			ExecuteSymbols(mStructures.Retrieve(it->Index));
	}
}

void Processor::ExecuteSequence(CommandList& seq, Variables& params)
{
	//store and restore internal variables
	std::vector<double> tempVarValues;
	tempVarValues.reserve(mTempVarIndices.size());
	for(IndexList::iterator it = mTempVarIndices.begin(); it != mTempVarIndices.end(); it++)
		tempVarValues.push_back(mVars.Retrieve(*it));

	//store flow-ctrl state in case of recursion
	CommandList::iterator next = mNextCommand;
	CommandList::iterator begin = mSequenceBegin;
	CommandList::iterator end = mSequenceEnd;

	//store parameters
	Variables* oldParams = mParamsPtr;

	//set parameters current
	mParamsPtr = &params;
	mSequenceEnd = seq.end();
	mNextCommand = mSequenceBegin = seq.begin();
	while(mNextCommand != mSequenceEnd)
	{
		Instruction* pCmd = *mNextCommand;
		mNextCommand++;
		ExecuteCommand(pCmd);
	}

	//restore params
	mParamsPtr = oldParams;

	//restore flow-ctrl state in case of recursion
	mNextCommand = next;
	mSequenceBegin = begin;
	mSequenceEnd = end;

	//store and restore internal variables
	std::vector<double>::iterator storedValue = tempVarValues.begin();
	for(IndexList::iterator it = mTempVarIndices.begin(); it != mTempVarIndices.end(); it++)
		mVars.Data()[*it] = *storedValue++;
}

void Processor::ExecuteCommand(Instruction* pCmd)
{
	//mLog.push_back(pCmd->source);
	InstructionSet op = pCmd->GetOperation();
	switch(op)
	{
	case MOVE_OP:
		if(pCmd->HasToken(1))
			mSproutPtr->Move((float)pCmd->GetNumber(0), (float)pCmd->GetNumber(1)); 
		else
			mSproutPtr->Move((float)pCmd->GetNumber(0)); 
		break;
	case POSITION_OP:
		mSproutPtr->SetPosition((float)pCmd->GetNumber(0), (float)pCmd->GetNumber(1)); break;
	case ROTATE_OP:
		mSproutPtr->Rotate((float)pCmd->GetNumber(0)); break;
	case DIRECTION_OP:
		mSproutPtr->SetRotation((float)pCmd->GetNumber(0)); break;
	case AIM_OP:
		mSproutPtr->LookAt((float)pCmd->GetNumber(0), (float)pCmd->GetNumber(1)); break;
	case SIZE_OP:
		mSproutPtr->SetWidth((float)pCmd->GetNumber(0)); break;
	case ALPHA_OP:
		mSproutPtr->SetAlpha(pCmd->IsBoolean(0) ? (pCmd->GetBool(0) ? 1.0f : 0.0f) : (float)pCmd->GetNumber(0)); break;
	case COLOR_RGB_OP:
		mSproutPtr->SetColorRGB((float)pCmd->GetNumber(0), (float)pCmd->GetNumber(1), (float)pCmd->GetNumber(2)); break;
	case COLOR_HSV_OP:
		mSproutPtr->SetColorHSV((float)pCmd->GetNumber(0), (float)pCmd->GetNumber(1), (float)pCmd->GetNumber(2)); break;
	case SET_OP:
		mVars.Insert(pCmd->GetToken(0), pCmd->GetNumber(1)); break;
	case PUSH_OP:
		PushState(pCmd->GetToken(0, "default")); break;
	case POP_OP:
		PopState(pCmd->GetToken(0, "default")); break;
	case OUT_OP:
		Print(pCmd); break;
	case SEED_OP:
		Seed(pCmd); break;
	case GROW_OP:
		Grow(pCmd->GetToken(0), pCmd->GetToken(1)); break;
	case GATE_OP:
		Gate(pCmd->GetBool(0), pCmd->GetBlockDepth()); break;
	case BREAK_OP:
		Break(pCmd->GetBlockDepth()); break;
	case REPEAT_OP:
		Repeat(pCmd->GetBlockDepth()); break;
	case SRAND_OP:
		Shuffle((int)pCmd->GetNumber(0)); break;
	case EXE_OP:
		Run(pCmd);
		break;
	}
}

void Processor::Shuffle(unsigned long seed)
{	
	mNextRnd = (seed * 17312 + 2531011L) * 214013L;
}

void Processor::Seed(Instruction* pCmd)
{
	const std::string& structure = pCmd->GetToken(0);
	SymbolList& list = mStructures.Retrieve(structure);
	list.clear();
	ParseSymbolList(pCmd->GetToken(1), list);
	pCmd->ResolveParams(1, list);
}

void Processor::Run(Instruction* pCmd)
{
	SymbolList list;
	ParseSymbolList(pCmd->GetToken(0), list, true);
	pCmd->ResolveParams(0, list);
	ExecuteSymbols(list);
}

void Processor::Gate(bool condition, int depth)
{
	//when condition evaluates false, skip all commands of depth >= op's depth except other branch op's of the same depth
	if(condition)
		return;

	while(mNextCommand != mSequenceEnd)
	{
		int nextDepth = (*mNextCommand)->GetBlockDepth(); 
		if(nextDepth < depth || (nextDepth == depth && (*mNextCommand)->GetOperation() == GATE_OP))
			return;
		mNextCommand++;
	}
}

void Processor::Break(int depth)
{
	//skip all commands with depth >= op's depth
	while(mNextCommand != mSequenceEnd && (*mNextCommand)->GetBlockDepth() >= depth)
		mNextCommand++;
}

void Processor::Repeat(int depth)
{
	//seek first previous command with op's depth
	while(mNextCommand != mSequenceBegin && (*(mNextCommand-1))->GetBlockDepth() >= depth)
		mNextCommand--;
}

void Processor::Grow(const std::string& structure, const std::string& ruleSet)
{
	//activate rules in ruleset
	for(Productions::iterator ruleIt = mProductions.begin(); ruleIt != mProductions.end(); ruleIt++)
		(*ruleIt)->Active = (*ruleIt)->HasTag(ruleSet);

	//apply active rules on structure
	SymbolList& symbols = mStructures.Retrieve(structure);
	SymbolList::iterator it = symbols.begin();
	SymbolList::iterator end = symbols.end();
	while(it != end)
	{
		bool match = false;
		for(Productions::iterator ruleIt = mProductions.begin(); ruleIt != mProductions.end(); ruleIt++)
		{
			ProductionRule* pRule = *ruleIt;
			match = pRule->Active && pRule->Match(symbols, it);
			if(match)
			{
				if(!pRule->Commands().empty())
					ExecuteSequence(pRule->Commands(), pRule->Params());
				break;
			}
		}
		if(!match)
			it++;
	}
}

void Processor::PushState(const std::string& stackId)
{
	State* pState;
	if(mTrash.empty())
		pState = new State();
	else
	{
		pState = mTrash.top();
		mTrash.pop();
	}
	pState->NextRnd = mNextRnd;
	pState->Vars = mVars.Data();
	mSproutPtr->ToState(pState->Sprout);
	mStacks[stackId].push(pState);
}

void Processor::PopState(const std::string& stackId)
{
	State* pState = mStacks[stackId].top();
	mNextRnd = pState->NextRnd;
	mVars.Data() = pState->Vars;
	mSproutPtr->FromState(pState->Sprout);
	mTrash.push(pState);
	mStacks[stackId].pop();
}

void Processor::Print(Instruction* pCmd)
{
	std::stringstream ss;
	ss << "PRINT:";
	int i = -1;
	while(pCmd->HasToken(++i))
	{
		ss << pCmd->GetToken(i);
		try
		{
			if(pCmd->IsExpression(i))
				if(pCmd->IsBoolean(i))
					ss << " = " << (pCmd->GetBool(i) ? "true " : "false ");
				else
					ss << " = " << pCmd->GetNumber(i) << " ";
		}
		catch(Error e)
		{
			ss << "Error: " << e.desc;
		}
		catch(...) { };
	}
	mLog.push_back(ss.str());
}

void Processor::ParseSymbolList(const std::string& line, SymbolList& out_symbols, bool searchStructures)
{	
	//TODO: migrate all parsing into script reader? 
	//but when symbol lists can't contain structures how will structures be executed?
	std::string symbolName;
	int pos = -1;
	int last = line.size()-1;
	while(pos < last)
	{
		symbolName.clear();
		//skip leading whitespaces
		while(pos < last && ::isspace(line.at(pos+1)))
			pos++;
		//split at each occurance of ' ' outside of parantheses
		int parenDepth = 0;
		while(++pos <= last)
		{
			char c = line.at(pos);
			if(c == ' ' && parenDepth == 0)
				break; //split here
			if(c == '(')
				parenDepth++;
			else if(c == ')')
				parenDepth--;
			else if(parenDepth == 0)
				symbolName += c;
		}
		Symbol symbol;
		if(searchStructures && mStructures.Contains(symbolName))
		{
			symbol.Type = Symbol::STRUCTURE;
			symbol.Index = mStructures.IndexOf(symbolName);
		}
		else
		{
			symbol.Type = Symbol::SEQUENCE;
			symbol.Index = mSequences.IndexOf(symbolName);
		}
		out_symbols.push_back(symbol);
	}
}

ProductionRule* Processor::AppendProduction()
{
	ProductionRule* pRule = new ProductionRule(this);
	mProductions.push_back(pRule);
	return pRule;
}

Instruction* Processor::AppendCommand(const std::string& seqId, InstructionSet type, int blockDepth)
{
	Instruction* pCmd = new Instruction(type, blockDepth);
	mSequences.Retrieve(seqId).push_back(pCmd);
	return pCmd;
}

std::string Processor::GetTempVar()
{
	std::stringstream ss;
	ss << "_c" << mTempVar++;
	std::string varName = ss.str();
	mTempVarIndices.insert(GetVarIndex(varName));
	return varName;
}

void Processor::ReleaseTempVar()
{
	mTempVar--;
}

int Processor::GetVarIndex(const std::string& varName) 
{ 
	return mVars.IndexOf(varName); 
}

//IExecutionContext
double Processor::GetVar(int i) 
{ 
	return mVars.Retrieve(i); 
}

double Processor::GetParam(int i) 
{ 
	if(mParamsPtr->size() > (unsigned int)i)
		return mParamsPtr->at(i); 
	else
		return 0.0;
}
		
double Processor::GetTime()
{
	clock_t ticks = clock();
	return (double)ticks / CLOCKS_PER_SEC;
}

double Processor::GetRandom(double min, double max)
{
	mNextRnd = mNextRnd * 214013L + 2531011L;
    double rnd = ((unsigned int)(mNextRnd/65536) % 32768) / (double)(32768);
	return min + rnd * (max-min);
}

void Processor::Log(const std::string& msg)
{
	mLog.push_back(msg);
}

void Processor::Abort(const std::string& msg)
{
	mLog.push_back("ERROR: "+msg);
	mValid = false;
}

