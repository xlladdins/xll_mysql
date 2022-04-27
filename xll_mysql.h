// xll_mysql.h - MySQL add-in
#pragma once
#include <exception>
#define _ITERATOR_DEBUG_LEVEL 2
#include <mysql.h>
#include "xll/xll/xll.h"

#ifndef CATEGORY
#define CATEGORY "MySQL"
#endif

namespace mysql {

	struct library_init {
		library_init(int argc, char** argv, char** groups)
		{
			mysql_library_init(argc, argv, groups);
		}
		~library_init()
		{
			mysql_library_end();
		}
	};
	inline const library_init init(0, nullptr, nullptr);

	class init {
		MYSQL* mysql_;
	public:
		init(MYSQL* mysql = nullptr)
			: mysql_{ mysql_init(mysql) }
		{
			if (!mysql_) {
				throw std::runtime_error(mysql_error(&mysql_));
			}
		}
		init(const init&) = delete;
		init& operator=(const init&) = delete;
		~init()
		{
			mysql_close(mysql_);
		}
		operator MYSQL* ()
		{
			return mysql_;
		}
	};

	class real_connect {
		static inline MYSQL* mysql_ = init();
	public:
		real_connect(/*MYSQL* mysql,*/
			const char* host,
			const char* user,
			const char* passwd,
			const char* db = nullptr,
			unsigned int port = 0,
			const char* unix_socket = nullptr,
			unsigned long client_flag = 0)
			: mysql_()
		{
			mysql_ = mysql_real_connect(mysql_, host, user, passwd, db, port, unix_socket, client_flag)
			if (!mysql_) {
				throw std::runtime_error(mysql_error(&mysql_));
			}
		}
		real_connect(const real_connect&) = delete;
		real_connect& operator=(const real_connect&) = delete;
		~real_connect()
		{

		}
	};

} // namespace mysql