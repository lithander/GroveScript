#pragma once
#include "Weirwood.h"
#include "Expression.h"

namespace Weirwood
{
	template <typename T>
	class Dictionary
	{
	private:
		std::vector<T> mData;
		IndexTable mIndices;
	public:
		std::vector<T>& Data() { return mData; }

		void Reserve(int size)
		{
			mData.reserve(size);
		}

		void Clear()
		{
			mIndices.clear();
			mData.clear();
		}

		bool Contains(const std::string& name)
		{
			IndexTable::iterator it = mIndices.find(name);
			return (it != mIndices.end());
		}

		int IndexOf(const std::string& name)
		{
			IndexTable::iterator it = mIndices.find(name);
			if(it != mIndices.end())
				return it->second;
			else
			{
				//insert
				int index = mData.size();
				mData.resize(index+1);
				mIndices[name] = index;
				return index;
			}
		}

		void Insert(const std::string& name, T value)
		{
			IndexTable::iterator it = mIndices.find(name);
			if(it != mIndices.end())
				mData[it->second] = value;
			else
			{
				//insert
				mData.push_back(value);
				mIndices[name] = mData.size() -1; //eg last index after resize
			}
		}

		T& Retrieve(const std::string& name)
		{
			IndexTable::iterator it = mIndices.find(name);
			if(it != mIndices.end())
				return mData[it->second];
			else
			{
				//insert
				int index = mData.size();
				mData.resize(index+1);
				mIndices[name] = index;
				return mData.at(index);
			}
		}

		inline T& Retrieve(int index)
		{
			return mData.at(index);
		}
	};
}