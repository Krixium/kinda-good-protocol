/*---------------------------------------------------------------------------------------
-- SOURCE FILE:             DependancyManager.h
--
-- PROGRAM:                 KindaGoodProtocol
--
-- FUNCTIONS:               N/A
--
-- DATE:                    November 27, 2018
--
-- REVISIONS:               N/A
--
-- DESIGNERS:               Benny Wang
--
-- PROGRAMMERS:             Benny Wang
--
-- NOTES:
--                          This is a singleton class that holds objects, variables
--                          and other dependencies required throughout the application.
---------------------------------------------------------------------------------------*/
#include "DependencyManager.h"

std::unique_ptr<kgp::DependencyManager> kgp::DependencyManager::instance = std::make_unique<DependencyManager>();
