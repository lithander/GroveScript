#include "Weirwood.h"
#include "Expression.h"
#include "Keywords.h"
#include <cmath>

using namespace Weirwood;

Expression::Expression(IExpressionContext* pContext) : mContextPtr(pContext), mLineNumber(-1), mVarsPtr(NULL)
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

void Expression::Throw(std::string error) const
{
	std::stringstream ss;
	if(mLineNumber >= 1)
		ss << "Line " << mLineNumber << ": ";
	ss << error;
	throw Error(ss.str());
}

double Expression::AsNumber() const
{
	mVarsPtr = mContextPtr->GetVars();
	mTokenIndex = 0;
	mDataIndex = 0;
	return EvalM1();
}

bool Expression::AsBool() const
{
	mVarsPtr = mContextPtr->GetVars();
	mTokenIndex = 0;
	mDataIndex = 0;
	return true;
}

bool Expression::IsBoolean() const
{
	return false;
}

double Expression::EvalM1() const
{
	//Addition/Subtraction
	double left = EvalM2();      
	while(true)
		switch (mType)
		{
			case OP_PLUS :
				left += EvalM2();
				break;
			case OP_MINUS :
				left -= EvalM2();
				break;
			default :
				return left;
		}
}

double Expression::EvalM2() const
{
	//Multiplication/Division/Modulo
	double left = EvalM3();
	while(true)
		switch (mType)
		{
			case OP_MOD:
				left = fmod(left, EvalM3());
				break;
			case OP_MUL :
				left *= EvalM3();
				break;
			case OP_DIV :
				if (double d = EvalM3())
					left /= d;
				else
					Throw("Division by Zero");
				break;
			default :
				return left;
		}
}

double Expression::EvalM3() const
{
	//Power
	double left = EvalM4();
	while(mType == OP_POWER)
		left = pow(left, EvalM4());
	return left;
}

double Expression::EvalM4() const
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
			return mVarsPtr->at(idx);
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
			double e = EvalM1();
			VerifyRP();
			return e;
		}
		// unary minus
		case OP_MINUS :
			return -EvalM3();
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
		case SIN_FN:
			return sin(DEG_TO_RAD * EvalM1());
		case COS_FN:
			return cos(DEG_TO_RAD * EvalM1());
		case TAN_FN:
			return tan(DEG_TO_RAD * EvalM1());
		case MIN_FN:
			a = EvalM1();
			while(mType != RP)
				a = std::min(a, EvalM1());
			return a;
		case MAX_FN:
			a = EvalM1();
			while(mType != RP)
				a = std::max(a, EvalM1());
			return a;
		case ABS_FN:
			return abs(EvalM1());
		case CLAMP_FN: //return clamp(v,min,max)
			a = EvalM1();
			return std::min(EvalM1(), std::max(a, EvalM1()));
		case ASIN_FN:
		case ACOS_FN:
		case ATAN_FN:
		case EXP_FN:
			return exp(EvalM1());
		case LN_FN:
			return log(EvalM1());
		case FLOOR_FN:
			return floor(EvalM1());
		case CEIL_FN:
			return ceil(EvalM1());
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