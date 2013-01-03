#include "Keywords.h"

using namespace Weirwood;

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
	std::string op = boost::to_upper_copy(token);
	std::map<std::string, InstructionSet>::iterator it = mOperations.find(op);
	if(it != mOperations.end())
		return it->second;
	else
		return NO_OP;
}

FunctionSet Keywords::Function(const std::string& token)
{
	return Instance()->GetFunction(token);
}

FunctionSet Keywords::GetFunction(const std::string& token)
{
	std::string fn = boost::to_upper_copy(token);
	std::map<std::string, FunctionSet>::iterator it = mFunctions.find(fn);
	if(it != mFunctions.end())
		return it->second;
	else
		return VOID_FN;
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
	mOperations["ROT"] = ROTATE_OP;
	mOperations["SZE"] = SIZE_OP;
	mOperations["RGB"] = COLOR_RGB_OP;
	mOperations["HSV"] = COLOR_HSV_OP;
	mOperations["POS"] = POSITION_OP;
	mOperations["DIR"] = DIRECTION_OP;
	mOperations["SET"] = SET_OP;
	mOperations["PSH"] = PUSH_OP;
	mOperations["POP"] = POP_OP;
	mOperations["OUT"] = OUT_OP;
	mOperations["GRW"] = GROW_OP;

	//FUNCTIONS
	mFunctions["SIN"] = SIN_FN;
	mFunctions["COS"] = COS_FN;
	mFunctions["TAN"] = TAN_FN;
	mFunctions["ASIN"] = ASIN_FN;
	mFunctions["ACOS"] = ACOS_FN;
	mFunctions["ATAN"] = ATAN_FN;
	mFunctions["EXP"] = EXP_FN;
	mFunctions["LN"] = LN_FN;
	mFunctions["FLOOR"] = FLOOR_FN;
	mFunctions["CEIL"] = CEIL_FN;
	mFunctions["ABS"] = ABS_FN;
	mFunctions["TIME"] = TIME_FN;
	mFunctions["MIN"] = MIN_FN;
	mFunctions["MAX"] = MAX_FN;
	mFunctions["CLAMP"] = CLAMP_FN;
}
