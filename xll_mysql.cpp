// xll_mysql.cpp
#pragma comment (lib, "crypt32")
#include "xll_mysql.h"

using namespace xll;

AddIn xai_mysql_connect(
	Function(XLL_HANDLEX, "xll_mysql_connect", "\\MYSQL.CONNECT")
	.Arguments({
		Arg(XLL_CSTRING4, "host", "is the database host name."),
		Arg(XLL_CSTRING4, "user", "is the user name."),
		Arg(XLL_CSTRING4, "pass", "is the password."),
		Arg(XLL_CSTRING4, "_db", "is the optional database name."),
		})
	.Uncalced()
	.Category(CATEGORY)
	.FunctionHelp("Return a handle to a MySQL database connection.")
	.HelpTopic("https://dev.mysql.com/doc/c-api/8.0/en/mysql-real-connect.html")
);
HANDLEX WINAPI xll_mysql_connect(const char* host, const char* user, const char* pass, const char* db)
{
#pragma XLLEXPORT
	HANDLEX h = INVALID_HANDLEX;

	try {
		handle<mysql::connect> h_(new mysql::connect(host, user, pass, db));
		h = h_.get();
	}
	catch (const std::exception& ex) {
		XLL_ERROR(ex.what());
	}

	return h;
}

AddIn xai_mysql_query(
	Function(XLL_LPOPER4, "xll_mysql_query", "MYSQL.QUERY")
	.Arguments({
		Arg(XLL_HANDLEX, "db", "is a handle to a database connection."),
		Arg(XLL_LPOPER4, "sql", "is a SQL query to execute."),
		})
		.Category(CATEGORY)
	.FunctionHelp("Return the result of a query.")
	.HelpTopic("https://dev.mysql.com/doc/c-api/8.0/en/mysql-query.html")
);
LPOPER4 WINAPI xll_mysql_query(HANDLEX h, const LPOPER4 psql)
{
#pragma XLLEXPORT
	static OPER4 result;

	try {
		handle<mysql::connect> h_(h);
		ensure(h_);
		auto sql = to_string(*psql);
		if (0 != h_->query(sql.c_str())) {
			throw std::runtime_error(h_->error());
		}
		MYSQL_RES* res = mysql_store_result(*h_);
		MYSQL_ROW ri = mysql_fetch_row(res);
	}
	catch (std::exception& ex) {
		XLL_ERROR(ex.what());
	}

	return &result;
}