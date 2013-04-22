#pragma once
#include "Weirwood.h"
#include "Expression.h"

namespace Weirwood
{
	class Keywords
	{
	public:		
		~Keywords(void);
		static InstructionSet Operation(const std::string& token);
		static Macros Macro(const std::string& token);
		static Expression::FunctionSet Function(const std::string& token);
		static Expression::TokenType Token(const std::string& token);
		static double Constant(const std::string& token);
		static double Constant(const std::string& token, bool& defined);
		static void Define(const std::string& token, double value);
	private:
		static Keywords* mInstance;
		static Keywords* Instance();
		
		Keywords(void);
		void Init();		
		InstructionSet GetOperation(const std::string& token);
		Macros GetMacro(const std::string& token);
		Expression::FunctionSet GetFunction(const std::string& token);
		Expression::TokenType GetToken(const std::string& token);
		double GetConstant(const std::string& token, bool& defined);
		void SetConstant(const std::string& token, double value);

		std::map<std::string, InstructionSet> mOperations;
		std::map<std::string, Expression::FunctionSet> mFunctions;
		std::map<std::string, Expression::TokenType> mTokens;
		std::map<std::string, Macros> mMacros;
		std::map<std::string, double> mConstants;
	};
}
