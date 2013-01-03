#pragma once
#include "Weirwood.h"

namespace Weirwood
{
	class Keywords
	{
	public:		
		~Keywords(void);
		static InstructionSet Operation(const std::string& token);
		static FunctionSet Function(const std::string& token);
	private:
		static Keywords* mInstance;
		static Keywords* Instance();
		
		Keywords(void);
		void Init();		
		InstructionSet GetOperation(const std::string& token);
		FunctionSet GetFunction(const std::string& token);

		std::map<std::string, InstructionSet> mOperations;
		std::map<std::string, FunctionSet> mFunctions;
	};
}
