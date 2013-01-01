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
	PushToken(FUNCTION); 
	mFunctions.push_back(func); 
}

void Expression::PushNumber(double value) 
{
	PushToken(NUMBER);
	mValues.push_back(value); 
};

void Expression::PushVariable(int varIdx) 
{ 
	PushToken(VAR); 
	mVars.push_back(varIdx); 
};

void Expression::Throw(std::string error) const
{
	std::stringstream ss;
	if(mLineNumber >= 1)
		ss << "Line " << mLineNumber << ": ";
	ss << error;
	throw Error(ss.str());
}

double Expression::Evaluate() const
{
	mVarsPtr = mContextPtr->GetVars();
	mTokenIndex = 0;
	mValueIndex = 0;
	mVarIndex = 0;
	mFunctionIndex = 0;
	return EvalP1();
}

double Expression::EvalP1() const
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

double Expression::EvalP2() const
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

double Expression::EvalP3() const
{
	//Power
	double left = EvalP4();
	while(mType == OP_POWER)
		left = pow(left, EvalP4());
	return left;
}

double Expression::EvalP4() const
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
			return 0;
	}
}

double Expression::EvalFunction() const
{
	const double DEG_TO_RAD = 0.01745329251994329444444444444444;//PI / 180.0;
	const double RAD_TO_DEG = 57.295779513082320876798154814105;//180.0 / PI;

	double a;
	FunctionSet function = mFunctions[mFunctionIndex++];
	switch(function)
	{
		case TIME_FN:
			mType = mTokens[mTokenIndex++];
			return mContextPtr->GetTime();
		case SIN_FN:
			return sin(DEG_TO_RAD * EvalP1());
		case COS_FN:
			return cos(DEG_TO_RAD * EvalP1());
		case MIN_FN:
			a = EvalP1();
			while(mType != RP)
				a = std::min(a, EvalP1());
			return a;
		case MAX_FN:
			a = EvalP1();
			while(mType != RP)
				a = std::max(a, EvalP1());
			return a;
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