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
		  DELIMITER
		};

		Expression(IExpressionContext* pContext);
		~Expression(void);
		void SetDebugInfo(int lineNr) { mLineNumber = lineNr; };
		void PushToken(TokenType token);
		void PushFunction(FunctionSet func);
		void PushNumber(double value);
		void PushVariable(int varIdx);
		double Evaluate() const;
	private:
		IExpressionContext* mContextPtr;
		
		std::vector<TokenType> mTokens;
		std::vector<FunctionSet> mFunctions;
		std::vector<double> mValues;
		std::vector<int> mVars;

		//debug
		int mLineNumber;
		void Throw(std::string error) const;		
		
		//evaluation helper
		double EvalP1() const;
		double EvalP2() const;
		double EvalP3() const;
		double EvalP4() const;
		double EvalFunction() const;
		inline void VerifyRP() const;
		
		//current evaluation state
		mutable TokenType mType;
		mutable Variables* mVarsPtr;
		mutable int mTokenIndex;
		mutable short mFunctionIndex;
		mutable short mValueIndex;
		mutable short mVarIndex;
	};
}
