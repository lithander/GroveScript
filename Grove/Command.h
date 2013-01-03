#pragma once
#include "Weirwood.h"
#include "Expression.h"

namespace Weirwood
{
	template <typename T>
	class Command
	{
	public:
		Command(T op) : mLineNumber(-1), mOperation(op) {}
		~Command(void);

		void SetDebugInfo(int lineNr) { mLineNumber = lineNr; };
		void SetOperation(T op) { mOperation = op; };

		void PushParam(const std::string& token, const Expression& exp)
		{
			mTokens.push_back(token);
			mExpressions.push_back(exp);
			mCount++;
		}
		
		T GetOperation() const
		{
			return mOperation;
		}
		
		std::string GetToken(int index) const;
		double GetNumber(int index) const;
		bool GetBool(int index) const;
		bool IsBoolean(int index) const;
	protected:
		int mLineNumber;
		std::vector<Expression> mExpressions;
		std::vector<std::string> mTokens;
		int mCount;
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
		if(index >= mExpressions.size())
		{
			std::stringstream ss;
			ss << "Line " << mLineNumber << ": Not enough parameters!";
			throw Error(ss.str());
		}
		return mExpressions[index].AsNumber();
	}

	template <typename T>
	bool Command<T>::GetBool(int index) const
	{
		if(index >= mExpressions.size())
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
		if(index >= mExpressions.size())
		{
			std::stringstream ss;
			ss << "Line " << mLineNumber << ": Not enough parameters!";
			throw Error(ss.str());
		}
		return mTokens[index];
	}

	template <typename T>
	bool Command<T>::IsBoolean(int index) const
	{
		if(index < mExpressions.size())
			return mExpressions[index].IsBoolean();
		else
			return false;
	}
}

