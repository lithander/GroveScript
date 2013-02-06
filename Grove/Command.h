#pragma once
#include "Weirwood.h"
#include "Expression.h"

namespace Weirwood
{
	template <typename T>
	class Command
	{
	public:
		Command(T op) : mLineNumber(-1), mOperation(op), mBlockDepth(0) {}
		Command(T op, int blockDepth) : mLineNumber(-1), mOperation(op), mBlockDepth(blockDepth) {}
		~Command(void);

		void SetDebugInfo(int lineNr) { mLineNumber = lineNr; };
		void SetOperation(T op) { mOperation = op; };

		void PushParam(const std::string& token, const Expression& exp)
		{
			mTokens.push_back(token);
			mExpressions.push_back(exp);
		}
		
		inline bool Command<T>::HasToken(int index) const
		{
			return (index < (int)mExpressions.size());
		}
		
		inline T GetOperation() const
		{
			return mOperation;
		}
		
		inline int GetBlockDepth() const
		{
			return mBlockDepth;
		}

		std::string GetToken(int index) const;
		std::string GetToken(int index, const std::string& defaultToken) const;
		double GetNumber(int index) const;
		double GetNumber(int index, double defaultValue) const;
		bool GetBool(int index) const;
		bool IsBoolean(int index) const;
		bool IsExpression(int index) const;
	protected:
		int mLineNumber;
		std::vector<Expression> mExpressions;
		std::vector<std::string> mTokens;
		int mBlockDepth;
		T mOperation;
	};


	template <typename T>
	Command<T>::~Command(void)
	{
		for(std::vector<Expression*>::iterator it = mExpressions.begin(); it != mExpressions.end(); it++)
			delete *it;
	}
	
	template <typename T>
	double Command<T>::GetNumber(int index) const
	{
		if(index >= (int)mExpressions.size())
		{
			std::stringstream ss;
			ss << "Line " << mLineNumber << ": Not enough parameters!";
			throw Error(ss.str());
		}
		return mExpressions[index].AsNumber();
	}

	template <typename T>
	double Command<T>::GetNumber(int index, double defaultValue) const
	{
		if(index >= (int)mExpressions.size())
			return defaultValue;
		return mExpressions[index].AsNumber();;
	}

	template <typename T>
	bool Command<T>::GetBool(int index) const
	{
		if(index >= (int)mExpressions.size())
		{
			std::stringstream ss;
			ss << "Line " << mLineNumber << ": Not enough parameters!";
			throw Error(ss.str());
		}
		return mExpressions[index].AsBool();
	}

	template <typename T>
	std::string Command<T>::GetToken(int index) const
	{
		if(index >= (int)mExpressions.size())
		{
			std::stringstream ss;
			ss << "Line " << mLineNumber << ": Not enough parameters!";
			throw Error(ss.str());
		}
		return mTokens[index];
	}

	template <typename T>
	std::string Command<T>::GetToken(int index, const std::string& defaultToken) const
	{
		if(index >= (int)mExpressions.size())
			return defaultToken;
		return mTokens[index];
	}

	template <typename T>
	bool Command<T>::IsBoolean(int index) const
	{
		if(index < (int)mExpressions.size())
			return mExpressions[index].IsBoolean();
		else
			return false;
	}

	template <typename T>
	bool Command<T>::IsExpression(int index) const
	{
		if(index < (int)mExpressions.size())
			return !mExpressions[index].IsEmpty();
		else
			return false;
	}


}

