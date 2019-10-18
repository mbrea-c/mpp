/* mpp: MIPS Preprocessor
 * meant to solve the glaring
 * lack of preprocessor in the
 * SPIM MIPS simulator */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#define MAXCOUNT 1000
#define MAXTOKEN 1000
#define MAXREPL  1000
#define MAXNAME  1000
#define MAXVARS  1000

typedef struct macrovar {
	char *name;
	struct macrovar *nxt;
} macrovar;

typedef struct macro {
	char *name; //name to be replaced
	char *repl; //replacement string
	macrovar *vars;
} macro;
enum verbosity { QUIET=0, DEFAULT=1, VERBOSE=2, DEBUG=3 };

void print_log(int verbosity, char *msg, ...);
void fatal(char *msg);
void fatalf(char *msg, ...);
void __debug(FILE *fp);

macro *findm(char *); //returns NULL on not found, repl otherwise
int insertm(char *name, char *repl, macrovar *vars); //returns 0 on error, 1 on success
int gettoken(FILE *fp); 
int sgettoken(char *str, int *ind);
void parsem(FILE *fp); //parse file and store macros
void substm(FILE *ifp, FILE *ofp);
void replacevars(char *out, macro *mcr, char **vars);
char *findvar(char *str, macrovar *varnames, char **vars);
macro *getm(FILE *fp);
void printallmacros();
void printallvars(macrovar *m);

char token[MAXTOKEN];
int g_verbosity = DEBUG;

int main(int argc, char **argv)
{
	FILE *inpfile, *outfile; 

	if (argc != 3) {
		fatalf("usage: mpp <input file> <output file>\n");
	}
	if ((inpfile = fopen(argv[1], "r")) == NULL) {
		printf("%s\n", argv[1]);
		fatalf("error: file %s couldn't be opened\n", argv[1]);
	}
	if ((outfile = fopen(argv[2], "w")) == NULL) {
		fatalf("error: file %s couldn't be opened\n", argv[2]);
	}

	/*__debug(inpfile);*/
	parsem(inpfile);
	if ((inpfile = fopen(argv[1], "r")) == NULL) {
		fatal("error: file couldn't be opened\n");
	}
	printallmacros();
	substm(inpfile, outfile);

	return 0;
}

void fatal(char *msg)
{
	fprintf(stderr, msg);
	exit(1);
}

void fatalf(char *msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	vfprintf(stderr, msg, ap);
	exit(1);
}

void __debug(FILE *fp)
{
	int c;
	while ((c = gettoken(fp)) != EOF) {
		printf(token);
	}
}

void print_log(int verbosity, char *msg, ...)
{
	if (verbosity >= g_verbosity) {
		va_list ap;
		va_start(ap, msg);
		vprintf(msg, ap);
	}
}

void printallvars(macrovar *m)
{
	printf("Printing all vars...\n");
	for ( ; m->nxt != NULL; m = m->nxt) {
		printf("%s\n", m->name);
	}
	printf("%s\n", m->name);
}


macro *macrostore[MAXCOUNT];
int count = 0;

void printallmacros()
{
	int i;
	printf("PRINTING MACROS...\n");
	for (i = 0; i < count; i++) {
		printf("macro: %s\ntext: %s\n-----------------\n", macrostore[i]->name, macrostore[i]->repl);
	}
}

void replacevars(char *out, macro *mcr, char **vars)
{
	strcpy(out, "");
	char *tmp;
	int ind, c;
	ind = 0;
	printf("DEBUG: Entering replacevars ---------------------\n");
	while ((c = sgettoken(mcr->repl, &ind)) != EOF) {
		printf("DEBUG: c = %c || token = %s\n", c, token);
		if (c == '$') {
			printf("DEBUG: c = %c\n", c );
			if ((tmp = findvar(token, mcr->vars, vars)) != NULL) {
				strcat(out, tmp);
			} else {
				strcat(out, token);
			}
		} else {
			strcat(out, token);
		}
	}
	printf("DEBUG: out = %s\n", out);
}

char *findvar(char *str, macrovar *varnames, char **vars)
{
	printf("DEBUG: Entering findvar ----------------------\n");
	printallvars(varnames);
	if (varnames == NULL) return NULL;
	for ( ; varnames->nxt != NULL; varnames=varnames->nxt, vars++) {
		int debugstat = 0;
		printf("debugpoint %d\n", debugstat++);
		printf("DEBUG: *vars = %s\n", *vars );
		printf("DEBUG: str = %s\n", str );
		printf("DEBUG: varnames->name = %s\n", varnames->name );
		printf("DEBUG: varnames->nxt = %p\n", varnames->nxt );
		if (strcmp(str, varnames->name) == 0) {
			printf("debugpoint %d\n", debugstat++);
			return *vars;
		}
	}
	if (strcmp(str, varnames->name) == 0) {
		return *vars;
	}
	printf("DEBUG: Exiting findvar ----------------------\n");
	return NULL;
}

void parsem(FILE *fp)
{
	int c, state;
	char *tempm, *name;
	macrovar *vars, *lastvar;

	tempm = malloc(MAXREPL);
	name = malloc(MAXNAME);
	vars = NULL;
	state = 0;
	strcpy(tempm, "");
	printf("DEBUG: Entering parsem ---------------------\n", state, c, c, token);
	while ((c = gettoken(fp)) != EOF) {
		printf("DEBUG: state: %d || c: %c(%d);token: %s\n", state, c, c, token);
		switch (state) {
		case 0:
			switch(c) {
			case '.':
				if (strcmp(token, ".macro") == 0)
					state = 1;
				else if (strcmp(token, ".end_macro") == 0)
					fatalf("error: macro syntax incorrect (%c : %s)\n", c, token);
				break;
			case 'a': case '\n': case ' ':
			case '(': case ')':  case '$': 
			case ',':
				break;
			}
			break;
		case 1:
			switch(c) {
			case '.':
				if (strcmp(token, ".macro") == 0)
					fatalf("error: macro syntax incorrect (%c : %s)\n", c, token);
				else if (strcmp(token, ".end_macro") == 0)
					fatalf("error: macro syntax incorrect (%c : %s)\n", c, token);
				else
					fatalf("error: macro syntax incorrect (%c : %s)\n", c, token);
				break;
			case 'a':
				strncpy(name, token, MAXNAME);
				state = 2;
				break;
			case ' ':
				break;
			case '\n': case ')': case '$': 
			case ',': case '(': default:
				fatalf("error: macro syntax incorrect (%c : %s)\n", c, token);
				break;
			}
			break;
		case 2:
			switch(c) {
			case '.':
				if (strcmp(token, ".macro") == 0)
					fatalf("error: macro syntax incorrect (%c : %s)\n", c, token);
				else if (strcmp(token, ".end_macro") == 0)
					fatalf("error: macro syntax incorrect (%c : %s)\n", c, token);
				else
					fatalf("error: macro syntax incorrect (%c : %s)\n", c, token);
				break;
			case 'a':
				strncpy(name, token, MAXNAME);
				state = 2;
				break;
			case '\n': 
				state = 6;
				break;
			case '(':
				state = 8;
				break;
			case ' ':
				break;
			case ')': case '$': case ',':
			default:
				fatalf("error: macro syntax incorrect (%c : %s)\n", c, token);
				break;
			}
			break;
		case 3:
			switch(c) {
			case '.':
				if (strcmp(token, ".macro") == 0)
					fatalf("error: macro syntax incorrect (%c : %s)\n", c, token);
				else if (strcmp(token, ".end_macro") == 0)
					fatalf("error: macro syntax incorrect (%c : %s)\n", c, token);
				else
					fatalf("error: macro syntax incorrect (%c : %s)\n", c, token);
				break;
			case '$': 
				lastvar->nxt = malloc(sizeof(macrovar));
				lastvar = lastvar->nxt;
				*lastvar = (macrovar) {malloc(strlen(token)), NULL};
				strcpy(lastvar->name, token);
				state = 4;
				break;
			case ' ':
				break;
			case 'a': case '\n': case ')':
			case ',': case '(': default:
				fatalf("error: macro syntax incorrect (%c : %s)\n", c, token);
				break;
			}
			break;
		case 4:
			switch(c) {
			case '.':
				if (strcmp(token, ".macro") == 0)
					fatalf("error: macro syntax incorrect (%c : %s)\n", c, token);
				else if (strcmp(token, ".end_macro") == 0)
					fatalf("error: macro syntax incorrect (%c : %s)\n", c, token);
				else
					fatalf("error: macro syntax incorrect (%c : %s)\n", c, token);
				break;
			case ')':
				state = 5;
				break;
			case ',':
				state = 3;
				break;
			case ' ':
				break;
			case 'a': case '\n': case '$': 
			case '(': default:
				fatalf("error: macro syntax incorrect (%c : %s)\n", c, token);
				break;
			}
			break;
		case 5:
			switch(c) {
			case '.':
				if (strcmp(token, ".macro") == 0)
					fatalf("error: macro syntax incorrect (%c : %s)\n", c, token);
				else if (strcmp(token, ".end_macro") == 0)
					fatalf("error: macro syntax incorrect (%c : %s)\n", c, token);
				else
					fatalf("error: macro syntax incorrect (%c : %s)\n", c, token);
				break;
			case '\n': 
				state = 6;
				break;
			case ' ':
				break;
			case 'a': case ')': case '$': 
			case ',': case '(': default:
				fatalf("error: macro syntax incorrect (%c : %s)\n", c, token);
				break;
			}
			break;
		case 6:
			switch(c) {
			case '.':
				if (strcmp(token, ".macro") == 0) {
					fatalf("error: macro syntax incorrect (%c : %s)\n", c, token);
				} else if (strcmp(token, ".end_macro") == 0) {
					printf("inserting macro\n");
					insertm(name, tempm, vars);
					state = 0;
				} else {
					strcat(tempm, token);
				}
				break;
			case 'a': case '\n': case ')':
			case '$': case ',':  case '(':
			case ' ': default:
				strcat(tempm, token);
				break;
			}
			break;
		case 8:
			switch(c) {
			case '.':
				if (strcmp(token, ".macro") == 0)
					fatalf("error: macro syntax incorrect (%c : %s)\n", c, token);
				else if (strcmp(token, ".end_macro") == 0)
					fatalf("error: macro syntax incorrect (%c : %s)\n", c, token);
				else
					fatalf("error: macro syntax incorrect (%c : %s)\n", c, token);
				break;
			case '$': 
				vars = malloc(sizeof(macrovar));
				lastvar = vars;
				*lastvar = (macrovar) {malloc(strlen(token)), NULL};
				strcpy(lastvar->name, token);
				state = 4;
				break;
			case ' ':
				break;
			case 'a': case '\n': case ')':
			case ',': case '(': default:
				fatalf("error: macro syntax incorrect (%c : %s)\n", c, token);
				break;
			}
			break;
		}
	}
}

void substm(FILE *ifp, FILE *ofp) //TODO: Implement parsing args and replacing in replacemnt text
{
	int c, state, nvars, i;
	macro *tmpm;
	char tmprepl[MAXREPL], *vars[MAXVARS], tmpstr[MAXNAME];

	state = 0;
	nvars = 0;
	print_log(DEBUG, "DEBUG: Entering substm ---------------------\n", state, c, c, token);
	while ((c = gettoken(ifp)) != EOF) {
		print_log(DEBUG, "DEBUG: state: %d || c: %c(%d);token: %s\n", state, c, c, token);
		if (c == '.') {
			if (strcmp(token, ".macro") == 0 && state == 0) {
				state = 1;
			} else if (strcmp(token, ".end_macro") == 0 && state == 1) {
				print_log(DEBUG, "DEBUG: .end_macro detected\n");
				state = 0;
			} else {
				print_log(DEBUG, "DEBUG: printing token to out\n");
				fprintf(ofp, token);
			}
		} else if (c == '(') {
			if (state == 2) {
				state = 3;
				strcat(tmpstr, token);
			} else if (state > 1) {
				state = 0;
				fprintf(ofp, tmpstr);
				fprintf(ofp, token);
				strcpy(tmpstr, "");
				for (i = 0; i < nvars; i++)
					free(vars[i]);
			} else if (state == 0) {
				fprintf(ofp, token);
			}
		} else if (c == 'a') { // Handle alphanumeric token
			print_log(DEBUG, "DEBUG: alphanumeric token detected\n");
			if (state == 3) {
				state = 4;
				strcat(tmpstr, token);
				vars[nvars] = malloc(MAXNAME);
				strcpy(vars[nvars++], token);
			} else if (state > 1) {
				fprintf(ofp, tmpstr);
				fprintf(ofp, token);
				strcpy(tmpstr, "");
				for (i = 0; i < nvars; i++)
					free(vars[i]);
			} else if (state == 0) {
				if ((tmpm = findm(token)) != NULL) {
					print_log(DEBUG, "DEBUG: found macro\n");
					if  (tmpm->vars == NULL) {
						print_log(DEBUG, "DEBUG: expecting no args\n");
						fprintf(ofp, tmpm->repl);
					} else {
						state = 2;
						print_log(DEBUG, "DEBUG: expecting args\n");
						strcpy(tmpstr, token);
					}
				} else {
					fprintf(ofp, token);
				}
			}
		} else if (c == ',') {
			if (state == 4) {
				state = 3;
				strcat(tmpstr, token);
			} else if (state > 1) {
				fprintf(ofp, tmpstr);
				fprintf(ofp, token);
				strcpy(tmpstr, "");
				for (i = 0; i < nvars; i++)
					free(vars[i]);
			} else if (state == 0) {
				fprintf(ofp, token);
			}
		} else if (c == ' ') {
			if (state == 0) {
				fprintf(ofp, token);
			} else if (state != 1) {
				strcat(tmpstr, token);
			}
		} else if (c == ')') {
			if (state == 4) {
				state = 0;
				//DEBUG
				for (i = 0; i < nvars; i++) {
					printf("%s\n", vars[i]);
				}
				//END DEBUG
				replacevars(tmpstr, tmpm, vars);
				fprintf(ofp, tmpstr);
			} else if (state > 1) {
				fprintf(ofp, tmpstr);
				fprintf(ofp, token);
				strcpy(tmpstr, "");
				for (i = 0; i < nvars; i++)
					free(vars[i]);
			} else if (state == 0) {
				fprintf(ofp, token);
			}
		} else if (state == 0) {
			fprintf(ofp, token);
		}
	}
}

macro *findm(char *name)
{
	int i;

	for (i = 0; i < count; i++)
		if (strcmp (macrostore[i]->name, name) == 0)
			return macrostore[i];
	return NULL;
}

int insertm(char *name, char *repl, macrovar *vars)
{
	if (findm(name) != NULL) {
		return 0;
	} else {
		if (count < MAXCOUNT) {
			macrostore[count] = malloc(sizeof(macro));
			*macrostore[count++] = (macro) {name, repl, vars};
			return 1;
		}
		fprintf(stderr, "error: macro space full\n");
		return 0;
	}
}

int gettoken(FILE *fp)
{
	int c, i;
	
	if ((c = getc(fp)) == ' ' || c == '\t') { //handle whitespace
		token[0] = c;
		token[1] = '\0';
		return ' ';
	}

	if (c == '#') {
		while ((c = getc(fp)) != '\n' && c != EOF) //ignore comments
			;
		/*c = getc(fp);*/
	}
	if ((token[0] = c) == '.') { //assembler directive
		for (i = 1; isalnum(token[i] = getc(fp)) || token[i] == '_'; i++)
			;
		ungetc(token[i], fp);
		token[i] = '\0';
		return '.';
	}
	if ((token[0] = c) == '(' || c == ')') { //parenthesis
		token[1] = '\0';
		return c;
	}
	if (isalnum(token[0] = c)) { //word/number (no need to differentiate for a preprocessor)
		for (i = 1; isalnum(token[i] = getc(fp)) || token[i] == '_'; i++)
			;
		ungetc(token[i], fp);
		token[i] = '\0';
		return 'a';
	}
	if ((token[0] = c) == '\n') { //newline
		token[1] = '\0';
		return '\n'; 
	}
	if ((token[0] = c) == '$') { //register/variable (in a macro)
		for (i = 1; isalnum(token[i] = getc(fp)); i++)
			;
		ungetc(token[i], fp);
		token[i] = '\0';
		return '$';
	}
	if (c == EOF) return EOF;
	token[0] = c;
	token[1] = '\0';
	return c; // unknown 
}

int sgettoken(char *str, int *ind)
{
	printf("DEBUG: Entering sgettoken ----------------------\n");
	int c, i;
	printf(str + *ind);
	if ((c = str[(*ind)++]) == ' ' || c == '\t') { //whitespace
		token[0] = c;
		token[1] = '\0';
		printf("DEBUG: token = %s\n", token);
		return ' ';
	}

	if (c == '#') {
		while ((c = str[(*ind)++]) != '\n' && c != EOF) //ignore comments
			;
		c = str[(*ind)++];
	}
	if ((token[0] = c) == '.') { //assembler directive
		for (i = 1; isalnum(token[i] = str[(*ind)++]) || token[i] == '_'; i++)
			;
		(*ind)--;
		token[i] = '\0';
		return '.';
	}
	if ((token[0] = c) == '(' || c == ')') { //parenthesis
		token[1] = '\0';
		return c;
	}
	if (isalnum(token[0] = c)) { //word/number (no need to differentiate for a preprocessor)
		for (i = 1; isalnum(token[i] = str[(*ind)++]) || token[i] == '_'; i++)
			;
		(*ind)--;
		token[i] = '\0';
		return 'a';
	}
	if ((token[0] = c) == '\n') { //newline
		token[1] = '\0';
		return '\n'; 
	}
	if ((token[0] = c) == '$') { //register/variable (in a macro)
		for (i = 1; isalnum(token[i] = str[(*ind)++]); i++)
			;
		(*ind)--;
		token[i] = '\0';
		return '$';
	}
	if (c == '\0') return EOF;
	token[0] = c;
	token[1] = '\0';
	return c; // unknown 
}
