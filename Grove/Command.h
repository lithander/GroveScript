#pragma once
#include "Weirwood.h"
#include "Expression.h"

namespace Weirwood
{
	template <typename T>
	class ICommandContext : public IExpressionContext
	{
	public:
		virtual T GetOperationType(const std::string& opToken) = NULL;
	};

	template <typename T>
	class Command
	{
	public:
		Command(ICommandContext<T>* pContext) : mContextPtr(pContext), mLineNumber(-1) {}
		~Command(void);
		
		void Parse(const std::string& line, int lineNr = -1);
		
		T GetOperation() const
		{
			return mOperation;
		}
		
		std::string GetToken(int index) const
		{
			return mTokens[index];
		}
		
		double GetNumber(int index) const;
	protected:
		int mLineNumber;
		std::vector<Expression*> mExpressions;
		std::vector<std::string> mTokens;
		int mCount;
		ICommandContext<T>* mContextPtr;
		T mOperation;
	};


	template <typename T>
	Command<T>::~Command(void)
	{
		for(std::vector<Expression*>::iterator it = mExpressions.begin(); it != mExpressions.end(); it++)
			delete *it;
	}

	template <typename T>
	void Command<T>::Parse(const std::string& line, int lineNr)
	{
		mLineNumber = lineNr;

		//Destroy
		for(std::vector<Expression*>::iterator it = mExpressions.begin(); it != mExpressions.end(); it++)
			delete *it;
		mExpressions.clear();
		mTokens.clear();

		//Parse
		int pos = line.find(' ', 0);
		mOperation = mContextPtr->GetOperationType(line.substr(0, pos));

		mCount = 0;
		while(pos != line.npos)
		{
			int nextPos = line.find(',', pos+1);
			if(nextPos != line.npos)
				mTokens.push_back(line.substr(pos+1, nextPos-pos-1));
			else
				mTokens.push_back(line.substr(pos+1));

			//Create Expression
			Expression* pExp = new Expression(mContextPtr);
			pExp->Parse(mTokens[mCount], lineNr);
			mExpressions.push_back(pExp);
			pos = nextPos;
			mCount++;
		}
	}

	template <typename T>
	double Command<T>::GetNumber(int index) const
	{
		if(index >= mCount)
		{
			std::stringstream ss;
			ss << "Line " << mLineNumber << ": Not enough parameters!";
			mContextPtr->Abort(ss.str());
			return 1;
		}
		return mExpressions[index]->Evaluate();

	}
}

