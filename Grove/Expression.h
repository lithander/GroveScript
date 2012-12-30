#pragma once
#include "Weirwood.h"

namespace Weirwood
{
	class IExpressionContext
	{
	public:
		virtual int GetVarIndex(const std::string& name) = NULL;
		virtual Variables* GetVars() = NULL;
		virtual void Log(const std::string& msg) = NULL;
	};

	class Expression
	{
	public:
		enum TokenType
		{
		  END,
		  NUMBER,
		  VAR,
		  PLUS='+',	
		  MINUS='-',
		  MOD='%',
		  POWER='^',
		  MUL='*',		
		  DIV='/',
		  LP='(',		
		  RP=')'
		};

		Expression(IExpressionContext* pContext);
		~Expression(void);
		void Parse(const std::string& line);
		double Evaluate();
	private:
		IExpressionContext* mContextPtr;
		Variables* mVarsPtr;

		std::vector<TokenType> mTokens;
		std::vector<double> mValues;
		std::vector<int> mVars;

		double EvalP1();
		double EvalP2();
		double EvalP3();
		double EvalP4();

		//current state
		TokenType mType;
		int mTokenIndex;
		int mValueIndex;
		int mVarIndex;
	};
}
