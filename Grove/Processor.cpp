#include "Weirwood.h"
#include "Processor.h"
#include "Sprout.h"
#include "Command.h"
#include "Expression.h"
#include <ctime>

using namespace Weirwood;

Processor::Processor(Sprout* sproutPtr)
{
	mSproutPtr = sproutPtr;
}

Processor::~Processor(void)
{
	ClearStacks();
}

void Processor::Reset()
{
	ClearStacks();

	mValid = true;
	mLog.clear();
	mSproutPtr->Reset();

	mVars.clear();
	mProductions.clear();
	mSequences.clear();

	//index tables
	mSequenceIndexTable.clear();
	mSymbolIndexTable.clear();
	mVarIndexTable.clear();

	//TODO: delete Commands from Sequences
}

void Processor::ClearStacks()
{
	for(StateStackTable::iterator it = mStacks.begin(); it != mStacks.end(); it++)
		while(it->second.size() > 0)
		{
			State* pState = it->second.top();
			delete pState;
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

		for(Variables::iterator it = mVars.begin(); it != mVars.end(); it++)
			(*it) = 0;

		CommandList& seq = mSequences[GetSequenceIndex(seqId)];
		for(CommandList::iterator it = seq.begin(); it != seq.end(); it++)
			Execute(*it);

		mSproutPtr->Flush();
	}
	catch(Error e)
	{
		Abort(e.desc);
	}
}

void Processor::Execute(SymbolList& symbols)
{
	//run
	for(SymbolList::iterator seqIt = symbols.begin(); seqIt != symbols.end(); seqIt++)
		if(*seqIt >= 0) //skip non-sequence symbols
		{
			CommandList& seq = mSequences[*seqIt];
			for(CommandList::iterator it = seq.begin(); it != seq.end(); it++)
				Execute(*it);
		}
}


InstructionSet Processor::GetOperationType(const std::string& opOperation)
{
	//TODO: Use Keyword-class to map string to enumeration member. Resolve operation based on that
	std::string op = boost::to_upper_copy(opOperation);
	if(op == "MOV")
		return MOVE_OP;
	else if(op == "ROT")
		return ROTATE_OP;
	else if(op == "SZE")
		return SIZE_OP;
	else if(op == "POS")
		return POSITION_OP;
	else if(op == "DIR")
		return DIRECTION_OP;
	else if(op == "SET")
		return SET_OP;
	else if(op == "PSH")
		return PUSH_OP;
	else if(op == "POP")
		return POP_OP;
	else if(op == "OUT")
		return OUT_OP;
	else if(op == "GRW")
		return GROW_OP;
	else
		return NO_OP;
}

void Processor::Execute(Processor::Command* pCmd)
{
	//mLog.push_back(pCmd->source);
	InstructionSet op = pCmd->GetOperation();
	switch(op)
	{
	case MOVE_OP:
		mSproutPtr->Move((float)pCmd->GetNumber(0)); break;
	case ROTATE_OP:
		mSproutPtr->Rotate((float)pCmd->GetNumber(0)); break;
	case SIZE_OP:
		mSproutPtr->SetWidth((float)pCmd->GetNumber(0)); break;
	case POSITION_OP:
		mSproutPtr->SetPosition((float)pCmd->GetNumber(0), (float)pCmd->GetNumber(1)); break;
	case DIRECTION_OP:
		mSproutPtr->SetRotation((float)pCmd->GetNumber(0)); break;
	case SET_OP:
		SetVariable(pCmd->GetToken(0), pCmd->GetNumber(1)); break;
	case PUSH_OP:
		PushState(pCmd->GetToken(0)); break;
	case POP_OP:
		PopState(pCmd->GetToken(0)); break;
	case OUT_OP:
		Print(pCmd->GetToken(0)); break;
	case GROW_OP:
		Grow(pCmd->GetToken(0), (int)pCmd->GetNumber(1)); break;
	}
}

void Processor::Grow(const std::string& line, int iterations)
{
	SymbolList* pSymbols = NULL;
	if(!mGrowSymbols.empty())
	{
		pSymbols = mGrowSymbols.top();
		mGrowSymbols.pop();
	}
	else 
		pSymbols = new SymbolList();

	pSymbols->clear();
	ParseSymbolList(line, *pSymbols);
	Grow(*pSymbols, iterations);
	Execute(*pSymbols);
	mGrowSymbols.push(pSymbols);
}

void Processor::Grow(SymbolList& symbols, int iterations)
{
	if(iterations <= 0)
		return;

	SymbolList::iterator it = symbols.begin();
	while(it != symbols.end())
	{
		bool match = false;
		for(Productions::iterator ruleIt = mProductions.begin(); ruleIt != mProductions.end(); ruleIt++)
		{
			match = ruleIt->Match(symbols, it);
			if(match)
				break;
		}
		if(!match)
			it++;
	}
	Grow(symbols, --iterations);
}

void Processor::PushState(const std::string& stackId)
{
	//TODO: instead of new/delete cache and reuse states
	State* pState = new State();
	pState->Vars = mVars;
	mSproutPtr->ToState(pState->Sprout);
	mStacks[stackId].push(pState);
}

void Processor::PopState(const std::string& stackId)
{
	//TODO: instead of new/delete cache and reuse states
	State* pState = mStacks[stackId].top();
	mVars = pState->Vars;
	mSproutPtr->FromState(pState->Sprout);
	delete pState;
	mStacks[stackId].pop();
}

void Processor::Print(const std::string& token)
{
	std::stringstream ss;
	ss << "PRINT: " << token << " = ";
	try
	{
		Expression ex(this);
		ex.Parse(token);
		double result = ex.Evaluate();
		ss << result;
	}
	catch(Error e)
	{
		ss << "Error: " << e.desc;
	}
	catch(...) { };
	mLog.push_back(ss.str());
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

void Processor::ParseSymbolList(const std::string& line, SymbolList& out_symbols)
{
	//TODO: migrate all parsing into script reader?
	int pos = 0;
	for(;;)
	{
		int nextPos = line.find(' ', pos);
		const std::string s = line.substr(pos, nextPos-pos);
		if(s != "")
			out_symbols.push_back(GetSymbolIndex(s));
		
		if(nextPos == line.npos)
			return;

		pos = nextPos+1;
	}
}

int Processor::GetSymbolIndex(const std::string& name)
{
	IndexTable::iterator it = mSymbolIndexTable.find(name);
	if(it != mSymbolIndexTable.end())
		return it->second;
	else
	{
		IndexTable::iterator it = mSequenceIndexTable.find(name);
		if(it != mSequenceIndexTable.end())
		{
			//Symbol is a Sequence: return sequence index and cache it for next query
			int index = it->second;
			mSymbolIndexTable[name] = index;
			return index;
		}
		//Symbol is new - return unoccupied negative number and cache for next query
		int index = -(1+(int)mSymbolIndexTable.size());
		mSymbolIndexTable[name] = index; //eg last index after resize
		return index;
	}
}

ProductionRule* Processor::AppendProduction()
{
	mProductions.push_back(ProductionRule(this));
	return &mProductions.back();
}

Processor::Command* Processor::AppendCommand(const std::string& seqId)
{
	Processor::Command* pCmd = new Processor::Command(this);
	mSequences[GetSequenceIndex(seqId)].push_back(pCmd);
	return pCmd;
}

int Processor::GetSequenceIndex(const std::string& name)
{
	IndexTable::iterator it = mSequenceIndexTable.find(name);
	if(it != mSequenceIndexTable.end())
		return it->second;
	else
	{
		//insert
		int index = mSequences.size();
		mSequences.resize(index+1);
		mSequenceIndexTable[name] = index; //eg last index after resize
		return index;
	}
}

void Processor::SetVariable(const std::string& name, double value)
{
	IndexTable::iterator it = mVarIndexTable.find(name);
	if(it != mVarIndexTable.end())
	{
		int index = it->second;
		mVars[index] = value;
	}
	else
	{
		//insert
		mVars.push_back(value);
		mVarIndexTable[name] = mVars.size() -1; //eg last index after resize
	}
}

int Processor::GetVarIndex(const std::string& name)
{
	IndexTable::iterator it = mVarIndexTable.find(name);
	if(it != mVarIndexTable.end())
		return it->second;
	else
	{
		//insert
		int index = mVars.size();
		mVars.resize(index+1);
		mVarIndexTable[name] = index;
		return index;
	}
}
		
double Processor::GetTime()
{
	clock_t ticks = clock();
	return (double)ticks / CLOCKS_PER_SEC;
}

