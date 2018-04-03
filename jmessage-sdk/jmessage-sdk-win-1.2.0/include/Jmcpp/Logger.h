#pragma once

#include <string>
#include <memory>
namespace Jmcpp
{
	class Logger
	{
	public:
		virtual void debug(const std::string&) = 0;
		virtual void warning(const std::string&) = 0;
		virtual void error(const std::string&) = 0;
	};
	using LoggerPtr = std::shared_ptr<Logger>;
}