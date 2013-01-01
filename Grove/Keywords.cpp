#include "Keywords.h"

using namespace Weirwood;

Keywords::Keywords(void)
{
}

Keywords::~Keywords(void)
{
}

InstructionSet Keywords::Operation(const std::string& token)
{
	//TODO: Use map<string, InstructionSet> to map string to enumeration member for faster lookup.
	std::string op = boost::to_upper_copy(token);
	if(op == "MOV")
		return MOVE_OP;
	else if(op == "ROT")
		return ROTATE_OP;
	else if(op == "SZE")
		return SIZE_OP;
	else if(op == "POS")
		return POSITION_OP;
	else if(op == "DIR")
		return DIRECTION_OP;
	else if(op == "SET")
		return SET_OP;
	else if(op == "PSH")
		return PUSH_OP;
	else if(op == "POP")
		return POP_OP;
	else if(op == "OUT")
		return OUT_OP;
	else if(op == "GRW")
		return GROW_OP;
	else
		return NO_OP;
}

FunctionSet Keywords::Function(const std::string& token)
{
	//TODO: Use map<string, InstructionSet> to map string to enumeration member for faster lookup.
	std::string fn = boost::to_upper_copy(token);
	if(fn == "TIME")
		return TIME_FN;
	else if(fn == "SIN")
		return SIN_FN;
	else if(fn == "COS")
		return COS_FN;
	else if(fn == "MIN")
		return MIN_FN;
	else if(fn == "MAX")
		return MAX_FN;
	else
		return VOID_FN;
}
