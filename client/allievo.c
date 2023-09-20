#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defines.h"

static void print_info(MYSQL *conn)
{
	MYSQL_STMT *prepared_stmt;
	MYSQL_BIND param[1];

    if(!setup_prepared_stmt(&prepared_stmt, "Select matricola, username, nome, cognome from allievo where username = ?", conn)) {
		finish_with_stmt_error(conn, prepared_stmt, "Unable to initialize printing of student statement\n", false);
	}
		// Prepare parameters
	memset(param, 0, sizeof(param));

	param[0].buffer_type =  MYSQL_TYPE_VAR_STRING;
	param[0].buffer = conf.username;
	param[0].buffer_length = strlen(conf.username);

	if (mysql_stmt_bind_param(prepared_stmt, param) != 0) {
		finish_with_stmt_error(conn, prepared_stmt, "Could not bind parameters for printing student\n", true);
	}

			// Run procedure
	if (mysql_stmt_execute(prepared_stmt) != 0) {
		print_stmt_error(prepared_stmt, "An error occurred while printing student.");
		goto out;
	}

    dump_result_set(conn, prepared_stmt, "\nStudent info: ");
    mysql_stmt_next_result(prepared_stmt);
    //close statement
    out:
        mysql_stmt_close(prepared_stmt);
}

void run_as_allievo(MYSQL *conn)
{
	//Get basical information about Student
	printf("Switching to Student role...\n");

	if(!parse_config("users/allievo.json", &conf)) {
		fprintf(stderr, "Unable to load student configuration\n");
		exit(EXIT_FAILURE);
	}

	if(mysql_change_user(conn, conf.db_username, conf.db_password, conf.database)) {
		fprintf(stderr, "mysql_change_user() failed\n");
		exit(EXIT_FAILURE);
	}

	print_info(conn);

	return;
}
