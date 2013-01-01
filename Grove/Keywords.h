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
		Keywords(void);
	};
}
