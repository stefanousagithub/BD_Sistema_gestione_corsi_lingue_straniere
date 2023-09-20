#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defines.h"

struct insegn{
    int codice;
    char username[46];
    char nome[46];
    char indirizzo[46];
    char nazionalita[46];
};

static size_t func_ins(MYSQL *conn, MYSQL_STMT *stmt, struct insegn ** ins)
{
	int status;
	size_t row = 0;
	MYSQL_BIND param[5];

	int codice;
    char username[46];
    char nome[46];
    char indirizzo[46];
    char nazionalita[46];
    my_bool is_null1, is_null2;


	if (mysql_stmt_store_result(stmt)) {
		fprintf(stderr, " mysql_stmt_execute(), 1 failed\n");
		fprintf(stderr, " %s\n", mysql_stmt_error(stmt));
		exit(0);
	}

	*ins = malloc(mysql_stmt_num_rows(stmt) * sizeof(struct insegn));

	memset(param, 0, sizeof(param));
	param[0].buffer_type = MYSQL_TYPE_LONG;
	param[0].buffer = &codice;
	param[0].buffer_length = sizeof(codice);
    
    param[1].buffer_type = MYSQL_TYPE_VAR_STRING;
	param[1].buffer = username;
	param[1].buffer_length = 46;

	param[2].buffer_type = MYSQL_TYPE_VAR_STRING;
	param[2].buffer = nome;
	param[2].buffer_length = 46;

	param[3].buffer_type = MYSQL_TYPE_VAR_STRING;
	param[3].buffer = indirizzo;
	param[3].buffer_length = 46;
    param[3].is_null = &is_null1;

	param[4].buffer_type = MYSQL_TYPE_VAR_STRING;
	param[4].buffer = nazionalita;
	param[4].buffer_length = 46;
    param[4].is_null = &is_null2;

	
	if(mysql_stmt_bind_result(stmt, param)) {
		finish_with_stmt_error(conn, stmt, "Unable to bind column parameters\n", true);
	}
	
	/* assemble course general information */
	while (true) {
		status = mysql_stmt_fetch(stmt);
		if (status == 1 || status == MYSQL_NO_DATA){
			break;
        }
        (*ins)[row].codice = codice;
        strcpy((*ins)[row].username, username);
		strcpy((*ins)[row].nome, nome);
        strcpy((*ins)[row].indirizzo, indirizzo);
        strcpy((*ins)[row].nazionalita, nazionalita);
		row++;
	}
				
	return row;
}


static void report_insegnante(MYSQL *conn) {
	MYSQL_STMT *prepared_stmt;
    MYSQL_BIND param[1];
	int status;
    struct insegn* ins;
	bool first = true;
    int index = 0;
	char header[512];

	// Prepare stored procedure call
	if(!setup_prepared_stmt(&prepared_stmt, "call report_insegnante(?)", conn)) {
		finish_with_stmt_error(conn, prepared_stmt, "Unable to initialize career report statement\n", false);
	}
	
		// Prepare parameters
	memset(param, 0, sizeof(param));

	param[0].buffer_type =  MYSQL_TYPE_VAR_STRING;
	param[0].buffer = conf.username;
	param[0].buffer_length = strlen(conf.username);

	if (mysql_stmt_bind_param(prepared_stmt, param) != 0) {
		finish_with_stmt_error(conn, prepared_stmt, "Could not bind parameters for report\n", true);
	}
	
		// Run procedure
	if (mysql_stmt_execute(prepared_stmt) != 0) {
		print_stmt_error(prepared_stmt, "An error occurred while retrieving the career report.");
		goto out;
	}

	// We have multiple result sets here!
	printf("Teacher Report\n");
	do {
		// Skip OUT variables (although they are not present in the procedure...)
		if(conn->server_status & SERVER_PS_OUT_PARAMS) {
			goto next;
		}

		if(first) {
			func_ins(conn, prepared_stmt, &ins);
			first = false;
		} else {
			sprintf(header, "\nTeacher code: %d\nUsername: %s\nName: %s\nAddress: %s\nNazionality: %s\n", ins[index].codice, ins[index].username, ins[index].nome, ins[index].indirizzo, ins[index].nazionalita);
			dump_result_set(conn, prepared_stmt, header);
			index++;
		}

		// more results? -1 = no, >0 = error, 0 = yes (keep looking)
	    next:
		status = mysql_stmt_next_result(prepared_stmt);
		if (status > 0)
			finish_with_stmt_error(conn, prepared_stmt, "Unexpected condition", true);
		
	} while (status == 0);

    out:
	mysql_stmt_close(prepared_stmt);
}


void run_as_insegnante(MYSQL *conn)
{
	char options[2] = {'1','2'};
	char op;
	
	printf("Switching to Teacher role...\n");

	if(!parse_config("users/insegnante.json", &conf)) {
		fprintf(stderr, "Unable to load segreteria configuration\n");
		exit(EXIT_FAILURE);
	}

	if(mysql_change_user(conn, conf.db_username, conf.db_password, conf.database)) {
		fprintf(stderr, "mysql_change_user() failed\n");
		exit(EXIT_FAILURE);
	}

	while(true) {
		printf("\033[2J\033[H");
		printf("*** What should I do for you? ***\n\n");
		printf("1) Get weekly report\n");
		printf("2) Exit\n");

		op = multiChoice("Select an option: ", options, 2);

		switch(op) {
			case '1':
				report_insegnante(conn);
				break;
			case '2':
				return;
				
			default:
				fprintf(stderr, "Invalid condition at %s:%d\n", __FILE__, __LINE__);
				abort();
		}

		getchar();
	}
}
