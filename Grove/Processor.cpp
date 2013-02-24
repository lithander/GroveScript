#include "Weirwood.h"
#include "Processor.h"
#include "Sprout.h"
#include "Command.h"
#include "Expression.h"
#include "Keywords.h"
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
	while(!mTrash.empty())
	{
		delete mTrash.top();
		mTrash.pop();
	}

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

		for(Variables::iterator it = mVars.begin(); it != mVars.end(); it++)
			(*it) = 0;

		ExecuteSequence(mSequences[GetSequenceIndex(seqId)]);
		mSproutPtr->Flush();
	}
	catch(Error e)
	{
		Abort(e.desc);
	}
}

void Processor::ExecuteSymbols(SymbolList& symbols)
{
	for(SymbolList::iterator seqIt = symbols.begin(); seqIt != symbols.end(); seqIt++)
		if(*seqIt >= 0) //skip non-sequence symbols
			ExecuteSequence(mSequences[*seqIt]);
}

void Processor::ExecuteSequence(CommandList& seq)
{
	//TODO: store and restore internal variables

	//store flow-ctrl state in case of recursion
	CommandList::iterator next = mNextCommand;
	CommandList::iterator begin = mSequenceBegin;
	CommandList::iterator end = mSequenceEnd;
	
	mSequenceEnd = seq.end();
	mNextCommand = mSequenceBegin = seq.begin();
	while(mNextCommand != mSequenceEnd)
	{
		Processor::Command* pCmd = *mNextCommand;
		mNextCommand++;
		ExecuteCommand(pCmd);
	}
	//restore flow-ctrl state in case of recursion
	mNextCommand = next;
	mSequenceBegin = begin;
	mSequenceEnd = end;
}

void Processor::ExecuteCommand(Processor::Command* pCmd)
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
	case COLOR_RGB_OP:
		mSproutPtr->SetColorRGB((float)pCmd->GetNumber(0), (float)pCmd->GetNumber(1), (float)pCmd->GetNumber(2)); break;
	case COLOR_HSV_OP:
		mSproutPtr->SetColorHSV((float)pCmd->GetNumber(0), (float)pCmd->GetNumber(1), (float)pCmd->GetNumber(2)); break;
	case DIRECTION_OP:
		mSproutPtr->SetRotation((float)pCmd->GetNumber(0)); break;
	case SET_OP:
		SetVariable(pCmd->GetToken(0), pCmd->GetNumber(1)); break;
	case PUSH_OP:
		PushState(pCmd->GetToken(0, "default")); break;
	case POP_OP:
		PopState(pCmd->GetToken(0, "default")); break;
	case OUT_OP:
		Print(pCmd); break;
	case SEED_OP:
		Seed(pCmd->GetToken(0), pCmd->GetToken(1)); break;
	case GROW_OP:
		Grow(pCmd->GetToken(0), pCmd->GetToken(1)); break;
	case EXE_OP:
		Execute(pCmd->GetToken(0)); break;
	case GATE_OP:
		Gate(pCmd->GetBool(0), pCmd->GetBlockDepth()); break;
	case BREAK_OP:
		Break(pCmd->GetBlockDepth()); break;
	case REPEAT_OP:
		Repeat(pCmd->GetBlockDepth()); break;
	case SRAND_OP:
		srand((int)pCmd->GetNumber(0)); break;
	}
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

void Processor::Seed(const std::string& structure, const std::string& axiom)
{
	int index = GetStructureIndex(structure);
	mStructures[index].clear();
	FillSymbolList(axiom, mStructures[index]);
}

void Processor::Grow(const std::string& structure, const std::string& ruleSet)
{
	//activate rules in ruleset
	for(Productions::iterator ruleIt = mProductions.begin(); ruleIt != mProductions.end(); ruleIt++)
		ruleIt->Active = ruleIt->HasTag(ruleSet);

	//apply active rules on structure
	int index = GetStructureIndex(structure);
	SymbolList& symbols = mStructures[index];
	SymbolList::iterator it = symbols.begin();
	SymbolList::iterator end = symbols.end();
	while(it != end)
	{
		bool match = false;
		for(Productions::iterator ruleIt = mProductions.begin(); ruleIt != mProductions.end(); ruleIt++)
		{
			match = ruleIt->Active && ruleIt->Match(symbols, it);
			if(match)
				break;
		}
		if(!match)
			it++;
	}
}

void Processor::Execute(const std::string& name)
{
	IndexTable::const_iterator it = mSequenceIndexTable.find(name);
	if(it != mSequenceIndexTable.end())
		ExecuteSequence(mSequences[it->second]);
	else if((it = mStructureIndexTable.find(name)) != mStructureIndexTable.end())
		ExecuteSymbols(mStructures[it->second]);
	else
		throw Error("Neither Sequence nor Structure '"+name+"' is not defined!");
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
	pState->Vars = mVars;
	mSproutPtr->ToState(pState->Sprout);
	mStacks[stackId].push(pState);
}

void Processor::PopState(const std::string& stackId)
{
	State* pState = mStacks[stackId].top();
	mVars = pState->Vars;
	mSproutPtr->FromState(pState->Sprout);
	mTrash.push(pState);
	mStacks[stackId].pop();
}

void Processor::Print(Command* pCmd)
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
					ss << " = " << (pCmd->GetBool(i) ? "true" : "false");
				else
					ss << " = " << pCmd->GetNumber(i);
		}
		catch(Error e)
		{
			ss << "Error: " << e.desc;
		}
		catch(...) { };
	}
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

void Processor::FillSymbolList(const std::string& line, SymbolList& out_symbols)
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
	mProductions.push_back(ProductionRule());
	return &mProductions.back();
}

Processor::Command* Processor::AppendCommand(const std::string& seqId, InstructionSet type, int blockDepth)
{
	Processor::Command* pCmd = new Processor::Command(type, blockDepth);
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

int Processor::GetStructureIndex(const std::string& name)
{
	IndexTable::iterator it = mStructureIndexTable.find(name);
	if(it != mStructureIndexTable.end())
		return it->second;
	else
	{
		//insert
		int index = mStructures.size();
		mStructures.resize(index+1);
		mStructureIndexTable[name] = index; //eg last index after resize
		return index;
	}
}

void Processor::SetVariable(const std::string& name, double value)
{
	IndexTable::iterator it = mVarIndexTable.find(name);
	if(it != mVarIndexTable.end())
		mVars[it->second] = value;
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

