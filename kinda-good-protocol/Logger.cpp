#include "Logger.h"

std::unique_ptr<QFile> kgp::Logger::LogFile = std::make_unique<QFile>();
