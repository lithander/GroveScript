#include "Weirwood.h"
#include "ScriptReader.h"
#include "Command.h"
#include "Processor.h"
#include "Keywords.h"

using namespace Weirwood;

ScriptReader::ScriptReader(void) : mProcPtr(NULL), mLineNumber(-1), mBlockDepth(0), mGeneratingCode(false)
{
}


ScriptReader::~ScriptReader(void)
{
}

bool ScriptReader::Parse(std::istream& input, Processor* pProc)
{
	mProcPtr = pProc;
	mSeqId = "Root";
	mLine = "";
	mLineNumber = 0;
	bool isComment = false;
	try
	{
		while(input.good() && !input.eof())
		{
			mLineNumber++;
			std::getline(input, mLine);
			mBlockDepth = mLine.find_first_not_of(char(9)); //count TAB characters
			//skip if no non-tab characters found
			if(mBlockDepth == mLine.npos)
				continue;

			//remove tabs before parsing
			mLine.erase(0, mBlockDepth);
			
			//one-line comments
			if(mLine.find("//") == 0)
				continue;
			//comment-block
			if(mLine.find("/*") == 0)
				isComment = true;
			else if(mLine.find("*/") != mLine.npos)
			{
				isComment = false;
				continue;
			}

			//skip if comment
			if(isComment)
				continue;
			
			//sequence
			if(mLine.find('#') == 0)
			{
				mSeqId = mLine.substr(1);
			}
			//production
			else if(mLine.find("=>") != mLine.npos)
			{
				ProductionRule* pRule = pProc->AppendProduction();
				ParseProductionRule(pRule);
			}
			//commands
			else
			{
				if(!ParseCommand())
					ParseMacro();
			}
		}
	}
	catch(Error e)
	{
		mProcPtr->Abort(e.desc);
	}
	return true;
}

bool ScriptReader::ParseCommand()
{
	int pos = mLine.find(' ', 0);
	InstructionSet op = Keywords::Operation(mLine.substr(0, pos));
	if(op == NO_OP)
		return false;

	Processor::Command* pCmd = mProcPtr->AppendCommand(mSeqId, op, mBlockDepth); 
	pCmd->SetDebugInfo(mLineNumber);
	if(pos != mLine.npos)
		ParseParams(pCmd, mLine, pos);
	
	return true;
}

void ScriptReader::GenerateCommand(InstructionSet op, const std::string& params)
{
	mGeneratingCode = true;
	Processor::Command* pCmd = mProcPtr->AppendCommand(mSeqId, op, mBlockDepth); 
	if(params != "")
		ParseParams(pCmd, params);
	mGeneratingCode = false;
}

void ScriptReader::ParseMacro()
{
	int pos = mLine.find(' ', 0);
	std::string token = mLine.substr(0, pos);
	boost::to_upper(token);
	if(token == "REPEAT")
	{
		std::string varName = "_c0";
		GenerateCommand(SET_OP, varName+", "+mLine.substr(pos));//set _c0, expression
		mBlockDepth++;
		GenerateCommand(GATE_OP, varName+"> 0");				//  cnd _c0 > 0"
		GenerateCommand(OUT_OP, varName);						//  USER CODE
		GenerateCommand(SET_OP, varName+","+varName+"-1");		//  set _c0, _c0-1
		GenerateCommand(REPEAT_OP, "");							//  repeat
		mBlockDepth--;
	}
	else
	{
		std::stringstream ss;
		ss << "Line " << mLineNumber << " is not understood!";
		throw Error(ss.str());
	}
}

void ScriptReader::ParseProductionRule(ProductionRule* out)
{
	int splitPosition = mLine.find("=>", 0);
	std::string from = mLine.substr(0, splitPosition);
	std::string to = mLine.substr(splitPosition+2);
	
	out->Predecessor().clear();
	mProcPtr->FillSymbolList(from, out->Predecessor());
	out->Successor().clear();
	mProcPtr->FillSymbolList(to, out->Successor());
}

void ScriptReader::ParseParams(Processor::Command* out, const std::string& line, int pos)
{
	int end = line.size();
	//Parse Paramters
	int parenDepth = 0;
	do
	{
		//split at each occurance of ',' outside of parantheses
		int nextPos = pos;
		while(++nextPos < end)
		{
			char c = line.at(nextPos);
			if(c == '(')
				parenDepth++;
			else if(c == ')')
				parenDepth--;
			else if(c == ',' && parenDepth == 0)
				break; //split here
		}

		//Create Expression
		std::string token = line.substr(pos+1, nextPos-pos-1);
		Expression exp(mProcPtr);
		exp.SetDebugInfo(mLineNumber);
		ParseExpression(token, &exp);
		out->PushParam(token, exp); //exp will be copied. It's 64 bytes so it should be okay. (std::string is 32byte in comparision)
		pos = nextPos;
	}
	while(pos != end);

	//Check if parenthesis were nested okay
	if(parenDepth != 0)
	{
		std::stringstream ss;
		ss << "Parentheses are messed up in Line " << mLineNumber;
		throw Error(ss.str());
	}
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

