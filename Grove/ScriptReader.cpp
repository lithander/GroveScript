#include "Weirwood.h"
#include "ScriptReader.h"
#include "Command.h"
#include "Processor.h"

using namespace Weirwood;

ScriptReader::ScriptReader(void)
{
}


ScriptReader::~ScriptReader(void)
{
}

bool ScriptReader::Parse(std::istream& input, Processor* procPtr)
{
	std::string id = "ROOT";
	std::string line;
	while(input.good() && !input.eof())
	{
		std::getline(input, line);
		if(line == "")
		{
			continue;
		}
		if(line.find('#') == 0)
		{
			id = line.substr(1);
		}
		else if(line.find("=>") != line.npos)
		{
			procPtr->AppendProduction()->Parse(line);
		}
		else
			procPtr->AppendCommand(id)->Parse(line);
	}
	return true;
}
