#pragma once

#include <memory>

#include "res.h"
#include "Logger.h"

namespace kgp
{
	class DependencyManager
	{
	private:
		static std::unique_ptr<DependencyManager> instance;

		Logger mLogger;

	public:
		/*--------------------------------------------------------------------------------------------------
		-- FUNCTION:				kgp::DependencyManager::Instance
		--
		-- DATE:					November 27, 2018
		--
		-- REVISIONS:				N/A
		--
		-- DESIGNER:				Benny Wang
		--
		-- PROGRAMMER:				Benny Wang
		--
		-- INTERFACE:				DependancyManager& kgp::DependencyManager::Instance()
		--
		-- RETURNS:					A reference to the dependency manager singleton.
		--
		-- NOTES:
		--							This function should be used instead to interact with the dependency
		--							manager. The constructor should never be explicitly called.
		--------------------------------------------------------------------------------------------------*/
		inline static DependencyManager& Instance() { return *instance; }

		/*--------------------------------------------------------------------------------------------------
		-- FUNCTION:				kgp::DependancyManager::DependancyManager
		--
		-- DATE:					November 27, 2018
		--
		-- REVISIONS:				N/A
		--
		-- DESIGNER:				Benny Wang
		--
		-- PROGRAMMER:				Benny Wang
		--
		-- INTERFACE:				kgp::DependencyManager::DependencyManager()
		--
		-- NOTES:
		--							Default constructor.
		--------------------------------------------------------------------------------------------------*/
		inline DependencyManager() = default;

		/*--------------------------------------------------------------------------------------------------
		-- FUNCTION:				kgp::DependancyManager::~DependancyManager
		--
		-- DATE:					November 27, 2018
		--
		-- REVISIONS:				N/A
		--
		-- DESIGNER:				Benny Wang
		--
		-- PROGRAMMER:				Benny Wang
		--
		-- INTERFACE:				kgp::DependencyManager::~DependencyManager()
		--
		-- NOTES:
		--							Default deconstructor.
		--------------------------------------------------------------------------------------------------*/		
		~DependencyManager() = default;

		/*--------------------------------------------------------------------------------------------------
		-- FUNCTION:				kgp::DependancyManager::Logger
		--
		-- DATE:					November 27, 2018
		--
		-- REVISIONS:				N/A
		--
		-- DESIGNER:				Benny Wang
		--
		-- PROGRAMMER:				Benny Wang
		--
		-- INTERFACE:				Logger& kgp::DependencyManager::Logger()
		--
		-- RETURNS:					A reference to the logging class.
		--
		-- NOTES:
		--							This function gets a reference to the current logger. This is so that
		--							it is ensured that the entire application uses the same logger so that
		--							all messages are written to the same place.
		--------------------------------------------------------------------------------------------------*/
		inline Logger& Logger() { return mLogger; }
	};
}
