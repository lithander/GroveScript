#include "Weirwood.h"
#include "ScriptReader.h"
#include "Command.h"
#include "Processor.h"
#include "Keywords.h"

using namespace Weirwood;

ScriptReader::ScriptReader(void) : mProcPtr(NULL), mLineNumber(-1), mBlockDepth(0), mGeneratingCode(false), mStreamPtr(NULL)
{
}


ScriptReader::~ScriptReader(void)
{
}

bool ScriptReader::Parse(std::istream& input, Processor* pProc)
{
	mStreamPtr = &input;
	mProcPtr = pProc;
	mSeqId = "Root";
	mLine = "";
	mCommentFlag = false;
	mLineNumber = 0;
	mTempVar = 0;
	try
	{
		while(ReadLine())
			ParseLine();
	}
	catch(Error e)
	{
		mProcPtr->Abort(e.desc);
	}
	return true;
}

bool ScriptReader::ReadLine()
{
	if(mStoredLine != "")
	{
		mLine = mStoredLine;
		mStoredLine = "";
		mBlockDepth = mLine.find_first_not_of(char(9)); //count TAB characters
		return true;
	}
	while(mStreamPtr->good() && !mStreamPtr->eof())
	{
		mLineNumber++;
		std::getline(*mStreamPtr, mLine);
		mBlockDepth = mLine.find_first_not_of(char(9)); //count TAB characters
		//skip if no non-tab characters found
		if(mBlockDepth != mLine.npos)
			return true;
	}
	mLine = "";
	mBlockDepth = -1;
	return false;
}

bool ScriptReader::ReadToken()
{
	mPos = mLine.find(' ', 0);
	mToken = mLine.substr(0, mPos);
	return !mToken.empty();
}

bool ScriptReader::ReadParam()
{
	int end = mLine.size();
	if(mPos >= end)
		return false;
	if(mPos == mLine.npos)
		return false;
	return ParseParam(mLine, mPos, mParamToken);
}

void ScriptReader::ParseLine()
{
	//remove tabs before parsing
	mLine.erase(0, mBlockDepth);
			
	//one-line comments
	if(mLine.find("//") == 0)
		return;
	//comment-block
	if(mLine.find("/*") == 0)
		mCommentFlag = true;
	else if(mLine.find("*/") != mLine.npos)
	{
		mCommentFlag = false;
		return;
	}

	//skip if comment
	if(mCommentFlag)
		return;
			
	//sequence
	if(mLine.find('#') == 0)
		mSeqId = mLine.substr(1);
	//production
	else if(mLine.find("=>") != mLine.npos)
		ParseProductionRule();
	//commands
	else 
		ParseCommand();
}

void ScriptReader::ParseProductionRule()
{
	ProductionRule* pRule = mProcPtr->AppendProduction();

	int splitPosition = mLine.find("=>", 0);
	std::string from = mLine.substr(0, splitPosition);
	std::string to = mLine.substr(splitPosition+2);
	
	pRule->Predecessor().clear();
	mProcPtr->FillSymbolList(from, pRule->Predecessor());
	pRule->Successor().clear();
	mProcPtr->FillSymbolList(to, pRule->Successor());
}

void ScriptReader::ParseCommand()
{
	ReadToken();
	//Operation?
	InstructionSet op = Keywords::Operation(mToken);
	if(op != NO_OP)
	{
		Processor::Command* pCmd = mProcPtr->AppendCommand(mSeqId, op, mBlockDepth); 
		pCmd->SetDebugInfo(mLineNumber);
		while(ReadParam())
		{
			Expression exp(mProcPtr);
			exp.SetDebugInfo(mLineNumber);
			ParseExpression(mParamToken, &exp);
			pCmd->PushParam(mParamToken, exp); //exp will be copied. It's 64 bytes so it should be okay. (std::string is 32byte in comparision)
		}
		return;
	}
	//Macro?
	Macros mc = Keywords::Macro(mToken);
	switch(mc)
	{
		case REPEAT_MC:
			GenerateRepeat(); break;
		default:
			std::stringstream ss;
			ss << "Command at Line " << mLineNumber << " is not understood!";
			throw Error(ss.str());
	}
}

void ScriptReader::GenerateRepeat()
{
	if(!ReadParam())
	{
		std::stringstream ss;
		ss << "Macro Repeat in Line " << mLineNumber << " requires iteration count!";
		throw Error(ss.str());
	}
	std::string varName = GetTempVar();
	GenerateCommand(SET_OP, varName+", "+mParamToken, mBlockDepth);//set _c0, expression
	int depth = mBlockDepth+1;
	GenerateCommand(GATE_OP, varName+"> 0", depth);				//  cnd _c0 > 0"
	//User Code
	while(ReadLine() && mBlockDepth >= depth)
		ParseLine();
	if(!mLine.empty())
		RestoreLine();
	GenerateCommand(SET_OP, varName+","+varName+"-1", depth);		//  set _c0, _c0-1
	GenerateCommand(REPEAT_OP, depth);							//  repeat
	ReleaseTempVar();
}

void ScriptReader::GenerateCommand(InstructionSet op, int depth)
{
	mProcPtr->AppendCommand(mSeqId, op, depth);
}		

void ScriptReader::GenerateCommand(InstructionSet op, const std::string& params, int depth)
{
	mGeneratingCode = true;
	Processor::Command* pCmd = mProcPtr->AppendCommand(mSeqId, op, depth); 
	int pos = -1;
	std::string param;
	while(ParseParam(params, pos, param))
	{
		Expression exp(mProcPtr);
		ParseExpression(param, &exp);
		pCmd->PushParam(param, exp); //exp will be copied. It's 64 bytes so it should be okay. (std::string is 32byte in comparision)
	}
	mGeneratingCode = false;
}

std::string ScriptReader::GetTempVar()
{
	std::stringstream ss;
	ss << "_c" << mTempVar++;
	return ss.str();
}

void ScriptReader::ReleaseTempVar()
{
	mTempVar--;
}

bool ScriptReader::ParseParam(const std::string& token, int& inout_pos, std::string& out)
{
	int end = token.size();
	if(inout_pos >= end)
		return false;
	//split at each occurance of ',' outside of parantheses
	int nextPos = inout_pos;
	int parenDepth = 0;
	while(++nextPos < end)
	{
		char c = token.at(nextPos);
		if(c == '(')
			parenDepth++;
		else if(c == ')')
			parenDepth--;
		else if(c == ',' && parenDepth == 0)
			break; //split here
				//Create Expression
	}
	//Check if parenthesis were nested okay
	if(parenDepth != 0)
	{
		std::stringstream ss;
		ss << "Parentheses are messed up in Line " << mLineNumber;
		throw Error(ss.str());
	}
	out = token.substr(inout_pos+1, nextPos-inout_pos-1);
	inout_pos = nextPos;
	return true;
}

void ScriptReader::ParseExpression(const std::string& token, Expression* out)
{
	//PARSE
	std::stringstream stream = std::stringstream(token);
	//skip blanks
	stream >> std::skipws;
	bool done = false;
	for(;;)
	{
		char next = 0;
		char ch = 0;
		stream >> ch;
		switch (ch)
		{
			case 0 :
				out->PushToken(Expression::END);
				return;
			//operator
			case '+' :
				out->PushToken(Expression::OP_PLUS); break;
			case '-' :
				out->PushToken(Expression::OP_MINUS); break;
			case '*' :
				out->PushToken(Expression::OP_MUL); break;
			case '/' :
				out->PushToken(Expression::OP_DIV); break;
			case '%' :
				out->PushToken(Expression::OP_MOD); break;
			case '^' :
				out->PushToken(Expression::OP_POWER); break;
			case '(' :
				out->PushToken(Expression::LP); break;
			case ')' :
				out->PushToken(Expression::RP); break;
			case ',' :
				out->PushToken(Expression::DELIMITER); break;
			case '=' :
				out->PushToken(Expression::LG_EQL); break;
			case '<' :
				//NEQL, LESS or LESS_EQL
				next = 0;
				stream.get(next);
				if(next == '>')
					out->PushToken(Expression::LG_NEQL);
				else if(next == '=')
					out->PushToken(Expression::LG_LESS_EQL);
				else
					out->PushToken(Expression::LG_LESS);
				break;
			case '>' :
				//GREATER or GREATER_EQL
				next = 0;
				stream.get(next);
				if(next == '=')
					out->PushToken(Expression::LG_GREATER_EQL);
				else
					out->PushToken(Expression::LG_GREATER);
				break;
			//number
			case '0' :	case '1' :	case '2' :	case '3' : 	case '4' :
			case '5' :	case '6' :	case '7' :	case '8' :	case '9' :
			case '.' :
				stream.putback(ch);
				double value;
				stream >> value;
				out->PushNumber(value);
				break;
			//string token (function or variable)
			default:
				if (!mGeneratingCode && !isalpha(ch)) //tolerate prefixed variables for generated code
				{
					std::stringstream ss;
					ss << "Symbol '" << ch << "' in Line " << mLineNumber << " is not understood!";
					throw Error(ss.str());
				}
				//read all alphanumeric characters
				std::string token;
				token+=ch;
				while (stream.get(ch) && isalnum(ch)) 
					token+=ch;
				//if ch is '(' token specifies a function
				if(ch == '(')
					out->PushFunction(Keywords::Function(token)); //the LP isn't needed in the token chain
				else //token specifies a variable
				{
					Expression::TokenType tkn = Keywords::Token(token);
					if(tkn != Expression::END)
						out->PushToken(tkn);
					else
						out->PushVariable(mProcPtr->GetVarIndex(token));
					//ch belongs to the next token
					stream.putback(ch);
				}
				break;
		}
	}
}

