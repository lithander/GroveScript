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
	std::string id = "Root";
	std::string line;
	int lineNumber = 0;
	try
	{
		while(input.good() && !input.eof())
		{
			lineNumber++;
			std::getline(input, line);
			if(line == "" || line.find("//") == 0)
				continue;

			if(line.find('#') == 0)
			{
				id = line.substr(1);
			}
			else if(line.find("=>") != line.npos)
			{
				procPtr->AppendProduction()->Parse(line);
			}
			else
			{
				Processor::Command* cmdPtr = procPtr->AppendCommand(id); 
				cmdPtr->Parse(line, lineNumber);
			}
		}
	}
	catch(Error e)
	{
		procPtr->Abort(e.desc);
	}
	return true;
}
