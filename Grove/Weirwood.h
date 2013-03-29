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

namespace Weirwood 
{
	template <typename T>
	class Command;
	
	class Sprout;
	class Processor;
	class Sprout;
	class Expression;
	class ProductionRule;

	enum InstructionSet
	{
		NO_OP,
		MOVE_OP,
		POSITION_OP,
		ROTATE_OP,
		DIRECTION_OP,
		AIM_OP,
		CURVE_OP,
		SIZE_OP,
		ALPHA_OP,
		COLOR_RGB_OP,
		COLOR_HSV_OP,
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
	typedef Command<InstructionSet> Instruction;
	typedef std::vector<Instruction*> CommandList;

	typedef std::map<std::string, int> IndexTable;
	typedef std::list<std::string> StringList;

	typedef std::vector<double> Variables;
	typedef std::vector<ProductionRule*> Productions;

	struct Error : public std::exception
	{
		std::string desc;
		Error(std::string errorDesc) : desc(errorDesc) {}
		const char* what() const throw() { return desc.c_str(); }
	};

	struct Symbol
	{
		enum SymbolType
		{
			TEMPORARY,
			STRUCTURE,
			SEQUENCE
		};
		SymbolType Type; 
		int Index;
		Variables Params;
	};

	typedef std::list<Symbol> SymbolList;

	//Utility
	void SplitString(const std::string& input, StringList& output);
};

