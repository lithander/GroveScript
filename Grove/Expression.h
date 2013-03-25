#pragma once
#include "Weirwood.h"

namespace Weirwood
{
	class IExpressionContext
	{
	public:
		virtual double GetVar(int index) = NULL;
		virtual double GetParam(int index) = NULL;
		virtual double GetTime() = NULL;
		//TODO: little messy to handle log's that way. singleton? exeptions?
		virtual void Log(const std::string& msg) = NULL;
		virtual void Abort(const std::string& msg) = NULL;
	};

	class Expression
	{
	public:
		enum TokenType
		{
		  END,
		  NUMBER,
		  VAR,
		  PARAM,
		  FUNCTION,
		  LP,		
		  RP,
		  DELIMITER,
		  
		  OP_PLUS,	
		  OP_MINUS,
		  OP_MOD,
		  OP_POWER,
		  OP_MUL,		
		  OP_DIV,
		  //WARNING: IsBoolean() assumes LG tokens at the end of the enum and to start with LG_TRUE
		  LG_TRUE,
		  LG_FALSE,
		  LG_NOT,
		  LG_AND,
		  LG_OR,
		  LG_EQL,
		  LG_NEQL,
		  LG_LESS,
		  LG_LESS_EQL,
		  LG_GREATER,
		  LG_GREATER_EQL,
		  LG_EQV,
		  LG_NEQV
		};

		//TODO: create subclass that supports external functions provided through the context, maybe as template
		enum FunctionSet
		{
			VOID_FN,
		
			SIN_FN,
			COS_FN,
			TAN_FN,

			ASIN_FN,
			ACOS_FN,
			ATAN_FN,
		
			EXP_FN,
			LN_FN,

			FLOOR_FN,
			CEIL_FN,
			ABS_FN,

			TIME_FN,
			RND_FN,
			MIN_FN,
			MAX_FN,
			CLAMP_FN,
			FRAC_FN
		};

		Expression();
		Expression(IExpressionContext* pContext);
		~Expression(void);
		void SetDebugInfo(int lineNr) { mLineNumber = lineNr; };
		void PushToken(TokenType token);
		void PushFunction(FunctionSet func);
		void PushNumber(double value);
		void PushVariable(int varIdx);
		void PushParam(int paramIdx);
		double AsNumber() const;
		bool AsBool() const;
		void ResolveParams(SymbolList& out_symbols) const;
		bool IsBoolean() const;
		bool IsEmpty() const;
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
		
		//arithmetic evaluation
		double EvalA1() const;
		double EvalA2() const;
		double EvalA3() const;
		double EvalA4() const;
		double EvalFunction(FunctionSet func) const;
		//logic evaluation
		bool EvalL1() const;
		bool EvalL2() const;
		bool EvalL3() const;
		bool EvalL4() const;
		bool EvalComparision() const;
		//misc
		inline void VerifyRP() const;
		
		//current evaluation state
		mutable TokenType mType;
		mutable int mTokenIndex;
		mutable int mDataIndex;
	};
}
