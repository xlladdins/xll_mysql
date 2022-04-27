// xll_mysql.h - MySQL add-in
#pragma once
#include <exception>
#include <mysql/mysql.h>
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

	class connect {
		MYSQL mysql;
	public:
		connect(
			const char* host,
			const char* user,
			const char* passwd,
			const char* db = nullptr,
			unsigned int port = 0,
			const char* unix_socket = nullptr,
			unsigned long client_flag = 0)
		{
			if (!mysql_init(&mysql)) {
				throw std::runtime_error(mysql_error(&mysql));
			}
			bool t = true;
			mysql_options(&mysql, MYSQL_ENABLE_CLEARTEXT_PLUGIN, &t);
			if (!mysql_real_connect(&mysql, host, user, passwd, db, port, unix_socket, client_flag)) {
				throw std::runtime_error(mysql_error(&mysql));
			}
		}
		connect(const connect&) = delete;
		connect& operator=(const connect&) = delete;
		~connect()
		{
			mysql_close(&mysql);
		}
		operator MYSQL* ()
		{
			return &mysql;
		}
		const char* error()
		{
			return mysql_error(&mysql);
		}
		int query(const char* sql)
		{
			return mysql_query(&mysql, sql);
		}
	};

} // namespace mysql

namespace xll {
#if 0
	inline OPER4 mysql_to_oper(const MYSQL_FIELD& f)
	{
		switch (f.type) {
			//case MYSQL_TYPE_DECIMAL:
		case MYSQL_TYPE_TINY:
			return OPER(static_cast<unsigned char>())
				MYSQL_TYPE_SHORT,
				MYSQL_TYPE_LONG,
				MYSQL_TYPE_FLOAT,
				MYSQL_TYPE_DOUBLE,
				MYSQL_TYPE_NULL,
				MYSQL_TYPE_TIMESTAMP,
				MYSQL_TYPE_LONGLONG,
				MYSQL_TYPE_INT24,
				MYSQL_TYPE_DATE,
				MYSQL_TYPE_TIME,
				MYSQL_TYPE_DATETIME,
				MYSQL_TYPE_YEAR,
				MYSQL_TYPE_NEWDATE, /**< Internal to MySQL. Not used in protocol */
				MYSQL_TYPE_VARCHAR,
				MYSQL_TYPE_BIT,
				MYSQL_TYPE_TIMESTAMP2,
				MYSQL_TYPE_DATETIME2,   /**< Internal to MySQL. Not used in protocol */
				MYSQL_TYPE_TIME2,       /**< Internal to MySQL. Not used in protocol */
				MYSQL_TYPE_TYPED_ARRAY, /**< Used for replication only */
				MYSQL_TYPE_JSON = 245,
				MYSQL_TYPE_NEWDECIMAL = 246,
				MYSQL_TYPE_ENUM = 247,
				MYSQL_TYPE_SET = 248,
				MYSQL_TYPE_TINY_BLOB = 249,
				MYSQL_TYPE_MEDIUM_BLOB = 250,
				MYSQL_TYPE_LONG_BLOB = 251,
				MYSQL_TYPE_BLOB = 252,
				MYSQL_TYPE_VAR_STRING = 253,
				MYSQL_TYPE_STRING = 254,
				MYSQL_TYPE_GEOMETRY = 255
		}
	}
#endif // 0
	std::string to_string(const OPER4& o)
	{
		std::string s;

		for (unsigned i = 0; i < o.size(); ++i) {
			ensure(o[i].is_str());
			s.append(o[i].val.str + 1, o[i].val.str[0]);
			s.append(" ");
		}

		return s;
	}

	inline OPER4 query(mysql::connect& db, const char* sql)
	{
		OPER4 o;

		if (!mysql_query(db, sql)) {
			throw std::runtime_error(mysql_error(db));
		}
		MYSQL_RES* res = mysql_store_result(db);
		if (!res) {
			const char* err = mysql_error(db);
			if (err) {
				throw std::runtime_error(err);
			}
		}
		else {
			auto c = mysql_num_fields(res);
			auto r = mysql_num_rows(res);
			o.resize((unsigned)r, c);
			for (unsigned i = 0; i < r; ++i) {
				MYSQL_ROW ri = mysql_fetch_row(res);
				MYSQL_FIELD* oi = mysql_fetch_fields(res);
				for (unsigned j = 0; j < c; ++j) {
					
				}
			}
		}

		return o;
	}

}