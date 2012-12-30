#include "Weirwood.h"
#include "Expression.h"
#include <cmath>

using namespace Weirwood;

Expression::Expression(IExpressionContext* pContext) : mContextPtr(pContext), mLineNumber(-1)
{
}

Expression::~Expression(void)
{
}

void Expression::Throw(std::string error)
{
	std::stringstream ss;
	if(mLineNumber >= 1)
		ss << "Line " << mLineNumber << ": ";
	ss << error;
	throw Error(ss.str());
}

void Expression::Parse(const std::string& line, int lineNumber)
{
	mLineNumber = lineNumber;
	//RESET
	mTokens.clear();
	mVars.clear();
	mValues.clear();
	mFunctions.clear();
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
				mTokens.push_back(END);
				return;
			//operator
			case '+' :
				mTokens.push_back(OP_PLUS); break;
			case '-' :
				mTokens.push_back(OP_MINUS); break;
			case '*' :
				mTokens.push_back(OP_MUL); break;
			case '/' :
				mTokens.push_back(OP_DIV); break;
			case '%' :
				mTokens.push_back(OP_MOD); break;
			case '^' :
				mTokens.push_back(OP_POWER); break;
			case '(' :
				mTokens.push_back(LP); break;
			case ')' :
				mTokens.push_back(RP); break;
			//number
			case '0' :	case '1' :	case '2' :	case '3' : 	case '4' :
			case '5' :	case '6' :	case '7' :	case '8' :	case '9' :
			case '.' :
				stream.putback(ch);
				double value;
				stream >> value;
				mValues.push_back(value);
				mTokens.push_back(NUMBER);
				break;
			//string token
			default:
				if (!isalpha(ch))
				{
					std::string badSymbol;
					badSymbol+=ch;
					Throw("Symbol '"+badSymbol+"' is not understood.");
				}
				//read all alphanumeric characters
				std::string token;
				token+=ch;
				while (stream.get(ch) && isalpha(ch)) 
					token+=ch;
				//if ch is '(' token specifies a function
				if(ch == '(')
				{
					mFunctions.push_back(GetFunctionType(token));
					mTokens.push_back(FUNCTION);
					//the LP isn't needed in the token chain
				}
				else //token specifies a variable
				{
					mVars.push_back(mContextPtr->GetVarIndex(token));
					mTokens.push_back(VAR);
					//ch belongs to the next token
					stream.putback(ch);
				}
				break;
		}
	}
}

Expression::FunctionType Expression::GetFunctionType(const std::string& fnToken)
{
	//only used when parsing expression, performance shoulnd't be critical
	//TODO: Use Keyword-class to map string to enumeration member. Resolve function based on that
	std::string fn = boost::to_upper_copy(fnToken);
	if(fn == "TIME")
		return FN_TIME;
	else if(fn == "SIN")
		return FN_SIN;
	else if(fn == "COS")
		return FN_COS;

	Throw("Function '"+fnToken+"' is not understood!");
	return FN_VOID;
}

double Expression::Evaluate()
{
	mVarsPtr = mContextPtr->GetVars();
	mTokenIndex = 0;
	mValueIndex = 0;
	mVarIndex = 0;
	mFunctionIndex = 0;
	return EvalP1();
}

double Expression::EvalP1()
{
	//Addition/Subtraction
	double left = EvalP2();      
	while(true)
		switch (mType)
		{
			case OP_PLUS :
				left += EvalP2();
				break;
			case OP_MINUS :
				left -= EvalP2();
				break;
			default :
				return left;
		}
}

double Expression::EvalP2()
{
	//Multiplication/Division/Modulo
	double left = EvalP3();
	while(true)
		switch (mType)
		{
			case OP_MOD:
				left = fmod(left, EvalP3());
				break;
			case OP_MUL :
				left *= EvalP3();
				break;
			case OP_DIV :
				if (double d = EvalP3())
					left /= d;
				else
					Throw("Division by Zero");
				break;
			default :
				return left;
		}
}

double Expression::EvalP3()
{
	//Power
	double left = EvalP4();
	while(mType == OP_POWER)
		left = pow(left, EvalP4());
	return left;
}

double Expression::EvalP4()
{
	mType = mTokens[mTokenIndex++];  
	switch (mType)
	{
		//floating point constant
		case NUMBER :
		{
			double v = mValues[mValueIndex++];
			mType = mTokens[mTokenIndex++];
			return v;
		}
		//variable
		case VAR :
		{
			int idx = mVars[mVarIndex++];
			mType = mTokens[mTokenIndex++];
			return mVarsPtr->at(idx);
		}
		case FUNCTION :
		{
			double e = EvalFunction();
			VerifyRP();
			return e;
		}
		//parentheses
		case LP :
		{
			double e = EvalP1();
			VerifyRP();
			return e;
		}
		// unary minus
		case OP_MINUS :
			return -EvalP3();
		default :
			Throw("Primary expected");
	}
}

double Expression::EvalFunction()
{
	const double DEG_TO_RAD = 0.01745329251994329444444444444444;//PI / 180.0;
	const double RAD_TO_DEG = 57.295779513082320876798154814105;//180.0 / PI;

	FunctionType function = mFunctions[mFunctionIndex++];
	switch(function)
	{
		case FN_TIME:
			mType = mTokens[mTokenIndex++];
			return mContextPtr->GetTime();
		case FN_SIN:
			return sin(DEG_TO_RAD * EvalP1());
		case FN_COS:
		case FN_MIN:
		case FN_MAX:
			break;
	}
	Throw("FunctionType not implemented!");
}

void Expression::VerifyRP()
{
	if (mType != RP) 
		Throw("')' expected.");
	mType = mTokens[mTokenIndex++];
}