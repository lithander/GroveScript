#pragma once
#include "Weirwood.h"

namespace Weirwood
{
	class IExpressionContext
	{
	public:
		virtual int GetVarIndex(const std::string& name) = NULL;
		virtual Variables* GetVars() = NULL;
		virtual double GetTime() = NULL;
		virtual void Log(const std::string& msg) = NULL;
		virtual void Abort(const std::string& msg) = NULL;
	};

	//TODO: create subclass that supports external functions provided through the context, maybe as template
	class Expression
	{
	public:
		enum TokenType
		{
		  END,
		  NUMBER,
		  VAR,
		  FUNCTION,
		  OP_PLUS,	
		  OP_MINUS,
		  OP_MOD,
		  OP_POWER,
		  OP_MUL,		
		  OP_DIV,
		  LP,		
		  RP,
		};

		Expression(IExpressionContext* pContext);
		~Expression(void);
		void Parse(const std::string& line, int lineNumber = -1);
		double Evaluate();
	private:
		IExpressionContext* mContextPtr;
		Variables* mVarsPtr;

		std::vector<TokenType> mTokens;
		std::vector<FunctionSet> mFunctions;
		std::vector<double> mValues;
		std::vector<int> mVars;

		//debug
		int mLineNumber;
		void Throw(std::string error);
				
		//functions
		FunctionSet GetFunctionType(const std::string& fnToken); //TODO: move into context as-well as implementation
		
		//evaluation helper
		double EvalP1();
		double EvalP2();
		double EvalP3();
		double EvalP4();
		double EvalFunction();
		inline void VerifyRP();
		
		//current state
		TokenType mType;
		int mFunctionIndex;
		int mTokenIndex;
		int mValueIndex;
		int mVarIndex;
	};
}
