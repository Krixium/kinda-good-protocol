#pragma once

#include <memory>

#include "res.h"
#include "Logger.h"

namespace kgp
{
	class DependancyManager
	{
	private:
		static std::unique_ptr<DependancyManager> instance;

		State mState;
		Logger mLogger;

	public:
		inline static DependancyManager& Instance() { return *instance; }

		inline DependancyManager()
		{
			if (instance == nullptr)
			{
				initState();
			}
		}

		~DependancyManager() = default;

		inline Logger& Logger() { return mLogger; }

	private:
		inline void initState()
		{
			memset(&mState, 0, sizeof(struct State));
			mState.idle = true;
		}
	};
}
