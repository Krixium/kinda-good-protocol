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

		Logger mLogger;

	public:
		inline static DependancyManager& Instance() { return *instance; }

		inline DependancyManager() = default;
		~DependancyManager() = default;

		inline Logger& Logger() { return mLogger; }
	};
}
