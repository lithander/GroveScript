#pragma once
#include <string>
#include <list>
#include <vector>
#include <map>
#include <stack>
#include <sstream>
#include <istream>
#include <algorithm>
#include <iterator>
#include "boost\algorithm\string.hpp"

namespace Weirwood 
{
	template <typename T>
	class Command;
	
	class Sprout;
	class Processor;
	class Sprout;
	class Expression;
	class ProductionRule;
	
	struct Error : public std::exception
	{
		std::string desc;
		Error(std::string errorDesc) : desc(errorDesc) {}
		const char* what() const throw() { return desc.c_str(); }
	};

	enum InstructionSet
	{
		NO_OP,
		MOVE_OP,
		ROTATE_OP,
		SIZE_OP,
		COLOR_RGB_OP,
		COLOR_HSV_OP,
		POSITION_OP,
		DIRECTION_OP,
		SET_OP,
		PUSH_OP,
		POP_OP,
		OUT_OP,
		EXE_OP,
		SEED_OP,
		GROW_OP,
		GATE_OP,
		BREAK_OP,
		REPEAT_OP,
		SRAND_OP
	};
		
	enum Macros
	{
		VOID_MC,
		REPEAT_MC,
		WHILE_MC,
		UNTIL_MC,
		IF_MC,
		ELSE_MC,
		RAISE_MC,
		LOWER_MC
	};
		
	
	//Types
	typedef std::map<std::string, double> VarTable;
	typedef std::list<std::string> StringList;
	typedef std::list<int> SymbolList;

	typedef std::vector<Command<InstructionSet>*> CommandList;
	typedef std::vector<CommandList> Sequences;
	typedef std::vector<ProductionRule> Productions;
	typedef std::vector<double> Variables;
	typedef std::vector<SymbolList> Structures;

	//Utility
	void SplitString(const std::string& input, StringList& output);
};

