#include "Weirwood.h"
#include "ScriptReader.h"
#include "Command.h"
#include "Processor.h"
#include "Keywords.h"

using namespace Weirwood;

ScriptReader::ScriptReader(void) : mProcPtr(NULL), mLineNumber(-1), mBlockDepth(0), mGeneratingCode(false), mStreamPtr(NULL), mRulePtr(NULL)
{
}


ScriptReader::~ScriptReader(void)
{
}

void ScriptReader::Throw(std::string error) const
{
	std::stringstream ss;
	if(mLineNumber >= 1)
		ss << "Line " << mLineNumber << ": ";
	ss << error;
	throw Error(ss.str());
}

bool ScriptReader::Read(std::istream& input, Processor* pProc)
{
	mStreamPtr = &input;
	mProcPtr = pProc;
	mId = "Root";
	mLine = "";
	mCommentFlag = false;
	mLineNumber = 0;
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

void ScriptReader::ReadBlock(int depth)
{
	//User Code
	while(ReadLine() && mBlockDepth >= depth)
		ParseLine();
	if(!mLine.empty())
		RestoreLine();
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

//******************
// SCRIPT PARSING //
//******************

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
		mId = mLine.substr(1);
	//production
	else if(mLine.find("=>") != mLine.npos)
		ParseProductionRule();
	//commands
	else 
		ParseCommand();
}

void ScriptReader::ParseProductionRule()
{
	mRulePtr = mProcPtr->AppendProduction();
		
	int pos = -1;
	std::string tag;
	while(ParseParam(mId, pos, tag))
		mRulePtr->AddTag(tag);

	//condition
	int cBegin = mLine.find('[');
	int cEnd = mLine.find(']');
	if(cBegin != mLine.npos && cEnd != mLine.npos)
	{
		Expression cnd = Expression(mProcPtr);
		cnd.SetDebugInfo(mLineNumber);
		std::string condition = mLine.substr(cBegin+1, cEnd-1);
		mLine.erase(cBegin, cEnd+1);
		ParseExpression(condition, &cnd);
		mRulePtr->SetCondition(cnd);
	}

	int splitPosition = mLine.find("=>");
	std::string from = mLine.substr(0, splitPosition-1);
	std::string to = mLine.substr(splitPosition+2);
	mProcPtr->ParseSymbolList(from, mRulePtr->Predecessor());
	mProcPtr->ParseSymbolList(to, mRulePtr->Successor());

	int depth = mBlockDepth+1;
	ReadBlock(depth); //as long as mRulePtr is set, commands will get appended there
	mRulePtr = NULL;
}

void ScriptReader::ParseCommand()
{
	ReadToken();
	//Operation?
	InstructionSet op = Keywords::Operation(mToken);
	if(op != NO_OP)
	{
		Instruction* pCmd = GenerateCommand(op, mBlockDepth);
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
		case WHILE_MC:
			GenerateWhile(); break;
		case UNTIL_MC:
			GenerateUntil(); break;
		case IF_MC:
			GenerateIf(); break;
		case ELSE_MC:
			GenerateElse(); break;
		case RAISE_MC:
			GenerateMod('+'); break;
		case LOWER_MC:
			GenerateMod('-'); break;
		default:
			Throw("Command not known!");
	}
}

bool ScriptReader::ParseParam(const std::string& token, int& inout_pos, std::string& out)
{
	out.clear();
	int last = token.size()-1;
	//skip leading whitespaces
	while(inout_pos < last && ::isspace(token.at(inout_pos+1)))
		inout_pos++;
	//split at each occurance of ',' outside of parantheses
	int parenDepth = 0;
	while(++inout_pos <= last)
	{
		char c = token.at(inout_pos);
		if(c == ',' && parenDepth == 0)
			break; //split here
		if(c == '(')
			parenDepth++;
		else if(c == ')')
			parenDepth--;
		
		out.push_back(c);
	}
	//Check if parenthesis were nested okay
	if(parenDepth != 0)
	{
		std::stringstream ss;
		ss << "Parentheses are messed up in Line " << mLineNumber;
		throw Error(ss.str());
	}
	return out.length() > 0;
}

void ScriptReader::ParseExpression(const std::string& token, Expression* out)
{
	//detect string-literal
	if(token.find('\'') != token.npos)
		return;
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

//*****************************
// COMMAND GENERATION MACROS //
//*****************************

void ScriptReader::GenerateWhile()
{
	if(!ReadParam())
		Throw("'While' expects condition!");
	int depth = mBlockDepth+1;
	GenerateCommand(GATE_OP, mParamToken, depth);
	ReadBlock(depth);
	GenerateCommand(REPEAT_OP, depth);
	GenerateCommand(NO_OP, depth-1);
}

void ScriptReader::GenerateUntil()
{
	if(!ReadParam())
		Throw("'Until' expects condition!");
	int depth = mBlockDepth+1;
	GenerateCommand(GATE_OP, "not ("+mParamToken+")", depth);
	ReadBlock(depth);
	GenerateCommand(REPEAT_OP, depth);
	GenerateCommand(NO_OP, depth-1);
}

void ScriptReader::GenerateMod(char mod)
{	
	if(!ReadParam())
		Throw("'Raise' expects variable name!");
	std::string varName = mParamToken;
	if(ReadParam())
		GenerateCommand(SET_OP, varName +", "+varName+mod+"("+mParamToken+")", mBlockDepth);
	else
		GenerateCommand(SET_OP, varName +", "+varName+mod+"1", mBlockDepth);
}


void ScriptReader::GenerateIf()
{
	if(!ReadParam())
		Throw("'If' expects condition!");
	int depth = mBlockDepth+1;
	GenerateCommand(GATE_OP, mParamToken, depth);
	ReadBlock(depth);
	GenerateCommand(BREAK_OP, depth);
}

void ScriptReader::GenerateElse()
{
	int depth = mBlockDepth+1;
	GenerateCommand(GATE_OP, "true", depth);
	if(mPos != mLine.npos)
	{
		int nextPos = mLine.find(' ', mPos+1);
		std::string remains = mLine.substr(mPos+1, nextPos-mPos-1);
		if(!remains.empty() && Keywords::Macro(remains) != IF_MC)
			Throw("'Else' can only be followed by 'If'!");
		mPos = nextPos;
		GenerateIf();
	}
}

void ScriptReader::GenerateRepeat()
{
	if(!ReadParam())
		Throw("'Repeat' expects iteration number!");
	std::string varName = mProcPtr->GetTempVar();
	GenerateCommand(SET_OP, varName+", "+mParamToken, mBlockDepth);//set _c0, expression
	int depth = mBlockDepth+1;
	GenerateCommand(GATE_OP, varName+"> 0", depth);				   //  cnd _c0 > 0"
	ReadBlock(depth);
	GenerateCommand(SET_OP, varName+","+varName+"-1", depth);	//  set _c0, _c0-1
	GenerateCommand(REPEAT_OP, depth);							//  repeat
	GenerateCommand(NO_OP, depth-1);							// block end
	mProcPtr->ReleaseTempVar();
}

Instruction* ScriptReader::GenerateCommand(InstructionSet op, int depth)
{
	if(mRulePtr)
		return mRulePtr->AppendCommand(op, depth);
	else
		return mProcPtr->AppendCommand(mId, op, depth); 
}		

void ScriptReader::GenerateCommand(InstructionSet op, const std::string& params, int depth)
{
	mGeneratingCode = true;
	Instruction* pCmd = GenerateCommand(op, depth); 
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

