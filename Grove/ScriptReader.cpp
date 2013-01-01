#include "Weirwood.h"
#include "ScriptReader.h"
#include "Command.h"
#include "Processor.h"
#include "Keywords.h"

using namespace Weirwood;

ScriptReader::ScriptReader(void) : mProcPtr(NULL)
{
}


ScriptReader::~ScriptReader(void)
{
}

bool ScriptReader::Parse(std::istream& input, Processor* pProc)
{
	mProcPtr = pProc;
	std::string id = "Root";
	std::string line;
	int lineNumber = 0;
	try
	{
		while(input.good() && !input.eof())
		{
			lineNumber++;
			std::getline(input, line);
			if(line == "" || line.find("//") == 0)
				continue;

			if(line.find('#') == 0)
			{
				id = line.substr(1);
			}
			else if(line.find("=>") != line.npos)
			{
				ProductionRule* pRule = pProc->AppendProduction();
				ParseProductionRule(line, pRule);
			}
			else
			{
				Processor::Command* pCmd = pProc->AppendCommand(id); 
				ParseCommand(line, lineNumber, pCmd);
			}
		}
	}
	catch(Error e)
	{
		pProc->Abort(e.desc);
	}
	return true;
}

void ScriptReader::ParseProductionRule(const std::string& line, ProductionRule* out)
{
	int splitPosition = line.find("=>", 0);
	std::string from = line.substr(0, splitPosition);
	std::string to = line.substr(splitPosition+2);
	
	out->Predecessor().clear();
	mProcPtr->FillSymbolList(from, out->Predecessor());
	out->Successor().clear();
	mProcPtr->FillSymbolList(to, out->Successor());
}

void ScriptReader::ParseCommand(const std::string& line, int lineNr, Processor::Command* out)
{
	out->SetDebugInfo(lineNr);

	//Parse Operation
	int end = line.size();
	int pos = line.find(' ', 0);
	InstructionSet op = Keywords::Operation(line.substr(0, pos));
	out->SetOperation(op);
	if(pos == line.npos)
		return;

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
				break;
		}

		//Create Expression
		std::string token = line.substr(pos+1, nextPos-pos-1);
		Expression exp(mProcPtr);
		ParseExpression(token, lineNr, &exp);
		out->PushParam(token, exp); //exp will be copied. too costly?
		pos = nextPos;
	}
	while(pos != end);

	//Check if parenthesis were nested okay
	if(parenDepth != 0)
	{
		std::stringstream ss;
		ss << "Parentheses are messed up at Line " << lineNr;
		throw Error(ss.str());
	}
}

void ScriptReader::ParseExpression(const std::string& line, int lineNumber, Expression* out)
{
	out->SetDebugInfo(lineNumber);
	//PARSE
	std::stringstream stream = std::stringstream(line);
	//skip blanks
	stream >> std::skipws;
	bool done = false;
	for(;;)
	{
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
				if (!isalpha(ch))
				{
					std::string badSymbol;
					badSymbol+=ch;

					std::stringstream ss;
					ss << "Symbol at Line " << lineNumber << " is not understood!";
					throw Error(ss.str());
				}
				//read all alphanumeric characters
				std::string token;
				token+=ch;
				while (stream.get(ch) && isalpha(ch)) 
					token+=ch;
				//if ch is '(' token specifies a function
				if(ch == '(')
					out->PushFunction(Keywords::Function(token)); //the LP isn't needed in the token chain
				else //token specifies a variable
				{
					out->PushVariable(mProcPtr->GetVarIndex(token));
					//ch belongs to the next token
					stream.putback(ch);
				}
				break;
		}
	}
}

