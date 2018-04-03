#pragma once

#include <string>
#include "Logger.h"
namespace Jmcpp
{
	/**
	* Client 配置参数
	**/
	struct Configuration
	{
		std::string		serverUrl; 
		std::string		uploadUrl;	
		std::string		downloadUrl;	

		std::string		storagePath; //!< SDK内部数据存储路径 utf8编码
		int				logLevel = 1;//!< SDK内部日志控制 0:关闭 1:error 2:warning 3:debug 4:debug(更易读)
		LoggerPtr		logger;		 //!< 自定义logger, 默认输出到 console
	};
	
	/**
	* 鉴权信息
	**/
	struct Authorization
	{
		std::string		appKey;		//!< 开发者注册的 IM 应用 appkey
		std::string		randomStr;	//!< 20-36 长度的随机字符串, 作为salt使用
		std::string		timestamp;	//!< 当前时间戳
		std::string		signature;	//!< 签名 
	};
}