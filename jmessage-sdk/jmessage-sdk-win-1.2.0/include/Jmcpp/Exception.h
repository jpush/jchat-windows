#pragma once

#include <string>
#include <stdexcept>
#include <system_error>
#include "JmcppExport.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4251 4275)
#endif

namespace Jmcpp
{
	//! 调用SDK,服务器返回的错误异常
	struct JMCPP_API ServerException :public std::runtime_error
	{
	public:
		ServerException(int errc, std::string const& msg, std::string const& context = {});
		~ServerException();

		//! 错误码
		/**
		* \see https://docs.jiguang.cn/jmessage/client/im_errorcode_js/
		**/
		int	code() const;

		std::string const& context() const;

	protected:
		int	_errc = 0;
		std::string _context;
	};
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
