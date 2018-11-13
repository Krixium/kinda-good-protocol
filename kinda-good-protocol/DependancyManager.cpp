#include "DependancyManager.h"

std::unique_ptr<kgp::DependancyManager> kgp::DependancyManager::instance = std::make_unique<DependancyManager>();
