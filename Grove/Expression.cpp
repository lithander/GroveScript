#include "Weirwood.h"
#include "Expression.h"

using namespace Weirwood;

Expression::Expression(IExpressionContext* pContext) : mContextPtr(pContext)
{
}

Expression::~Expression(void)
{
}

void Expression::Parse(const std::string& line)
{
	//RESET
	mTokens.clear();
	mVars.clear();
	mValues.clear();
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
			case '-' :
			case '*' :
			case '/' :
			case '%' :
			case '^' :
			case '(' :
			case ')' :
				mTokens.push_back(TokenType(ch));
				break;
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
			//variable name
			default:
				std::string varName;
				if (isalpha(ch))
				{
					varName.push_back(ch);
					while (stream.get(ch) && isalnum(ch)) 
						varName.push_back(ch);
					stream.putback(ch);
					mVars.push_back(mContextPtr->GetVarIndex(varName));
					mTokens.push_back(VAR);
					break;
				}
				std::string badSymbol;
				badSymbol+=ch;
				throw Error("Symbol '"+badSymbol+"' is not understood.");
		}
	}
}

double Expression::Evaluate()
{
	mVarsPtr = mContextPtr->GetVars();
	mTokenIndex = 0;
	mValueIndex = 0;
	mVarIndex = 0;
	return EvalP1();
}

double Expression::EvalP1()
{
	//Addition/Subtraction
	double left = EvalP2();      
	while(true)
		switch (mType)
		{
			case PLUS :
				left += EvalP2();
				break;
			case MINUS :
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
			case MOD:
				left = fmod(left, EvalP3());
				break;
			case MUL :
				left *= EvalP3();
				break;
			case DIV :
				if (double d = EvalP3())
					left /= d;
				else
					throw Error("Division by Zero");
				break;
			default :
				return left;
		}
}

double Expression::EvalP3()
{
	//Power
	double left = EvalP4();
	while(mType == POWER)
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
		// unary minus
		case MINUS :
			return -EvalP3();
		//parentheses
		case LP :
		{
			double e = EvalP1();
			if (mType != RP) 
				throw Error("')' expected.");
			mType = mTokens[mTokenIndex++];
			return e;
		}
		default :
			throw Error("Primary expected");
	}
}