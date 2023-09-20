#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defines.h"


static void partecipa_attivita(MYSQL *conn)
{
	MYSQL_STMT *prepared_stmt;
	MYSQL_BIND param[2];

	// Input
    int allievo_int;
    char allievo[46];
    int attivita_int;
    char attivita[46];

    // Print generic information useful for the function
    if(!setup_prepared_stmt(&prepared_stmt, "Select codice, tipo, inizio, fine from attivita", conn)) {
		finish_with_stmt_error(conn, prepared_stmt, "Unable to initialize the printing of activities statement\n", false);
	}

	if (mysql_stmt_execute(prepared_stmt) != 0) {
		print_stmt_error(prepared_stmt, "An error occurred while printing activities.");
		goto out;
	}

    dump_result_set(conn, prepared_stmt, "\nActivities: ");
    mysql_stmt_next_result(prepared_stmt);
    mysql_stmt_close(prepared_stmt);


    if(!setup_prepared_stmt(&prepared_stmt, "select matricola, username, nome, cognome from allievo", conn)) {
		finish_with_stmt_error(conn, prepared_stmt, "Unable to initialize printing of students statement\n", false);
	}
			// Run procedure
	if (mysql_stmt_execute(prepared_stmt) != 0) {
		print_stmt_error(prepared_stmt, "An error occurred while printing students.");
		goto out;
	}

    dump_result_set(conn, prepared_stmt, "\nStudents: ");
    mysql_stmt_next_result(prepared_stmt);
    mysql_stmt_close(prepared_stmt);



	// Get the required information
	printf("\nStudent: ");
	getInput(46, allievo, false);
	printf("Activity: ");
	getInput(46, attivita, false);

    allievo_int = atoi(allievo);
    attivita_int = atoi(attivita);

	// Prepare stored procedure call
	if(!setup_prepared_stmt(&prepared_stmt, "call partecipa_attivita(?, ?)", conn)) {
		finish_with_stmt_error(conn, prepared_stmt, "Unable to initialize activity partecipation statement\n", false);
	}


	// Prepare parameters
	memset(param, 0, sizeof(param));

	param[0].buffer_type =  MYSQL_TYPE_LONG;
	param[0].buffer = &allievo_int;
	param[0].buffer_length = sizeof(allievo_int);

	param[1].buffer_type =  MYSQL_TYPE_LONG;
	param[1].buffer = &attivita_int;
	param[1].buffer_length = sizeof(attivita_int);


	if (mysql_stmt_bind_param(prepared_stmt, param) != 0) {
		finish_with_stmt_error(conn, prepared_stmt, "Could not bind parameters for partecipation insertion\n", true);
	}

	// Run procedure
	if (mysql_stmt_execute(prepared_stmt) != 0) {
		print_stmt_error(prepared_stmt, "An error occurred while adding the partecipation.");
		goto out;
	}

	printf("Partecipation correctly added...\n");

    out:
	mysql_stmt_close(prepared_stmt);
}

static void aggiungi_assenza(MYSQL *conn)
{
	MYSQL_STMT *prepared_stmt;
	MYSQL_BIND param[4];

	// Input for the registration routine
	int corso_int;
    char corso[46];
	char livello[46];
    int numero_int;
    char numero[46];
    int allievo_int;
    char allievo[46];

                //View courses
    if(!setup_prepared_stmt(&prepared_stmt, "Select * from lezione", conn)) {
		finish_with_stmt_error(conn, prepared_stmt, "Unable to initialize printing of lessons statement\n", false);
	}
			// Run procedure
	if (mysql_stmt_execute(prepared_stmt) != 0) {
		print_stmt_error(prepared_stmt, "An error occurred while printing the lessons.");
		goto out;
	}

    dump_result_set(conn, prepared_stmt, "\nLessons: ");
    mysql_stmt_next_result(prepared_stmt);
    mysql_stmt_close(prepared_stmt);


            //View courses
    if(!setup_prepared_stmt(&prepared_stmt, "Select matricola, username, nome, cognome from allievo", conn)) {
		finish_with_stmt_error(conn, prepared_stmt, "Unable to initialize printing of students statement\n", false);
	}
			// Run procedure
	if (mysql_stmt_execute(prepared_stmt) != 0) {
		print_stmt_error(prepared_stmt, "An error occurred while printing the students.");
		goto out;
	}

    dump_result_set(conn, prepared_stmt, "\nStudents: ");
    mysql_stmt_next_result(prepared_stmt);
    mysql_stmt_close(prepared_stmt);


	// Get the required information
	printf("\nCourse: ");
	getInput(46, corso, false);
	printf("Level: ");
	getInput(46, livello, false);
	printf("Number of lesson: ");
	getInput(46, numero, false);
    printf("Student: ");
	getInput(46, allievo, false);

    corso_int = atoi(corso);
    numero_int = atoi(numero);
    allievo_int = atoi(allievo);

	// Prepare stored procedure call
	if(!setup_prepared_stmt(&prepared_stmt, "call aggiungi_assenza(?, ?, ?, ?)", conn)) {
		finish_with_stmt_error(conn, prepared_stmt, "Unable to initialize absence insertion statement\n", false);
	}


	// Prepare parameters
	memset(param, 0, sizeof(param));

	param[0].buffer_type =  MYSQL_TYPE_LONG;
	param[0].buffer = &corso_int;
	param[0].buffer_length = sizeof(corso_int);

	param[1].buffer_type = MYSQL_TYPE_VAR_STRING;
	param[1].buffer = livello;
	param[1].buffer_length = strlen(livello);

	param[2].buffer_type =  MYSQL_TYPE_LONG;
	param[2].buffer = &numero_int;
	param[2].buffer_length = sizeof(numero_int);

    param[3].buffer_type = MYSQL_TYPE_LONG;
	param[3].buffer = &allievo_int;
	param[3].buffer_length = sizeof(allievo_int);


	if (mysql_stmt_bind_param(prepared_stmt, param) != 0) {
		finish_with_stmt_error(conn, prepared_stmt, "Could not bind parameters for absence insertion\n", true);
	}

	// Run procedure
	if (mysql_stmt_execute(prepared_stmt) != 0) {
		print_stmt_error(prepared_stmt, "An error occurred while adding the absence.");
		goto out;
	}

	printf("Absence correctly added...\n");

    out:
	mysql_stmt_close(prepared_stmt);
}




static void registra_studente_corso(MYSQL *conn)
{
	MYSQL_STMT *prepared_stmt;
	MYSQL_BIND param[3];

	// Input for the registration routine
	int corso_int;
    char corso[46];
    char livello[46];
    int allievo_int;
    char allievo[46];

            //View courses
    if(!setup_prepared_stmt(&prepared_stmt, "Select * from corso", conn)) {
		finish_with_stmt_error(conn, prepared_stmt, "Unable to initialize printing of courses statement\n", false);
	}
			// Run procedure
	if (mysql_stmt_execute(prepared_stmt) != 0) {
		print_stmt_error(prepared_stmt, "An error occurred while printing of courses.");
		goto out;
	}

    dump_result_set(conn, prepared_stmt, "\nCourses: ");
    mysql_stmt_next_result(prepared_stmt);
    mysql_stmt_close(prepared_stmt);


            //View courses
    if(!setup_prepared_stmt(&prepared_stmt, "select matricola, username, nome, cognome from allievo", conn)) {
		finish_with_stmt_error(conn, prepared_stmt, "Unable to initialize printing of students statement\n", false);
	}
			// Run procedure
	if (mysql_stmt_execute(prepared_stmt) != 0) {
		print_stmt_error(prepared_stmt, "An error occurred while printing of students.");
		goto out;
	}

    dump_result_set(conn, prepared_stmt, "\nStudents: ");
    mysql_stmt_next_result(prepared_stmt);
    mysql_stmt_close(prepared_stmt);


	// Get the required information
	printf("\nCourse: ");
	getInput(46, corso, false);
	printf("Level: ");
	getInput(46, livello, false);
	printf("Student: ");
	getInput(46, allievo, false);

    //FORSE TIPO ERRORE SBAGLIATO
    corso_int = atoi(corso);
    allievo_int = atoi(allievo);

	// Prepare stored procedure call
	if(!setup_prepared_stmt(&prepared_stmt, "call iscrivi_allievo_corso(?, ?, ?)", conn)) {
		finish_with_stmt_error(conn, prepared_stmt, "Unable to register student course statement\n", false);
	}


	// Prepare parameters
	memset(param, 0, sizeof(param));

	param[0].buffer_type =  MYSQL_TYPE_LONG;
	param[0].buffer = &corso_int;
	param[0].buffer_length = sizeof(corso_int);

	param[1].buffer_type = MYSQL_TYPE_VAR_STRING;
	param[1].buffer = livello;
	param[1].buffer_length = strlen(livello);

	param[2].buffer_type =  MYSQL_TYPE_LONG;
	param[2].buffer = &allievo_int;
	param[2].buffer_length = sizeof(allievo_int);


	if (mysql_stmt_bind_param(prepared_stmt, param) != 0) {
		finish_with_stmt_error(conn, prepared_stmt, "Could not bind parameters for register student course\n", true);
	}

	// Run procedure
	if (mysql_stmt_execute(prepared_stmt) != 0) {
		print_stmt_error(prepared_stmt, "An error occurred while register student course.");
		goto out;
	}

	printf("Registration student course correctly executed\n");

    out:
	mysql_stmt_close(prepared_stmt);
}


void run_as_segreteria(MYSQL *conn)
{
	char options[4] = {'1','2', '3', '4'};
	char op;

	printf("Switching to Secretariat role...\n");

	if(!parse_config("users/segreteria.json", &conf)) {
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
		printf("1) Register student course\n");
		printf("2) Add student's absence\n");
		printf("3) Partecipate student to activity\n");
		printf("4) Exit\n");

		op = multiChoice("Select an option: ", options, 4);

		switch(op) {
			case '1':
				registra_studente_corso(conn);
				break;
			case '2':
				aggiungi_assenza(conn);
				break;
			case '3':
				partecipa_attivita(conn);
				break;
			case '4':
				return;

			default:
				fprintf(stderr, "Invalid condition at %s:%d\n", __FILE__, __LINE__);
				abort();
		}

		getchar();
	}
}
