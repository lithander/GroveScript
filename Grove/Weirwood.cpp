#include "Weirwood.h"

void Weirwood::SplitString(const std::string& input, Weirwood::StringList& output)
{
	std::stringstream ss(input);
	std::copy(std::istream_iterator<std::string>(ss),
         std::istream_iterator<std::string>(),
         std::back_inserter<std::list<std::string>>(output));
}
