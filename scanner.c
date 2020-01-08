//Mini c의 스캐너 구현.
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define ID_LENGTH 12
#define NO_KEYWORDS 7

enum tsymbol { //특수문자들의 심볼넘버
	tnull = -1,
	tnot, tnotequ, tmod, tmodAssign, tident, tnumber,		//0~5
	tand, tlparen, trparen, tmul, tmulAssign, tplus,		//5~11
	tinc, taddAssign, tcomma, tminus, tdec, tsubAssign,		//12~17
	tdiv, tdivAssign, tsemicolon, tless, tlesse, tassign,	//18~23
	tequal, tgreat, tgreate, tlbracket, trbracket, teof,	//24~29
			// word symbols //
	tconst, telse, tif, tint, treturn, tvoid,				//30~35
	twhile, tlbrace, tor, trbrace							//36~39
};

char *keyword[NO_KEYWORDS] = { "const","else","if","int","return","void","while" };
enum tsymbol tnum[NO_KEYWORDS] = { tconst,telse,tif,tint,treturn,tvoid,twhile };

int hexValue(char ch)
{
	switch (ch) {
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		return (ch - '0');
	case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
		return (ch - 'A' + 10);
	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
		return (ch - 'a' + 10);
	default: return -1;
	}
}
int getIntNum(char firstCharacter, FILE* fp) { //숫자를 받기위한 함수
	int num = 0;
	int value;
	char ch;

	if (firstCharacter != '0') {//decimal
		ch = firstCharacter;
		do {
			num = 10 * num + (int)(ch - '0');
			ch = fgetc(fp);
		} while (isdigit(ch));
	}
	else {
		ch = fgetc(fp);
		if ((ch >= '0') && (ch <= '7'))  //octal
			do {
				num = 8 * num + (int)(ch - '0');
				ch = fgetc(fp);
			} while ((ch >= '0') && (ch <= '7'));
		else if ((ch == 'X') || (ch == 'x')) { // hexa decimal
			while ((value = hexValue(ch = fgetc(fp))) != -1)
				num = 16 * num + value;
		}
		else num = 0;	// zero
	}
	ungetc(ch, fp); // retract 
	return num;
}

void lexicalError(int n) {
	printf(" *** Lexical Error : ");
	switch (n) {
	case 1: printf("an identifier length must be less than 12.\n"); break;
	case 2: printf("next character must be &.\n"); break;
	case 3: printf("next character must be |.\n"); break;
	case 4: printf("invaild character!!!\n"); break;
	}
}

int superLetter(char ch) {
	if (isalpha(ch) || ch == '_') return 1;
	else return 0;
}
int superLetterOrDigit(char ch) {
	if (isalnum(ch) || ch == '_') return 1;
	else return 0;
}

struct tokenType {
	int number; //token number

	union {
		char id[ID_LENGTH];
		int num;
	}value;
};

struct tokenType scanner(FILE* fp) {
	struct tokenType token;
	int i, index;
	char ch, id[ID_LENGTH];
	token.number = tnull;

	do {
		while (isspace(ch = fgetc(fp))); // state 1: 공백제거
		if (superLetter(ch)) { // identifier or keyword
			i = 0;
			do {
				if (i < ID_LENGTH) id[i++] = ch;
				ch = fgetc(fp);
			} while (superLetterOrDigit(ch));

			if (i >= ID_LENGTH) lexicalError(1);
			id[i] = '\0';
			ungetc(ch, fp); //retract

			//find the identifier in the keyword table

			for (index = 0; index < NO_KEYWORDS; index++) {
				strcpy(token.value.id, id);
				if (!strcmp(id, keyword[index])) break;
			}
			if (index < NO_KEYWORDS)		 // found, keyword exit
				token.number = tnum[index];
			else 			//not found, identifier exit
				token.number = tident;
		}// end of identifier or key word
		else if (isdigit(ch)) {		//integer constant
			token.number = tnumber;
			token.value.num = getIntNum(ch, fp);
		}
		else
		{
			for (i = 0; i < ID_LENGTH; i++) {
				token.value.id[i] = '\0';
			}i = 0;
			token.value.id[i++] = ch;

			switch (ch) {//special character

			case '/': //state 10
				ch = fgetc(fp);
				token.value.id[i++] = ch;
				if (ch == '*') //text comment
					do {
						while (ch != '*') ch = fgetc(fp);
						ch = fgetc(fp);
					} while (ch != '/');// line comment 

				else if (ch == '/')
					while (fgetc(fp) != '\n');
				else if (ch == '=') token.number = tdivAssign;
				else {
					token.number = tdiv;
					ungetc(ch, fp); // retract
				}
				break;
			case '!': // state 17
				ch = fgetc(fp);
				token.value.id[i++] = ch;
				if (ch == '=') token.number = tnotequ;
				else {
					token.number = tnot;
					ungetc(ch, fp); // retract
				}
				break;

			case '%': // state 20
				ch = fgetc(fp);
				token.value.id[i++] = ch;
				if (ch == '=')
					token.number = tmodAssign;
				else {
					token.number = tmod; ungetc(ch, fp);
				}
				break;

			case '&': // state 23
				ch = fgetc(fp);
				token.value.id[i++] = ch;
				if (ch == '&')
					token.number = tand;
				else {
					lexicalError(2);
					ungetc(ch, fp); // retract
				}
				break;

			case '*': // state 25
				ch = fgetc(fp);
				token.value.id[i++] = ch;
				if (ch == '=')
					token.number = tmulAssign;
				else {
					token.number = tmul;
					ungetc(ch, fp); // retract
				}
				break;

			case '+': // state 28
				ch = fgetc(fp);
				token.value.id[i++] = ch;
				if (ch == '+')
					token.number = tinc;
				else if (ch == '=')
					token.number = taddAssign;
				else {
					token.number = tplus;
					ungetc(ch, fp); // retract
				}
				break;

			case '-': // state 32
				ch = fgetc(fp);
				token.value.id[i++] = ch;
				if (ch == '-')
					token.number = tdec;
				else if (ch == '=')
					token.number = tsubAssign;
				else {
					token.number = tminus;
					ungetc(ch, fp); // retract
				}
				break;

			case '<': // state 36
				ch = fgetc(fp);
				token.value.id[i++] = ch;
				if (ch == '=') token.number = tlesse;
				else {
					token.number = tless;
					ungetc(ch, fp); // retract
				}
				break;

			case '=': // state 39
				ch = fgetc(fp);
				token.value.id[i++] = ch;
				if (ch == '=')  token.number = tequal;
				else {
					token.number = tassign;
					ungetc(ch, fp); // retract
				}
				break;

			case '>': // state 42
				ch = fgetc(fp);
				token.value.id[i++] = ch;
				if (ch == '=') token.number = tgreate;
				else {
					token.number = tgreat;
					ungetc(ch, fp); // retract
				}
				break;

			case '|': // state 45
				ch = fgetc(fp);
				token.value.id[i++] = ch;
				if (ch == '|')  token.number = tor;
				else {
					lexicalError(3);
					ungetc(ch, fp); // retract
				}
				break;
			case '(': token.number = tlparen;		break;
			case ')': token.number = trparen;		break;
			case ',': token.number = tcomma;		break;
			case ';': token.number = tsemicolon;	break;
			case '[': token.number = tlbracket;		break;
			case ']': token.number = trbracket;		break;
			case '{': token.number = tlbrace;		break;
			case '}': token.number = trbrace;		break;
			case EOF: token.number = teof;			break;

			default: {
				printf("Current character : %c", ch);
				lexicalError(4);
				break;
			}
			}// switch end 
		}
	} while (token.number == tnull);
	return token;
} // end of scanner

void Printtoken(struct tokenType token) {
	FILE *wfp;
	wfp = fopen("Scan_prime.txt", "a");
	fprintf(wfp,"Token ->	");
	if (token.number == 5)
		fprintf(wfp, "%d : (%d, %d)\n", token.value.num, token.number, token.value.num);
	else if (token.number == 4)
		fprintf(wfp, "%s : (%d, %s)\n", token.value.id, token.number, token.value.id);
	else
		fprintf(wfp, "%s : (%d, 0)\n", token.value.id, token.number);
	fclose(wfp);
}

int main()
{
	struct tokenType token;
	FILE *rfp;
	rfp = fopen("prime.mc", "r");
	while (!feof(rfp)) {
		token = scanner(rfp);
		Printtoken(token);
	}
	fclose(rfp);

	return 0;
}
