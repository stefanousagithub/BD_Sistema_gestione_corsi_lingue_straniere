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



static void reset_anno(MYSQL *conn)
{
	MYSQL_STMT *prepared_stmt;
    char options[2] = {'y','n'};
	char op;
    
    //Confirmation message
    printf("*** Are you sure?(y/n): ***\n");
    op = multiChoice("Select an option: ", options, 2);

    switch(op) {
        case 'y':
            break;
        case 'n':
            printf("Operation stopped\n");
            return;
        default:
            fprintf(stderr, "Could not parse the type string\n");
            abort();
    }
    
	// Prepare stored procedure call
	if(!setup_prepared_stmt(&prepared_stmt, "call reset_anno", conn)) {
		finish_with_stmt_error(conn, prepared_stmt, "Unable to initialize reset year statement\n", false);
	}

	// Run procedure
	if (mysql_stmt_execute(prepared_stmt) != 0) {
		print_stmt_error(prepared_stmt, "An error occurred while the reset year.");
		goto out;
	}
	
	printf("Operation correctly executed!\n");

    //close statement
    out:
	mysql_stmt_close(prepared_stmt);
}



static size_t func_ins(MYSQL *conn, MYSQL_STMT *stmt, struct insegn ** ins)
{
	int status;
	size_t row = 0;
	MYSQL_BIND param[5];

    //Output from stored procedures about teachers
	int codice;
    char username[46];
    char nome[46];
    char indirizzo[46];
    char nazionalita[46];
    
    //for null results
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
	
	/* assemble Teacher general information */
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


static void report_amministratore(MYSQL *conn) {
	MYSQL_STMT *prepared_stmt;
	int status;
    struct insegn* ins;
	bool first = true;
    int index = 0;
	char header[512];

	// Print generic information useful for the function
	if(!setup_prepared_stmt(&prepared_stmt, "call report_amministratore", conn)) {
		finish_with_stmt_error(conn, prepared_stmt, "Unable to initialize report statement\n", false);
	}
	
		// Run procedure
	if (mysql_stmt_execute(prepared_stmt) != 0) {
		print_stmt_error(prepared_stmt, "An error occurred while retrieving the  report.");
		goto out;
	}

	printf("Administration Report\n");
	do {
		// Skip OUT variables (although they are not present in the procedure...)
		if(conn->server_status & SERVER_PS_OUT_PARAMS) {
			goto next;
		}

		//first select about teacher's information
		if(first) {
			func_ins(conn, prepared_stmt, &ins);
			first = false;
		} else {
			sprintf(header, "\nTeacher code: %d\nUsername: %s\nName: %s\nAddress: %s\nNazionality: %s\n", ins[index].codice, ins[index].username, ins[index].nome, ins[index].indirizzo, ins[index].nazionalita);
            //print results
			dump_result_set(conn, prepared_stmt, header);
			index++;
		}

		// more results? -1 = no, >0 = error, 0 = yes (keep looking)
	    next:
		status = mysql_stmt_next_result(prepared_stmt);
		if (status > 0)
			finish_with_stmt_error(conn, prepared_stmt, "Unexpected condition", true);
		
	} while (status == 0);

    //close statement
    out:
	mysql_stmt_close(prepared_stmt);
}









static void registra_allievo(MYSQL *conn)
{
	MYSQL_STMT *prepared_stmt;
	MYSQL_BIND param[8];

	// Input for the registration routine
    char username[46];
    char pass[46];
    char nome[46];
    char cognome[46];
    char telefono[46];
    long telefono_num;
    MYSQL_TIME date_time = {};
    char  date[21];
    char indirizzo[46];
    
    //Input
    int matricola;


	// Get the required information
    printf("\nUsername: ");
    getInput(46, username, false);
    printf("Password: ");
    getInput(46, pass, false);
    printf("Nome: ");
    getInput(46, nome, false);
    printf("Cognome: ");
    getInput(46, cognome, false);
    printf("Telefono: ");
    getInput(46, telefono, false);
    printf("Date (YYYY/MM/DD): ");
    getInput(46, date, false);
    printf("Indirizzo: ");
    getInput(46, indirizzo, false);
    
    telefono_num = strtol(telefono, NULL, 10);

    if(convert_date(date, &date_time) == -1){
        fprintf(stderr, "Could not parse date string\n");
        abort();
    }
    
	// Prepare stored procedure call
	if(!setup_prepared_stmt(&prepared_stmt, "call iscrivi_allievo(?, ?, ?, ?, ?, ?, ?, ?)", conn)) {
		finish_with_stmt_error(conn, prepared_stmt, "Unable to initialize student's registration statement\n", false);
	}

	// Prepare parameters
	memset(param, 0, sizeof(param));


	param[0].buffer_type =  MYSQL_TYPE_VAR_STRING;
	param[0].buffer = username;
	param[0].buffer_length = strlen(username);
    
    param[1].buffer_type =  MYSQL_TYPE_VAR_STRING;
	param[1].buffer = pass;
	param[1].buffer_length = strlen(pass);
    
    param[2].buffer_type = MYSQL_TYPE_VAR_STRING;
	param[2].buffer = nome;
	param[2].buffer_length = strlen(nome);

    param[3].buffer_type = MYSQL_TYPE_VAR_STRING;
	param[3].buffer = cognome;
	param[3].buffer_length = strlen(cognome);

	param[4].buffer_type =  MYSQL_TYPE_LONGLONG;
	param[4].buffer = &telefono_num;
	param[4].buffer_length = sizeof(telefono_num);

	param[5].buffer_type =  MYSQL_TYPE_DATE;
	param[5].buffer = &date_time;
	param[5].buffer_length = sizeof(date_time);
    
    param[6].buffer_type = MYSQL_TYPE_VAR_STRING;
	param[6].buffer = indirizzo;
	param[6].buffer_length = strlen(indirizzo);

	param[7].buffer_type = MYSQL_TYPE_LONG; // OUT
	param[7].buffer = &matricola;
	param[7].buffer_length = sizeof(matricola);

	if (mysql_stmt_bind_param(prepared_stmt, param) != 0) {
		finish_with_stmt_error(conn, prepared_stmt, "Could not bind parameters for student's registration\n", true);
	}
	

	// Run procedure
	if (mysql_stmt_execute(prepared_stmt) != 0) {
		print_stmt_error(prepared_stmt, "An error occurred while student's registration.");
		goto out;
	}

	// Get back the ID of the newly-added student
	memset(param, 0, sizeof(param));
	param[0].buffer_type = MYSQL_TYPE_LONG; // OUT
	param[0].buffer = &matricola;
	param[0].buffer_length = sizeof(matricola);
	
	if(mysql_stmt_bind_result(prepared_stmt, param)) {
		finish_with_stmt_error(conn, prepared_stmt, "Could not retrieve output parameter", true);
	}
	
	// Retrieve output parameter
	if(mysql_stmt_fetch(prepared_stmt)) {
		finish_with_stmt_error(conn, prepared_stmt, "Could not buffer results", true);
	}

	printf("Student's registration correctly added with ID %d...\n", matricola);

    out:
	mysql_stmt_close(prepared_stmt);
}


static void registra_insegnante(MYSQL *conn)
{
	MYSQL_STMT *prepared_stmt;
	MYSQL_BIND param[6];

	// Input
    char username[46];
    char pass[46];
    char nome[46];
    char indirizzo[46];
    char nazionalita[46];
    
    //output
    int codice;

	// Get the required information
    printf("\nUsername: ");
    getInput(46, username, false);
    printf("Password: ");
    getInput(46, pass, false);
    printf("Nome: ");
    getInput(46, nome, false);
    printf("Indirizzo: ");
    getInput(46, indirizzo, false);
    printf("Nazionalita: ");
    getInput(46, nazionalita, false);

	// Prepare stored procedure call
	if(!setup_prepared_stmt(&prepared_stmt, "call iscrivi_insegnante(?, ?, ?, ?, ?, ?)", conn)) {
		finish_with_stmt_error(conn, prepared_stmt, "Unable to initialize teacher's registration statement\n", false);
	}

    
	// Prepare parameters
	memset(param, 0, sizeof(param));


	param[0].buffer_type =  MYSQL_TYPE_VAR_STRING;
	param[0].buffer = username;
	param[0].buffer_length = strlen(username);
    
    param[1].buffer_type = MYSQL_TYPE_VAR_STRING;
	param[1].buffer = pass;
	param[1].buffer_length = strlen(pass);
    
    param[2].buffer_type = MYSQL_TYPE_VAR_STRING;
	param[2].buffer = nome;
	param[2].buffer_length = strlen(nome);

    param[3].buffer_type = MYSQL_TYPE_VAR_STRING;
	param[3].buffer = indirizzo;
	param[3].buffer_length = strlen(indirizzo);

	param[4].buffer_type =  MYSQL_TYPE_VAR_STRING;
	param[4].buffer = nazionalita;
	param[4].buffer_length = strlen(nazionalita);

	param[5].buffer_type = MYSQL_TYPE_LONG; // OUT
	param[5].buffer = &codice;
	param[5].buffer_length = sizeof(codice);

	if (mysql_stmt_bind_param(prepared_stmt, param) != 0) {
		finish_with_stmt_error(conn, prepared_stmt, "Could not bind parameters for teacher's registration\n", true);
	}

	// Run procedure
	if (mysql_stmt_execute(prepared_stmt) != 0) {
		print_stmt_error(prepared_stmt, "An error occurred while teacher's registration.");
		goto out;
	}

	// Get back the ID of the newly-added student
	memset(param, 0, sizeof(param));
	param[0].buffer_type = MYSQL_TYPE_LONG; // OUT
	param[0].buffer = &codice;
	param[0].buffer_length = sizeof(codice);
	
	if(mysql_stmt_bind_result(prepared_stmt, param)) {
		finish_with_stmt_error(conn, prepared_stmt, "Could not retrieve output parameter", true);
	}
	
	// Retrieve output parameter
	if(mysql_stmt_fetch(prepared_stmt)) {
		finish_with_stmt_error(conn, prepared_stmt, "Could not buffer results", true);
	}

	printf("Teacher's registration correctly added with ID %d...\n", codice);

    out:
	mysql_stmt_close(prepared_stmt);
}




static void aggiungi_lezioneprivata(MYSQL *conn)
{
	MYSQL_STMT *prepared_stmt;
	MYSQL_BIND param[5];

	// Input
    MYSQL_TIME inizio_time = {};
    char  inizio[21];
    MYSQL_TIME fine_time = {};
    char  fine[21];
    int insegnante_int;
    char insegnante[46];
    int allievo_int;
    char allievo[46];
    
    //Output of the stored procedure
    int codice;
    
    
    // Print generic information useful for the function
    if(!setup_prepared_stmt(&prepared_stmt, "Select codice, username, nome from insegnante", conn)) {
		finish_with_stmt_error(conn, prepared_stmt, "Unable to initialize printing of teachers statement\n", false);
	}
			// Run procedure
	if (mysql_stmt_execute(prepared_stmt) != 0) {
		print_stmt_error(prepared_stmt, "An error occurred while printing teachers.");
		goto out;
	}
	
    dump_result_set(conn, prepared_stmt, "\nTeachers: ");
    mysql_stmt_next_result(prepared_stmt);
    mysql_stmt_close(prepared_stmt);
    
    
    if(!setup_prepared_stmt(&prepared_stmt, "Select matricola, username, nome, cognome from allievo", conn)) {
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
    printf("\nBegin (YYYY/MM/DD hh:mm): ");
	getInput(46, inizio, false);
	printf("End (YYYY/MM/DD hh:mm): ");
	getInput(46, fine, false);
    printf("Teacher: ");
    getInput(46, insegnante, false);
    printf("Student: ");
    getInput(46, allievo, false);

    //cast char to int AND convert char to MYSQL_TIME
    insegnante_int = atoi(insegnante);
    allievo_int = atoi(allievo);
    if(convert_timestamp(inizio, &inizio_time) == -1){
        fprintf(stderr, "Could not parse the begin private lesson string\n");
        abort();
    }
    if(convert_timestamp(fine, &fine_time) == -1){
        fprintf(stderr, "Could not parse the end private lesson string\n");
        abort();
    }

	// Prepare stored procedure call
	if(!setup_prepared_stmt(&prepared_stmt, "call aggiungi_lezioneprivata(?, ?, ?, ?, ?)", conn)) {
		finish_with_stmt_error(conn, prepared_stmt, "Unable to initialize private lesson insertion statement\n", false);
	}

    
	// Prepare parameters
	memset(param, 0, sizeof(param));


	param[0].buffer_type =  MYSQL_TYPE_TIMESTAMP;
	param[0].buffer = &inizio_time;
	param[0].buffer_length = sizeof(inizio_time);
    
    param[1].buffer_type = MYSQL_TYPE_TIMESTAMP;
	param[1].buffer = &fine_time;
	param[1].buffer_length = sizeof(fine_time);

    param[2].buffer_type = MYSQL_TYPE_LONG;
	param[2].buffer = &insegnante_int;
	param[2].buffer_length = sizeof(insegnante_int);
    
	param[3].buffer_type =  MYSQL_TYPE_LONG;
	param[3].buffer = &allievo_int;
	param[3].buffer_length = sizeof(allievo_int);

	param[4].buffer_type = MYSQL_TYPE_LONG; // OUT
	param[4].buffer = &codice;
	param[4].buffer_length = sizeof(codice);

	if (mysql_stmt_bind_param(prepared_stmt, param) != 0) {
		finish_with_stmt_error(conn, prepared_stmt, "Could not bind parameters for private lesson insertion\n", true);
	}

	// Run procedure
	if (mysql_stmt_execute(prepared_stmt) != 0) {
		print_stmt_error(prepared_stmt, "An error occurred while adding the private lesson.");
		goto out;
	}

	// Get back the ID of the newly-added student
	memset(param, 0, sizeof(param));
	param[0].buffer_type = MYSQL_TYPE_LONG; // OUT
	param[0].buffer = &codice;
	param[0].buffer_length = sizeof(codice);
	
	if(mysql_stmt_bind_result(prepared_stmt, param)) {
		finish_with_stmt_error(conn, prepared_stmt, "Could not retrieve output parameter", true);
	}
	
	// Retrieve output parameter
	if(mysql_stmt_fetch(prepared_stmt)) {
		finish_with_stmt_error(conn, prepared_stmt, "Could not buffer results", true);
	}

	printf("Private lesson correctly added with ID %d...\n", codice);

    out:
	mysql_stmt_close(prepared_stmt);
}





static void crea_attivita(MYSQL *conn)
{
	MYSQL_STMT *prepared_stmt;
	MYSQL_BIND param[8];
    
    
    //print corsi, insegnqanti, allievi; IMPORTANTE

	// Input
    MYSQL_TIME inizio_time = {};
    char  inizio[21];
    MYSQL_TIME fine_time = {};
    char  fine[21];
    int tipo_int = 0;
    char regista[46];
    char film[46];
    char argomento[46];
    char conferenziere[46];
    
    char options[5] = {'1','2'};
    char op;
	
    
    //Output of the stored procedure
    int codice;

	// Get activity type 
    printf("\n*** What type of activity? ***\n\n");
    printf("1) Proiezione\n");
    printf("2) Conferenza\n");

    op = multiChoice("Select an option", options, 2);

    switch(op) {
        case '1':
            tipo_int = 1;
            printf("Director: ");
            getInput(46, regista, false);
            printf("Film: ");
            getInput(46, film, false);
            
            //set conference parameters null
            strcpy(argomento,"/");
            strcpy(conferenziere, "/");
            break;
				
        case '2':
            tipo_int = 2;
            printf("Sudject: ");
            getInput(46, argomento, false);
            printf("Speaker: ");
            getInput(46, conferenziere, false);
            
            //set Proiezione parameters null
            strcpy(regista, "/");
            strcpy(film, "/");
            break;
				
        default:
            fprintf(stderr, "Could not parse the type string\n");
            abort();
		}
    
    printf("Begin (YYYY/MM/DD hh:mm): ");
	getInput(46, inizio, false);
	printf("End (YYYY/MM/DD hh:mm): ");
	getInput(46, fine, false);
    
    //convert char to MYSQL_TIME
    if(convert_timestamp(inizio, &inizio_time) == -1){
        fprintf(stderr, "Could not parse the begin activity string\n");
        abort();
    }
    if(convert_timestamp(fine, &fine_time) == -1){
        fprintf(stderr, "Could not parse the end activity string\n");
        abort();
    }

	// Prepare stored procedure call
	if(!setup_prepared_stmt(&prepared_stmt, "call crea_attivita(?, ?, ?, ?, ?, ?, ?, ?)", conn)) {
		finish_with_stmt_error(conn, prepared_stmt, "Unable to initialize activity insertion statement\n", false);
	}

    
	// Prepare parameters
	memset(param, 0, sizeof(param));


	param[0].buffer_type =  MYSQL_TYPE_TIMESTAMP;
	param[0].buffer = &inizio_time;
	param[0].buffer_length = sizeof(inizio_time);
    
    param[1].buffer_type = MYSQL_TYPE_TIMESTAMP;
	param[1].buffer = &fine_time;
	param[1].buffer_length = sizeof(fine_time);
    
    
	param[2].buffer_type = MYSQL_TYPE_LONG;
	param[2].buffer = &tipo_int;
	param[2].buffer_length = sizeof(tipo_int);

    param[3].buffer_type = MYSQL_TYPE_VAR_STRING;
	param[3].buffer = regista;
	param[3].buffer_length = strlen(regista);

	param[4].buffer_type =  MYSQL_TYPE_VAR_STRING;
	param[4].buffer = film;
	param[4].buffer_length = strlen(film);
        
    param[5].buffer_type = MYSQL_TYPE_VAR_STRING;
	param[5].buffer = argomento;
	param[5].buffer_length = strlen(argomento);

	param[6].buffer_type =  MYSQL_TYPE_VAR_STRING;
	param[6].buffer = conferenziere;
	param[6].buffer_length = strlen(conferenziere);

	param[7].buffer_type = MYSQL_TYPE_LONG; // OUT
	param[7].buffer = &codice;
	param[7].buffer_length = sizeof(codice);

	if (mysql_stmt_bind_param(prepared_stmt, param) != 0) {
		finish_with_stmt_error(conn, prepared_stmt, "Could not bind parameters for activity insertion\n", true);
	}

	// Run procedure
	if (mysql_stmt_execute(prepared_stmt) != 0) {
		print_stmt_error(prepared_stmt, "An error occurred while adding the activity.");
		goto out;
	}

	// Get back the ID of the newly-added student
	memset(param, 0, sizeof(param));
	param[0].buffer_type = MYSQL_TYPE_LONG; // OUT
	param[0].buffer = &codice;
	param[0].buffer_length = sizeof(codice);
	
	if(mysql_stmt_bind_result(prepared_stmt, param)) {
		finish_with_stmt_error(conn, prepared_stmt, "Could not retrieve output parameter", true);
	}
	
	// Retrieve output parameter
	if(mysql_stmt_fetch(prepared_stmt)) {
		finish_with_stmt_error(conn, prepared_stmt, "Could not buffer results", true);
	}

	printf("Activity correctly added with ID %d...\n", codice);

    out:
	mysql_stmt_close(prepared_stmt);
}



static void crea_corso(MYSQL *conn)
{
	MYSQL_STMT *prepared_stmt;
	MYSQL_BIND param[2];
    
    // Input
	char livello[46];
    
    //Output of the stored procedure
    int codice;
    
    // Print generic information useful for the function
    if(!setup_prepared_stmt(&prepared_stmt, "Select nome from livello", conn)) {
		finish_with_stmt_error(conn, prepared_stmt, "Unable to initialize printing of levels statement\n", false);
	}
	
			// Run procedure
	if (mysql_stmt_execute(prepared_stmt) != 0) {
		print_stmt_error(prepared_stmt, "An error occurred while printing levels.");
		goto out;
	}
    dump_result_set(conn, prepared_stmt, "\nLEVELS: ");
    mysql_stmt_next_result(prepared_stmt);
	mysql_stmt_close(prepared_stmt);

	// Get the required information
	printf("\nLevel: ");
	getInput(46, livello, false);
    
	// Prepare stored procedure call
	if(!setup_prepared_stmt(&prepared_stmt, "call crea_corso(?, ?)", conn)) {
		finish_with_stmt_error(conn, prepared_stmt, "Unable to initialize course insertion statement\n", false);
	}

    
	// Prepare parameters
	memset(param, 0, sizeof(param));

	param[0].buffer_type =  MYSQL_TYPE_VAR_STRING;
	param[0].buffer = livello;
	param[0].buffer_length = strlen(livello);


	param[1].buffer_type = MYSQL_TYPE_LONG; // OUT
	param[1].buffer = &codice;
	param[1].buffer_length = sizeof(codice);

	if (mysql_stmt_bind_param(prepared_stmt, param) != 0) {
		finish_with_stmt_error(conn, prepared_stmt, "Could not bind parameters for course insertion\n", true);
	}

	// Run procedure
	if (mysql_stmt_execute(prepared_stmt) != 0) {
		print_stmt_error(prepared_stmt, "An error occurred while adding the course.");
		goto out;
	}

	// Get back the ID of the newly-added student
	memset(param, 0, sizeof(param));
	param[0].buffer_type = MYSQL_TYPE_LONG; // OUT
	param[0].buffer = &codice;
	param[0].buffer_length = sizeof(codice);
	
	if(mysql_stmt_bind_result(prepared_stmt, param)) {
		finish_with_stmt_error(conn, prepared_stmt, "Could not retrieve output parameter", true);
	}
	
	// Retrieve output parameter
	if(mysql_stmt_fetch(prepared_stmt)) {
		finish_with_stmt_error(conn, prepared_stmt, "Could not buffer results", true);
	}

	printf("Course correctly added with ID %d...\n", codice);

    out:
	mysql_stmt_close(prepared_stmt);
}





static void aggiungi_lezione(MYSQL *conn)
{
	MYSQL_STMT *prepared_stmt;
	MYSQL_BIND param[6];

	// Input
	int corso_int;
    char corso[46];
	char livello[46];
    MYSQL_TIME inizio_time= {};
    char  inizio[21];
    MYSQL_TIME fine_time= {};
    char  fine[21];
    int insegnante_int;
    char insegnante[46];
    
    //Output of the stored procedure
    int numero;
    
    
    // Print generic information useful for the function
    if(!setup_prepared_stmt(&prepared_stmt, "Select * from corso", conn)) {
		finish_with_stmt_error(conn, prepared_stmt, "Unable to initialize printing of courses statement\n", false);
	}
	
			// Run procedure
	if (mysql_stmt_execute(prepared_stmt) != 0) {
		print_stmt_error(prepared_stmt, "An error occurred while printing courses.");
		goto out;
	}
    dump_result_set(conn, prepared_stmt, "\nCourses: ");
    mysql_stmt_next_result(prepared_stmt);
	mysql_stmt_close(prepared_stmt);
    
    if(!setup_prepared_stmt(&prepared_stmt, "Select codice, username, nome from insegnante", conn)) {
		finish_with_stmt_error(conn, prepared_stmt, "Unable to initialize printing of teachers statement\n", false);
	}
	
			// Run procedure
	if (mysql_stmt_execute(prepared_stmt) != 0) {
		print_stmt_error(prepared_stmt, "An error occurred while printing teachers.");
		goto out;
	}
    dump_result_set(conn, prepared_stmt, "\nTeachers: ");
    mysql_stmt_next_result(prepared_stmt);
	mysql_stmt_close(prepared_stmt);
    
    
	// Get the required information
	printf("\nCourse: ");
	getInput(46, corso, false);
	printf("Level: ");
	getInput(46, livello, false);
	printf("Teacher: ");
	getInput(46, insegnante, false);
    printf("Begin (YYYY/MM/DD hh:mm): ");
	getInput(46, inizio, false);
	printf("End (YYYY/MM/DD hh:mm): ");
	getInput(46, fine, false);
    
    //cast char to int AND cast char to MYSQL_TIME
    corso_int = atoi(corso);
    insegnante_int = atoi(insegnante);
    if(convert_timestamp(inizio, &inizio_time) == -1){
        fprintf(stderr, "Could not parse the begin lesson string\n");
        abort();
    }
    if(convert_timestamp(fine, &fine_time) == -1){
        fprintf(stderr, "Could not parse the end lesson string\n");
        abort();
    }

	// Prepare stored procedure call
	if(!setup_prepared_stmt(&prepared_stmt, "call aggiungi_lezione(?, ?, ?, ?, ?, ?)", conn)) {
		finish_with_stmt_error(conn, prepared_stmt, "Unable to initialize lesson insertion statement\n", false);
	}

    
	// Prepare parameters
	memset(param, 0, sizeof(param));

	param[0].buffer_type =  MYSQL_TYPE_LONG;
	param[0].buffer = &corso_int;
	param[0].buffer_length = sizeof(corso_int);

	param[1].buffer_type = MYSQL_TYPE_VAR_STRING;
	param[1].buffer = livello;
	param[1].buffer_length = strlen(livello);

	param[2].buffer_type =  MYSQL_TYPE_TIMESTAMP;
	param[2].buffer = &inizio_time;
	param[2].buffer_length = sizeof(inizio_time);
    
    param[3].buffer_type = MYSQL_TYPE_TIMESTAMP;
	param[3].buffer = &fine_time;
	param[3].buffer_length = sizeof(fine_time);

	param[4].buffer_type =  MYSQL_TYPE_LONG;
	param[4].buffer = &insegnante_int;
	param[4].buffer_length = sizeof(insegnante_int);

	param[5].buffer_type = MYSQL_TYPE_LONG; // OUT
	param[5].buffer = &numero;
	param[5].buffer_length = sizeof(numero);

	if (mysql_stmt_bind_param(prepared_stmt, param) != 0) {
		finish_with_stmt_error(conn, prepared_stmt, "Could not bind parameters for lesson insertion\n", true);
	}

	// Run procedure
	if (mysql_stmt_execute(prepared_stmt) != 0) {
		print_stmt_error(prepared_stmt, "An error occurred while adding the lesson.");
		goto out;
	}

	// Get back the ID of the newly-added student
	memset(param, 0, sizeof(param));
	param[0].buffer_type = MYSQL_TYPE_LONG; // OUT
	param[0].buffer = &numero;
	param[0].buffer_length = sizeof(numero);
	
	if(mysql_stmt_bind_result(prepared_stmt, param)) {
		finish_with_stmt_error(conn, prepared_stmt, "Could not retrieve output parameter", true);
	}
	
	// Retrieve output parameter
	if(mysql_stmt_fetch(prepared_stmt)) {
		finish_with_stmt_error(conn, prepared_stmt, "Could not buffer results", true);
	}

	printf("Lesson correctly added with ID %d...\n", numero);

    out:
	mysql_stmt_close(prepared_stmt);
}






void run_as_amministratore(MYSQL *conn)
{
	char options[9] = {'1','2', '3', '4', '5', '6', '7', '8', '9'};
	char op;
	
	printf("Switching to administrative role...\n");

	if(!parse_config("users/amministratore.json", &conf)) {
		fprintf(stderr, "Unable to load administrator configuration\n");
		exit(EXIT_FAILURE);
	}

	if(mysql_change_user(conn, conf.db_username, conf.db_password, conf.database)) {
		fprintf(stderr, "mysql_change_user() failed\n");
		exit(EXIT_FAILURE);
	}

	while(true) {
		printf("\033[2J\033[H");
		printf("*** What should I do for you? ***\n\n");
		printf("1) Add lesson\n");
		printf("2) Add course\n");
		printf("3) Create activity\n");
		printf("4) Add private lesson\n");
        printf("5) Register new teacher\n");
		printf("6) Register new student\n");
		printf("7) Teacher's report\n");
		printf("8) Start new scholastic year\n");
		printf("9) Exit\n");

		op = multiChoice("Select an option: ", options, 9);

		switch(op) {
			case '1':
				aggiungi_lezione(conn);
				break;
			case '2':
				crea_corso(conn);
				break;
			case '3':
				crea_attivita(conn);
				break;
			case '4':
				aggiungi_lezioneprivata(conn);
				break;
            case '5':
				registra_insegnante(conn);
				break;
			case '6':
				registra_allievo(conn);
				break;
			case '7':
				report_amministratore(conn);
				break;
			case '8':
				reset_anno(conn);
				break;
			case '9':
				return;
				
			default:
				fprintf(stderr, "Invalid condition at %s:%d\n", __FILE__, __LINE__);
				abort();
		}

		getchar();
	}
}
