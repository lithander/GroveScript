#include "Keywords.h"
#include <cctype>
#include <functional>

using namespace Weirwood;

std::string to_upper_copy(const std::string& token)
{
	std::string result = token;
	std::transform(result.begin(), result.end(), result.begin(), std::ptr_fun(toupper));
	return result;
}

Keywords* Keywords::mInstance = NULL;

Keywords::Keywords(void)
{
}

Keywords::~Keywords(void)
{
}

InstructionSet Keywords::Operation(const std::string& token)
{
	return Instance()->GetOperation(token);
}

InstructionSet Keywords::GetOperation(const std::string& token)
{
	std::string op = to_upper_copy(token);
	std::map<std::string, InstructionSet>::iterator it = mOperations.find(op);
	if(it != mOperations.end())
		return it->second;
	else
		return NO_OP;
}

Macros Keywords::Macro(const std::string& token)
{
	return Instance()->GetMacro(token);
}

Macros Keywords::GetMacro(const std::string& token)
{
	std::string op = to_upper_copy(token);
	std::map<std::string, Macros>::iterator it = mMacros.find(op);
	if(it != mMacros.end())
		return it->second;
	else
		return VOID_MC;
}

Expression::FunctionSet Keywords::Function(const std::string& token)
{
	return Instance()->GetFunction(token);
}

Expression::FunctionSet Keywords::GetFunction(const std::string& token)
{
	std::string fn = to_upper_copy(token);
	std::map<std::string, Expression::FunctionSet>::iterator it = mFunctions.find(fn);
	if(it != mFunctions.end())
		return it->second;
	else
		return Expression::VOID_FN;
}

Expression::TokenType Keywords::Token(const std::string& token)
{
	return Instance()->GetToken(token);
}

Expression::TokenType Keywords::GetToken(const std::string& token)
{
	std::string fn = to_upper_copy(token);
	std::map<std::string, Expression::TokenType>::iterator it = mTokens.find(fn);
	if(it != mTokens.end())
		return it->second;
	else
		return Expression::END;
}


double Keywords::Constant(const std::string& token)
{
	bool dontCare;
	return Instance()->GetConstant(token, dontCare);
}

double Keywords::Constant(const std::string& token, bool& defined)
{
	return Instance()->GetConstant(token, defined);
}

void Keywords::Define(const std::string& token, double value)
{
	std::string cnst = to_upper_copy(token);
	Instance()->SetConstant(cnst, value);
}

double Keywords::GetConstant(const std::string& token, bool& defined)
{
	std::string cnst = to_upper_copy(token);
	std::map<std::string, double>::iterator it = mConstants.find(cnst);
	if(it != mConstants.end())
	{
		defined = true;
		return it->second;
	}
	else
	{
		defined = false;
		return 0;
	}
}

void Keywords::SetConstant(const std::string& token, double value)
{
	mConstants[token] = value;
}

Keywords* Keywords::Instance()
{
	if(mInstance == NULL)
	{
		mInstance = new Keywords();
		mInstance->Init();
	}
	return mInstance;
}

void Keywords::Init()
{
	//OPERATIONS
	mOperations["MOV"] = MOVE_OP;
	mOperations["POS"] = POSITION_OP;
	mOperations["ROT"] = ROTATE_OP;
	mOperations["DIR"] = DIRECTION_OP;
	mOperations["AIM"] = AIM_OP;
	mOperations["SZE"] = SIZE_OP;
	mOperations["VIS"] = ALPHA_OP;
	mOperations["RGB"] = COLOR_RGB_OP;
	mOperations["HSV"] = COLOR_HSV_OP;
	mOperations["SET"] = SET_OP;
	mOperations["PSH"] = PUSH_OP;
	mOperations["POP"] = POP_OP;
	mOperations["OUT"] = OUT_OP;
	mOperations["RUN"] = EXE_OP;
	mOperations["GRW"] = GROW_OP;
	mOperations["SED"] = SEED_OP;
	mOperations["SRD"] = SRAND_OP;
	//TODO: hidden commands
	mOperations["CND"] = GATE_OP;
	mOperations["BRK"] = BREAK_OP; 
	mOperations["RPT"] = REPEAT_OP;

	mOperations["MOVE"] = MOVE_OP;
	mOperations["POSITION"] = POSITION_OP;
	mOperations["ROTATE"] = ROTATE_OP;
	mOperations["DIRECTION"] = DIRECTION_OP;
	mOperations["SIZE"] = SIZE_OP;
	mOperations["VISIBLE"] = ALPHA_OP;
	mOperations["RGB"] = COLOR_RGB_OP;
	mOperations["HSV"] = COLOR_HSV_OP;
	mOperations["SET"] = SET_OP;
	mOperations["PUSH"] = PUSH_OP;
	mOperations["POP"] = POP_OP;
	mOperations["OUT"] = OUT_OP;
	mOperations["EXECUTE"] = EXE_OP;
	mOperations["SEED"] = SEED_OP;
	mOperations["GROW"] = GROW_OP;
	mOperations["SHUFFLE"] = SRAND_OP;

	//Expression FUNCTIONS
	mFunctions["SIN"] = Expression::SIN_FN;
	mFunctions["COS"] = Expression::COS_FN;
	mFunctions["TAN"] = Expression::TAN_FN;
	mFunctions["ASIN"] = Expression::ASIN_FN;
	mFunctions["ACOS"] = Expression::ACOS_FN;
	mFunctions["ATAN"] = Expression::ATAN_FN;
	mFunctions["EXP"] = Expression::EXP_FN;
	mFunctions["LN"] = Expression::LN_FN;
	mFunctions["FLOOR"] = Expression::FLOOR_FN;
	mFunctions["CEIL"] = Expression::CEIL_FN;
	mFunctions["ABS"] = Expression::ABS_FN;
	mFunctions["TIME"] = Expression::TIME_FN;
	mFunctions["MIN"] = Expression::MIN_FN;
	mFunctions["MAX"] = Expression::MAX_FN;
	mFunctions["CLAMP"] = Expression::CLAMP_FN;
	mFunctions["FRAC"] = Expression::FRAC_FN;
	mFunctions["RND"] = Expression::RND_FN;
	mFunctions["RANDOM"] = Expression::RND_FN;

	//Expression TOKENS
	mTokens["NOT"] = Expression::LG_NOT;
	mTokens["AND"] = Expression::LG_AND;
	mTokens["OR"] = Expression::LG_OR;
	mTokens["TRUE"] = Expression::LG_TRUE;
	mTokens["FALSE"] = Expression::LG_FALSE;
	mTokens["IS"] = Expression::LG_EQV;
	mTokens["ISNT"] = Expression::LG_NEQV;

	//Macros
	mMacros["REPEAT"] = REPEAT_MC;
	mMacros["WHILE"] = WHILE_MC;
	mMacros["UNTIL"] = UNTIL_MC;
	mMacros["IF"] = IF_MC;
	mMacros["ELSE"] = ELSE_MC;
	mMacros["RAISE"] = RAISE_MC;
	mMacros["LOWER"] = LOWER_MC;

	//Constants
	mConstants["PI"] = 3.14159265359;
}
