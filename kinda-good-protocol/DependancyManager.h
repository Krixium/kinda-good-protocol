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
		static DependancyManager& Instance();

		inline DependancyManager()
		{
			if (instance == nullptr)
			{
				initState();
			}
		}
		~DependancyManager() = default;

		inline State& GetState() { return mState; }
		inline Logger& GetLogger() { return mLogger; }

	private:
		inline void initState()
		{
			memset(&mState, 0, sizeof(struct State));
			mState.IDLE = true;
		}
	};
}
