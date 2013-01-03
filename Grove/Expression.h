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
		double AsNumber() const;
		bool AsBool() const;
		bool IsBoolean() const;
	private:
		IExpressionContext* mContextPtr;
		
		union TokenData
		{
			TokenData(FunctionSet func) : Function(func) {}
			TokenData(double val) : Value(val) {}
			TokenData(int varIndex) : VarIndex(varIndex) {}
			FunctionSet Function;
			double Value;
			int VarIndex;
		};

		std::vector<TokenType> mTokens;
		std::vector<TokenData> mData;
		
		//debug
		int mLineNumber;
		void Throw(std::string error) const;		
		
		//evaluation helper
		double EvalM1() const;
		double EvalM2() const;
		double EvalM3() const;
		double EvalM4() const;
		double EvalFunction(FunctionSet func) const;
		inline void VerifyRP() const;
		
		//current evaluation state
		mutable TokenType mType;
		mutable Variables* mVarsPtr;
		mutable int mTokenIndex;
		mutable int mDataIndex;
	};
}
