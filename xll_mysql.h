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

		int query(const std::string& sql)
		{
			return mysql_real_query(&mysql, sql.c_str(), static_cast<unsigned long>(sql.length()));
		}
	};

	class stmt {
		MYSQL_STMT* pstmt;
	public:
		stmt(MYSQL* db)
			: pstmt(mysql_stmt_init(db))
		{
			if (!pstmt) {
				throw std::runtime_error(mysql_error(db));
			}
		}
		stmt(const stmt&) = delete;
		stmt& operator=(const stmt&) = delete;
		~stmt()
		{
			mysql_stmt_close(pstmt);
		}

		operator MYSQL_STMT* ()
		{
			return pstmt;
		}

		int prepare(const char* str, unsigned long len)
		{
			return mysql_stmt_prepare(pstmt, str, len);
		}
		int prepare(const std::string& str)
		{
			return mysql_stmt_prepare(pstmt, str.c_str(), static_cast<unsigned long>(str.length()));
		}

		int execute()
		{
			return mysql_stmt_execute(pstmt);
		}

		int fetch()
		{
			return mysql_stmt_fetch(pstmt);
		}
		int fetch_column(MYSQL_BIND* bind, unsigned int column, unsigned long offset = 0)
		{
			return mysql_stmt_fetch_column(pstmt, bind, column, offset);
		}

		bool bind_param(MYSQL_BIND* bind)
		{
			return mysql_stmt_bind_param(pstmt, bind);
		}
		bool bind_result(MYSQL_BIND* bind)
		{
			return mysql_stmt_bind_result(pstmt, bind);
		}
	};

} // namespace mysql

namespace xll {

	std::string to_string(const OPER4& o)
	{
		unsigned i = 0;
		ensure(o[i].is_str());
		std::string str(o[i].val.str + 1, o[i].val.str[0]);

		for (i = 1; i < o.size(); ++i) {
			ensure(o[i].is_str());
			str.append(" ");
			str.append(o[i].val.str + 1, o[i].val.str[0]);
		}

		return str;
	}

	class result {
		MYSQL_RES* res;
	public:
		result(MYSQL* db)
			: res(mysql_use_result(db))
		{ 
			if (!res) {
				throw std::runtime_error(mysql_error(db));
			}
		}
		result(const result&) = delete;
		result& operator=(const result&) = delete;
		~result()
		{
			mysql_free_result(res);
		}

		auto field_count() const
		{
			return res->field_count;
		}
		auto type(unsigned i) const
		{
			return res->fields[i].type;
		}

		MYSQL_ROW fetch()
		{
			return mysql_fetch_row(res);
		}
		OPER4 to_oper(MYSQL_ROW r)
		{
			OPER4 o(1, res->field_count);

			for (unsigned i = 0; i < o.size(); ++i) {
				o[i] = Excel4(xlfValue, OPER4(r[i]));
				if (o[i].is_err()) {
					o[i] = OPER4(r[i]);
				}
			}

			return o;
		}
	};

	inline OPER4 fetch(mysql::connect& db, const std::string& sql/*, const OPER4& params*/)
	{
		OPER4 o;

		int ret = db.query(sql);
		result rows(db);
		while (auto row = rows.fetch()) {
			o.push_bottom(rows.to_oper(row));
		}

		return o;
	}

	inline bool insert(mysql::connect& db, const char* table, const OPER4& o)
	{
		std::vector<MYSQL_BIND> bind;
		// field type info
		{
			std::string sel("select * from ");
			sel.append(table);
			db.query(sel);
			result res(db);
			ensure(res.field_count() == o.columns());
			bind.resize(o.columns());
			for (unsigned j = 0; j < o.columns(); ++j) {
				bind[j].buffer_length = 0;
				bind[j].is_null = 0;
				bind[j].buffer_type = res.type(j);
			}
		}

		mysql::stmt ins(db);

		std::string sql("insert into ");
		sql.append(table);
		sql.append(" values (?");
		for (unsigned j = 1; j < o.columns(); ++j) {
			sql.append(", ?");
		}
		sql.append(")");
		ins.prepare(sql);

		for (unsigned i = 0; i < o.rows(); ++i) {
			MYSQL_TIME t;
			long l;
			for (unsigned j = 0; j < o.columns(); ++j) {
				switch (o(i, j).type()) {
				case xltypeNum:
					if (MYSQL_TYPE_DATE == bind[j].buffer_type) {
						ZeroMemory(&t, sizeof(t));
						t.year = Excel4(xlfYear, o(i, j)).as_int();
						t.month = Excel4(xlfMonth, o(i, j)).as_int();
						t.day = Excel4(xlfDay, o(i, j)).as_int();
						t.hour = Excel4(xlfHour, o(i, j)).as_int();
						t.minute = Excel4(xlfMinute, o(i, j)).as_int();
						t.second = Excel4(xlfSecond, o(i, j)).as_int();
						bind[j].buffer = &t;
						bind[j].buffer_type = bind[j].buffer_type;
						// ensure is date/time type
					}
					else if (MYSQL_TYPE_LONG == bind[j].buffer_type) {
						l = static_cast<long>(o(i, j).as_int());
						bind[j].buffer = &l;
					}
					else {
						bind[j].buffer = (void*)&o(i, j).val.num;
					}
					break;
				case xltypeStr:
					bind[j].buffer = o(i, j).val.str + 1;
					bind[j].buffer_length = o(i, j).val.str[0];
					break;
				case xltypeBool:
					bind[j].buffer = (void*)&o(i, j).val.xbool;
					break;
				}
			}
			mysql_stmt_bind_param(ins, &bind[0]);
			if (0 != ins.execute()) {
				throw std::runtime_error(mysql_error(db));
			}
		}

		return true;
	}

}