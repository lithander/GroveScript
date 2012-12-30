#pragma once
#include "Weirwood.h"

namespace Weirwood
{
	class IProductionContext
	{
	public:
		virtual void ParseSymbolList(const std::string& line, SymbolList& out_symbols) = NULL;
		virtual void Log(const std::string& msg) = NULL;
	};

	class ProductionRule
	{
	public:
		ProductionRule(IProductionContext* pContext) : mContextPtr(pContext) {};
		~ProductionRule(void);
		void Parse(const std::string& line);
		bool Match(SymbolList& symbols, SymbolList::iterator& current);
	private:
		SymbolList mPredecessor;
		SymbolList mSuccessor;
		IProductionContext* mContextPtr;
	};
}
