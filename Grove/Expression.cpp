#include "Weirwood.h"
#include "Expression.h"
#include "Keywords.h"
#include <cmath>

using namespace Weirwood;

Expression::Expression() : mContextPtr(NULL), mLineNumber(-1)
{
}

Expression::Expression(IExpressionContext* pContext) : mContextPtr(pContext), mLineNumber(-1)
{
}

Expression::~Expression(void)
{
}

void Expression::PushToken(TokenType token) 
{ 
	mTokens.push_back(token); 
}

void Expression::PushFunction(FunctionSet func) 
{ 
	mTokens.push_back(FUNCTION); 
	mData.push_back(TokenData(func)); 
}

void Expression::PushNumber(double value) 
{
	mTokens.push_back(NUMBER); 
	mData.push_back(TokenData(value)); 
};

void Expression::PushVariable(int varIdx) 
{ 
	mTokens.push_back(VAR); 
	mData.push_back(TokenData(varIdx)); 
};

void Expression::PushParam(int paramIdx)
{
	mTokens.push_back(PARAM); 
	mData.push_back(TokenData(paramIdx)); 
}

void Expression::Throw(std::string error) const
{
	std::stringstream ss;
	if(mLineNumber >= 1)
		ss << "Line " << mLineNumber << ": ";
	ss << error;
	throw Error(ss.str());
}

bool Expression::IsEmpty() const
{
	return mTokens.empty();
}

bool Expression::IsBoolean() const
{
	for(std::vector<TokenType>::const_iterator it = mTokens.begin(); it != mTokens.end(); it++)
		if((*it) >= LG_TRUE)
			return true;

	return false;
}

void Expression::ResolveParams(SymbolList& out_symbols) const
{	
	mTokenIndex = 1;
	mDataIndex = 1;
	mType = mTokens[0];
	SymbolList::iterator it = out_symbols.begin();
	while(it != out_symbols.end() && (mType == FUNCTION || mType == VAR))
	{
		int varIdx = 0;
		it->Params.clear();
		if(mType == FUNCTION)
			while(mType != RP)
				it->Params.push_back(EvalA1());

		mDataIndex++;//don't care for the data, it's void anyway
		mType = mTokens[mTokenIndex++];
		it++;
	}
}

double Expression::AsNumber() const
{
	mType = END;
	mTokenIndex = 0;
	mDataIndex = 0;
	return EvalA1();
}

bool Expression::AsBool() const
{
	mType = END;
	mTokenIndex = 0;
	mDataIndex = 0;
	return EvalL1();
}

bool Expression::EvalL1() const
{
	//EQV & NEQV
	bool left = EvalL2();
	for(;;)
	{
		switch(mType)
		{	
			case LG_EQV:
				left = left == EvalL2(); break;
			case LG_NEQV:
				left = left != EvalL2(); break;
			default :
				return left;
		}
	}
	return left;
}

bool Expression::EvalL2() const
{
	//OR (left to right)
	bool left = EvalL3();
	while(mType == LG_OR)
		left =  EvalL3() || left;
	return left;
}

bool Expression::EvalL3() const
{
	//AND (left to right)
	bool left = EvalL4();
	while(mType == LG_AND)
		left = EvalL4() && left;
	return left;
}

bool Expression::EvalL4() const
{
	mType = mTokens[mTokenIndex++];
	switch(mType)
	{
		case LG_TRUE:
			mType = mTokens[mTokenIndex++];
			return true;
		case LG_FALSE:
			mType = mTokens[mTokenIndex++];
			return false;
		case LP :
			{
			bool e = EvalL1();
			VerifyRP();
			return e;
			}
		case LG_NOT:
			return !EvalL4();
	}
	
	mType = mTokens[--mTokenIndex];
	return EvalComparision();
}

bool Expression::EvalComparision() const
{
	//Comparision
	double left = EvalA1();
	switch(mType)
	{
	case LG_EQL:
		return left == EvalA1();
	case LG_NEQL:
		return left != EvalA1();
	case LG_LESS:
		return left < EvalA1();
	case LG_LESS_EQL:
		return left <= EvalA1();
	case LG_GREATER:
		return left > EvalA1();
	case LG_GREATER_EQL:
		return left >= EvalA1();
	default:
		Throw("Comparision expected!");
	}
}

double Expression::EvalA1() const
{
	//Addition/Subtraction
	double left = EvalA2();      
	while(true)
		switch (mType)
		{
			case OP_PLUS :
				left += EvalA2();
				break;
			case OP_MINUS :
				left -= EvalA2();
				break;
			default :
				return left;
		}
}

double Expression::EvalA2() const
{
	//Multiplication/Division/Modulo
	double left = EvalA3();
	while(true)
		switch (mType)
		{
			case OP_MOD:
				left = fmod(left, EvalA3());
				break;
			case OP_MUL :
				left *= EvalA3();
				break;
			case OP_DIV :
				if (double d = EvalA3())
					left /= d;
				else
					Throw("Division by Zero");
				break;
			default :
				return left;
		}
}

double Expression::EvalA3() const
{
	//Power
	double left = EvalA4();
	while(mType == OP_POWER)
		left = pow(left, EvalA4());
	return left;
}

double Expression::EvalA4() const
{
	mType = mTokens[mTokenIndex++];  
	switch (mType)
	{
		//floating point constant
		case NUMBER :
		{
			double v = mData[mDataIndex++].Value;
			mType = mTokens[mTokenIndex++];
			return v;
		}
		//variable
		case VAR :
		{
			int idx = mData[mDataIndex++].VarIndex;
			mType = mTokens[mTokenIndex++];
			return mContextPtr->GetVar(idx);
		}
		//param
		case PARAM :
		{
			int idx = mData[mDataIndex++].VarIndex;
			mType = mTokens[mTokenIndex++];
			return mContextPtr->GetParam(idx);
		}
		case FUNCTION :
		{
			FunctionSet func = mData[mDataIndex++].Function;
			double e = EvalFunction(func);
			VerifyRP();
			return e;
		}
		//parentheses
		case LP :
		{
			double e = EvalA1();
			VerifyRP();
			return e;
		}
		// unary minus
		case OP_MINUS :
			return -EvalA3();
		default :
			Throw("Primary expected");
			return 0;
	}
}

double Expression::EvalFunction(FunctionSet func) const
{
	const double DEG_TO_RAD = 0.01745329251994329444444444444444;//PI / 180.0;
	const double RAD_TO_DEG = 57.295779513082320876798154814105;//180.0 / PI;

	double a;
	switch(func)
	{
		case TIME_FN:
			mType = mTokens[mTokenIndex++];
			return mContextPtr->GetTime();
		case RND_FN:
			if(mTokens[mTokenIndex] == RP)
			{
				mType = mTokens[mTokenIndex++];
				return mContextPtr->GetRandom(0, 1);
			}
			a = EvalA1();
			if(mType == RP)
				return mContextPtr->GetRandom(0, a);
			else
				return mContextPtr->GetRandom(a, EvalA1());
		case SIN_FN:
			return sin(DEG_TO_RAD * EvalA1());
		case COS_FN:
			return cos(DEG_TO_RAD * EvalA1());
		case TAN_FN:
			return tan(DEG_TO_RAD * EvalA1());
		case MIN_FN:
			a = EvalA1();
			while(mType != RP)
				a = std::min(a, EvalA1());
			return a;
		case MAX_FN:
			a = EvalA1();
			while(mType != RP)
				a = std::max(a, EvalA1());
			return a;
		case ABS_FN:
			return abs(EvalA1());
		case CLAMP_FN: //return clamp(v,min,max)
			a = EvalA1();
			return std::min(EvalA1(), std::max(a, EvalA1()));
		case ASIN_FN:
		case ACOS_FN:
		case ATAN_FN:
		case EXP_FN:
			return exp(EvalA1());
		case LN_FN:
			return log(EvalA1());
		case FLOOR_FN:
			return floor(EvalA1());
		case CEIL_FN:
			return ceil(EvalA1());
		case FRAC_FN:
			a = EvalA1();
			return a - (long)a;
			break;
	}
	Throw("FunctionType not implemented!");
	return 0;
}

void Expression::VerifyRP() const
{
	if (mType != RP) 
		Throw("Closing parenthesis expected.");
	mType = mTokens[mTokenIndex++];
}