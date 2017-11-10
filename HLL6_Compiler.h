#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
/*	PROGRAM HLL5_Compiler

This program accepts a sentence in the language HLL6 and translates it into a semantically
equivalent sentence in the language ILL5.

Grammer of HLL6:
<HLL6-sentence>   -> <varDeclSection> <mainProgSection> '.'
<varDeclaration>  -> 'DECLARE' <varIdent> { ',' <varIdent> } ';'
<varIdent>        -> <letter> { <letter> | <digit> }
<mainProgSection> -> 'BEGIN' <statSequence> 'END'
<statSequence>    -> <statement> { ';' <statement> }
<statement>       -> <i-assignStat> | <writeStat> | <ifStat> | <whileStat>					
<whileStat>       -> 'WHILE' <condition> 'DO' <statSequence> 'END'							
<ifStat>		  -> 'IF' <condition> 'THEN' <statSequence> [ 'ELSE' <statSequence> ] 'END' 
<condition>		  -> <i-expression> <relOp> <i-expression>									
<relOp>			  -> '=' | '#' | '<' | '<=' | '>' | '>='									
<writeStat>       -> 'WRITE' <writeParam> | 'ENDL'
<writeParam>      ->  <varIdent> | <number> | <charString>
<i-assignStat>    -> <varIdent> ':=' <i-expression>
<i-expression>    -> <term> { ( '+' | '-') <term> }
<term>            -> <factor> { ( '*' | '/' ) <factor> }
<factor>          -> <number> | <varIdent> | '(' <i-expression> ')'
<number>          -> <digit> { <digit> }
<digit>           -> '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9'
<letter>          -> 'a' | 'b' | ... | 'z' | 'A' | 'B' | ... | 'Z'
<charString>      -> '"' <graphicChar> { graphicChar> } '"'
<charString>      -> ''' <graphicChar> { graphicChar> } '''
<graphicChar>     -> <letter> | <digit> | any-printable-ASCII-char

The semantic rules which augment the grammar rules above are:
(1) All variable identifiers in an HLL6 sentence must be declared first before they are used in the
<mainProgSection>;
(2) All declared variables must be distinct.
(3) write simply prints it's argument on the output screen
(4) endl prints the carriage-return and line-feed sequence on the output screen
(5) <charString> is a quoted character string of printable ASCII characters; it is quoted with
a pair of double quotes, or a pair of single quotes, mixed quotes are not allowed.
(6) The quotes of a string must appear on the same line, i.e., the leseme of a
<charString> may not extend over the line.


Grammer of ILL5:
<ILL5-sentence>  -> <p-instruction> { <p-instruction> } 'HLT'
<p-instruction>  -> <p-mnemonic> [ <argument> ]
<p-mnemonic>     -> 'ADD' | 'SUB' | 'MUL' | 'DVD' | 'LDI' | 'PRN' | 'LDA' | 'LDV' | 'STO' | 'INT' |
'PRC' | 'PRS' | 'NLN' | 'EQL' | 'NEQ' | 'LSS' | 'LEQ' | 'GTR' | 'GEQ' | 'JMP' | 'JMZ' | 'NUL'
<argument>       -> <number>

*/


#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>

using namespace std;

#define lineMax  80
#define MaxInt   32767
#define tableMax 100
#define wLeng    8			//length of longest varIdent or HLL6 reserved word
#define resWords 10			//Number of reserved words in HLL6

/*=============================================================*/

class compiler
{
public:
	compiler(void);  //Constructor
	~compiler() {};  //Destructor

private:
	char bs, bell;
	enum symbols{
		unknownSym, numberSym, plusSym, minusSym, timesSym, slashSym, leftParenSym,
		rightParenSym, periodSym, semicolonSym, assignSym, varIdentSym, declareSym,
		beginSym, endSym, writeSym, commaSym, endlSym, stringSym, ifSym, thenSym, elseSym,
		eqlSym, neqSym, lessSym, gtrSym, geqSym, leqSym, whileSym, doSym
	};

	enum opCodes { add, sub, mul, dvd, ldi, lda, ldv, prc, prs, nln, prn, sto, inc, eql, neq, lss, leq, gtr, geq, jmp, jmz, nul, hlt };

	ifstream sourceFile;
	ofstream codeFile;

	int number, nextCode, lineLen, charCount, lastEntry, chStringLen;
	bool hasError = false;
	char ch, strBuff[30], line[lineMax], tableIndex[tableMax], chStringText[lineMax];

	symbols sym, resSymList[resWords + 1];
	typedef char shortString[4];
	typedef char alfa[wLeng];
	alfa id;
	alfa resWordList[resWords + 1];
	shortString mnemonic[hlt + 1]; //NUMBER HAS TO BE 1 GREATER THAN THE NUMBER OF opCodes
	struct symTabRec { alfa name; int address; };
	symTabRec symTab[tableMax];
	struct pInstruction { opCodes op; int arg; };
	pInstruction pCode[MaxInt];


	void prologue(void);
	void initialize(void);
	void epilogue(void);
	void getSourceFile(void);
	void getCodeFile(void);
	void gen(opCodes op, int arg);
	void dumpCode(void);
	void CGbinaryIntOp(symbols op);
	void CGprintNumOp(void)			  { gen(prn, 0); }
	void CGdoCRLF(void)				  { gen(nln, 0); }
	void CGloadConstant(int num)	  { gen(ldi, num); }
	void CGloadAddress(int addr)	  { gen(lda, addr); }
	void CGdereference(void)		  { gen(ldv, 0); }
	void CGincrementStack(int offset) { gen(inc, offset); }
	void CGassignment(void)           { gen(sto, 0); }
	void CGHalt(void)				  { gen(hlt, 0); }
	void CGrelOp(opCodes op)		  { gen(op, 0); }
	void CGjumpOnFalse(int arg)		  { gen(jmz, arg); }
	void CGJump(int arg)			  { gen(jmp, arg); }
	void CGprintString(void);
	void backPatch(int loc, int arg);
	void error(int n);
	void GetCh(void);
	void getSym(void);
	void compile(void);
	void parseSymbol(void);
	void accept(symbols expected, int errorNum);
	void assignStat(void);
	void writeStat(void);
	void statement(void);
	void statementSequence(void);
	void mainProgSection(void);
	void varDeclaration(void);
	void printSymTab(void);
	void expression(void);
	void term(void);
	void factor(void);
	void enter(void);
	void searchIdLoc(int &idEntry);
	void condition(void);
	void ifStat(void);
	void whileStat(void);

	bool isDigit(void);
	bool isLetter(void);
}; // class compiler

/*==================================================================*/
/*==================================================================*/

//-----------//
//CONSTRUCTOR//
//-----------//
compiler::compiler(void) { prologue(); initialize(); compile(); epilogue(); }


//*******************************************************************//
//*******************************************************************//
//
//						void getSourceFile(void)
//
//*******************************************************************//
//*******************************************************************//
void compiler::getSourceFile(void)
{
	cout << "SOURCE FILE   : ";
	do
	{
		cin.getline(strBuff, 20, '\n');
		sourceFile.open(strBuff);

	} while (!sourceFile);
}

//*******************************************************************//
//*******************************************************************//
//
//						void getCodeFile(void)
//
//*******************************************************************//
//*******************************************************************//
void compiler::getCodeFile(void)
{
	do
	{
		cout << "OBJ-CODE FILE : H.OUT.txt";
		codeFile.open("H.OUT.txt");
	} while (!codeFile);
}

//*******************************************************************//
//*******************************************************************//
//
//						  void prologue(void)
//
//*******************************************************************//
//*******************************************************************//
void compiler::prologue(void)
{
	cout << endl << " ===  HLL6 Compiler  === " << endl << endl
		<< "This program accepts as input an HLL6 sentence of arthmetic expressions" << endl
		<< "given in infix notation. It translates it into an equivalent ILL5 " << endl
		<< "sentence of p-code instructions in postfix ordering." << endl << endl
		<< "To test out the language, type in one of the 3 file names:" << endl
		<< "\"TestFile1.txt\"" << endl
		<< "\"TestFile2.txt\"" << endl
		<< "\"TestFile3.txt\"" << endl << endl;

	getSourceFile();
	getCodeFile();
}

//*******************************************************************//
//*******************************************************************//
//
//							void initialize(void)
//
//*******************************************************************//
//*******************************************************************//
void compiler::initialize(void)
{
	bs = 8;		bell = 7;	ch = ' '; chStringLen = 0;

	//list of HLL6 reserved words, listed in ascending order
	strcpy_s(resWordList[1],  "BEGIN");
	strcpy_s(resWordList[2],  "DECLARE");
	strcpy_s(resWordList[3],  "DO");
	strcpy_s(resWordList[4],  "ELSE");
	strcpy_s(resWordList[5],  "END");
	strcpy_s(resWordList[6],  "ENDL");
	strcpy_s(resWordList[7],  "IF");
	strcpy_s(resWordList[8],  "THEN");
	strcpy_s(resWordList[9],  "WHILE");
	strcpy_s(resWordList[10], "WRITE");

	//grammar-symbol list of those reserved words
	resSymList[1]  = beginSym;
	resSymList[2]  = declareSym;
	resSymList[3]  = doSym;
	resSymList[4]  = elseSym;
	resSymList[5]  = endSym;
	resSymList[6]  = endlSym;
	resSymList[7]  = ifSym;
	resSymList[8]  = thenSym;
	resSymList[9]  = whileSym;
	resSymList[10] = writeSym;

	// mnemonics of ILL5 p-code
	lineLen = charCount = nextCode = 0;
	strcpy_s(mnemonic[add], "ADD");
	strcpy_s(mnemonic[sub], "SUB");
	strcpy_s(mnemonic[mul], "MUL");
	strcpy_s(mnemonic[dvd], "DVD");
	strcpy_s(mnemonic[ldi], "LDI");
	strcpy_s(mnemonic[lda], "LDA");
	strcpy_s(mnemonic[ldv], "LDV");
	strcpy_s(mnemonic[prc], "PRC");
	strcpy_s(mnemonic[prs], "PRS");
	strcpy_s(mnemonic[nln], "NLN");
	strcpy_s(mnemonic[prn], "PRN");
	strcpy_s(mnemonic[hlt], "HLT");
	strcpy_s(mnemonic[sto], "STO");
	strcpy_s(mnemonic[inc], "INT");
	strcpy_s(mnemonic[eql], "EQL");
	strcpy_s(mnemonic[neq], "NEQ");
	strcpy_s(mnemonic[lss], "LSS");
	strcpy_s(mnemonic[leq], "LEQ");
	strcpy_s(mnemonic[gtr], "GTR");
	strcpy_s(mnemonic[geq], "GEQ");
	strcpy_s(mnemonic[jmp], "JMP");
	strcpy_s(mnemonic[jmz], "JMZ");
	strcpy_s(mnemonic[nul], "NUL");
}

//*******************************************************************//
//*******************************************************************//
//
//							void epilogue(void)
//
//*******************************************************************//
//*******************************************************************//
void compiler::epilogue(void)
{
	cout << endl << " === End of Compilation ===" << endl;
	sourceFile.close(); codeFile.close(); // both not necessary
}

//*******************************************************************//
//*******************************************************************//
//
//							void error(int n)
//
//*******************************************************************//
//*******************************************************************//
void compiler::error(int n)
{
	if (!hasError)
	{
		cout << endl << bell << bell << "Error " << n << ": ";
		switch (n)
		{
		case 1: cout << "Number is too large."; break;
		case 2: cout << "A \')\' is expected."; break;
		case 3: cout << "Source incomplete, unexpected EOF."; break;
		case 4: cout << "Unknown symbol found."; break;
		case 5: cout << "A period is expected."; break;
		case 6: cout << "A number, variable or \'(\' is expected."; break;
		case 7: cout << "A variable identifier is expected."; break;
		case 8: cout << "Assignment operator ':=' is expected."; break;
		case 9: cout << "DECLARE expected."; break;
		case 10: cout << "BEGIN expected."; break;
		case 11: cout << "Whoaaaa!! Symbol table is full."; break;
		case 12: cout << "A semicolon is expected."; break;
		case 13: cout << "An identifier, WRITE, or ENDL is expected."; break;
		case 14: cout << "The END is expected."; break;
		case 15: cout << "Identifier not declared."; break;
		case 16: cout << "Mamma mia, no re-declaration please."; break;
		case 17: cout << "Character string is incomplete."; break;
		case 18: cout << "Relational operator expected."; break;
		case 19: cout << "'THEN' symbol expected."; break;
		case 20: cout << "'DO' symbol exprected."; break;
		}
		hasError = true;
		cout << endl << endl << "Program exectuion haulted." << endl;
		epilogue();
	}
} // error

/* --------------------------------  Lexical Analyzer  --------------------------------------------- */


//*******************************************************************//
//*******************************************************************//
//
//							void getSym(void)
//
//*******************************************************************//
//*******************************************************************//
void compiler::getSym(void)
{
	// recognize and form next sym from sourceFile
	while (ch == ' ') GetCh(); // skip leading blanks
	sym = unknownSym; // initial assumption
	parseSymbol();
}

//*******************************************************************//
//*******************************************************************//
//
//						void CGprintString(void)
//
//*******************************************************************//
//*******************************************************************//
void compiler::CGprintString(void)
{
	for (int i = 0; i < chStringLen; i++)
		gen(ldi, chStringText[i]);
	gen(ldi, chStringLen);
	gen(prs, 0);
}

//*******************************************************************//
//*******************************************************************//
//
//							void GetCh(void)
//
//*******************************************************************//
//*******************************************************************//
void compiler::GetCh(void)
{
	// get next character from sourceFile
	if (charCount == lineLen)
	{
		if (sourceFile.eof()) error(3);
		lineLen = 0;
		charCount = 0;
		//creating the compile listing
		sourceFile.getline(line, lineMax, '\n');
		cout << setw(6) << nextCode << " " << line << endl;
		while (line[lineLen++] != '\0') { /* do nothing */ }
		line[lineLen - 1] = ' ';
	}
	ch = line[charCount++];
}

//*******************************************************************//
//*******************************************************************//
//
//						parseSymbol HELPLER functions
//
//*******************************************************************//
//*******************************************************************//

bool compiler::isDigit(void)
{
	if ((ch >= '0') && (ch <= '9'))
		return true;
	else
		return false;
}

bool compiler::isLetter(void)
{
	if ((ch >= 'A') && (ch <= 'Z') || (ch >= 'a' && ch <= 'z'))
		return true;
	else
		return false;

}

//*******************************************************************//
//*******************************************************************//

//*******************************************************************//
//*******************************************************************//
//
//							void parseSymbol(void)
//
//*******************************************************************//
//*******************************************************************//
void compiler::parseSymbol(void)
{
	int i, j, k;

	switch (ch)
	{
	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p':
	case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
	case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N': case 'O': case 'P':
	case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
		k = 0;
		for (int count = 0; count < wLeng; count++)
			id[count] = '\0';

		do  //Check: check for correct logic
		{
			if ((ch >= 'a') && (ch <= 'z'))
				ch -= 32;
			if (k <= wLeng)
			{
				id[k] = ch;
				k++;
			}
			GetCh();
		} while (isLetter() || isDigit());
		i = 1; //Index counter of the reserved words list (index of the first word)
		j = resWords;
		do
		{
			k = (i + j) / 2;
			if (strcmp(id, resWordList[k]) < 0 || strcmp(id, resWordList[k]) == 0)
				j = k - 1;
			if (strcmp(id, resWordList[k]) > 0 || strcmp(id, resWordList[k]) == 0)
				i = k + 1;
		} while (i <= j);
		if ((i - 1) > j)
			sym = resSymList[k];
		else
			sym = varIdentSym;
		break;

	case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7':
	case '8': case '9': //If it is a number, go through the "do while" loop below
		number = 0;
		sym = numberSym;
		do
		{
			if (number <= (MaxInt - (ch - '0')) / 10) // overflow?
				number = 10 * number + (ch - '0');
			else
				error(1);

			GetCh();
		} while (ch <= '9' && ch >= '0');
		break;

	case '*': sym = timesSym;			GetCh();  break;
	case '/': sym = slashSym;			GetCh();  break;
	case '+': sym = plusSym;			GetCh();  break;
	case '-': sym = minusSym;			GetCh();  break;
	case '(': sym = leftParenSym;		GetCh();  break;
	case ')': sym = rightParenSym;		GetCh();  break;
	case ';': sym = semicolonSym;		GetCh();  break;
	case '.': sym = periodSym;			GetCh();  break;
	case ',': sym = commaSym;			GetCh();  break;
	case '"': case '\'':
	{
		char startChar = ch; chStringLen = 0;
		GetCh();
		while (ch != startChar)
		{
			chStringText[chStringLen] = ch;
			chStringLen++;
			GetCh();
			if (chStringLen > lineMax)
				error(17);
		}
		GetCh();
		sym = stringSym;
		break;
	}
	case ':': GetCh();
		if (ch == '=') sym = assignSym;
		else error(8);
		GetCh(); break;
	case '<': GetCh();
		if (ch == '=') { sym = leqSym; GetCh(); }
		else             sym = lessSym;
		break;
	case '=': sym = eqlSym; GetCh();
		break;
	case '#': sym = neqSym; GetCh();
		break;
	case '>': GetCh();
		if (ch == '=') { sym = geqSym; GetCh(); }
		else		     sym = gtrSym;
		break;
	default:
	{
		GetCh();	error(4);
	}
	}
} // parseSymbol

/* --------------------------------  Syntax Analyzer  --------------------------------------------- */

//*******************************************************************//
//*******************************************************************//
//
//							void compile(void)
//
//*******************************************************************//
//*******************************************************************//
void compiler::compile(void)
{
	// <HLL6-sentence> -> <varDeclaration> <vainProgSection> '.'
	cout << endl << "  Compile Listing:  " << endl;
	lastEntry = 0;
	varDeclaration();
	CGincrementStack(lastEntry);
	mainProgSection();

	if (sym != periodSym)
		error(5);
	else
	{ 
		CGHalt();
		printSymTab();
		dumpCode();
	}
}

//*******************************************************************//
//*******************************************************************//
//
//							void enter(void)
//
//*******************************************************************//
//*******************************************************************//
void compiler::enter(void)
{
	int i;

	if (lastEntry == tableMax)
		error(11);
	for (i = 1; i <= lastEntry; i++)
		if (strcmp(symTab[i].name, id) == 0)
			error(16);

	lastEntry++;
	strcpy(symTab[lastEntry].name, id);
	symTab[lastEntry].address = lastEntry;
}

//*******************************************************************//
//*******************************************************************//
//
//						void searchIdLoc(int &idEntry)
//
//*******************************************************************//
//*******************************************************************//
void compiler::searchIdLoc(int &idEntry)
{
	strcpy(symTab[0].name, id);
	idEntry = lastEntry + 1;
	do
		idEntry--;
	while (strcmp(symTab[idEntry].name, id) != 0);

	if (idEntry == 0) error(15);
}


//*******************************************************************//
//*******************************************************************//
//
//							void accept(void)
//
//*******************************************************************//
//*******************************************************************//
void compiler::accept(symbols expected, int errorNum)
{
	if (sym == expected)
		getSym();
	else
		error(errorNum);
}

//*******************************************************************//
//*******************************************************************//
//
//							void assignStat(void)
//
//*******************************************************************//
//*******************************************************************//
void compiler::assignStat(void)
{
	// <i-assignStat> -> <varIdent> '=' <i-expression>
	int varIdLoc;

	searchIdLoc(varIdLoc);
	if (varIdLoc > tableMax) error(11);
	CGloadAddress(varIdLoc);
	getSym();
	accept(assignSym, 8);
	expression();
	CGassignment();
}

//*******************************************************************//
//*******************************************************************//
//
//							void writeStat(void)
//
//*******************************************************************//
//*******************************************************************//
void compiler::writeStat(void)
{
	// <writeStat>  -> 'WRITE' <writeParam> | 'ENDL'
	// <writeParam> ->  <varIdent> | <number> | <charString>
	int loc = 0;

	getSym();
	switch (sym)
	{
	case numberSym:
		CGloadConstant(number);
		CGprintNumOp(); break;
	case varIdentSym:
		searchIdLoc(loc);
		if (loc > tableMax) error(11);
		CGloadAddress(loc);
		CGdereference();
		CGprintNumOp();
		break;
	case stringSym:
		CGprintString();
		break;
	case endlSym:
		CGdoCRLF();
		break;
	}
	getSym();
}

//*******************************************************************//
//*******************************************************************//
//
//							void condition(void)
//
//*******************************************************************//
//*******************************************************************//
void compiler::condition(void)
{
	//<condition> -> <i-expression> <relOp> <i-expression>
	expression();
	switch (sym)
	{
	case eqlSym:  getSym(); expression(); CGrelOp(eql); break;
	case neqSym:  getSym(); expression(); CGrelOp(neq); break;
	case lessSym: getSym(); expression(); CGrelOp(lss); break;
	case leqSym:  getSym(); expression(); CGrelOp(leq); break;
	case gtrSym:  getSym(); expression(); CGrelOp(gtr); break;
	case geqSym:  getSym(); expression(); CGrelOp(geq); break;
	default:
		error(18);
	}
}

//*******************************************************************//
//*******************************************************************//
//
//							void ifStat(void)
//
//*******************************************************************//
//*******************************************************************//
void compiler::ifStat(void)
{
	// <ifStat> -> 'IF' <condition> 'THEN' <statSequence> ['ELSE' <statSequence>] 'END'
	int jmzLabel, jmpLabel;
	getSym();
	condition();
	jmzLabel = nextCode;
	CGjumpOnFalse(-1);
	if (sym != thenSym) error(19);
	statementSequence();
	if (sym == elseSym)
	{
		jmpLabel = nextCode;
		CGJump(-1);
		backPatch(jmzLabel, nextCode);
		statementSequence();
		backPatch(jmpLabel, nextCode);
	}
	else
		backPatch(jmzLabel, nextCode);
	accept(endSym, 14);
}
//*******************************************************************//
//*******************************************************************//
//
//							void whileStat(void)
//
//*******************************************************************//
//*******************************************************************//
void compiler::whileStat(void)
{
	//<whileStat> -> 'WHILE' <condition> 'DO' <statSequence> 'END'
	int startLabel, endLabel;
	startLabel = nextCode;
	getSym();
	condition();
	endLabel = nextCode;
	CGjumpOnFalse(-1);
	if (sym != doSym) error(20);
	statementSequence();
	CGJump(startLabel);
	backPatch(endLabel, nextCode);
	accept(endSym, 14);
}

//*******************************************************************//
//*******************************************************************//
//
//							void backPatch(void)
//
//*******************************************************************//
//*******************************************************************//
void compiler::backPatch(int loc, int arg)
{
	pCode[loc].arg = arg;
}

//*******************************************************************//
//*******************************************************************//
//
//							void statement(void)
//
//*******************************************************************//
//*******************************************************************//
void compiler::statement(void)
{
	// <statement> -> <i-assignStat> | <writeStat> | <ifStat> | <whileStat>
	switch (sym)
	{
	case varIdentSym: assignStat(); break;
	case writeSym:    writeStat(); break;
	case endlSym:     CGdoCRLF(); getSym();  break;
	case ifSym:		  ifStat();  break;
	case whileSym:    whileStat();
	}
}

//*******************************************************************//
//*******************************************************************//
//
//						void statementSequence(void)
//
//*******************************************************************//
//*******************************************************************//
void compiler::statementSequence(void)
{
	// <statSequence> -> <statement> { ';' <statement> }
	do
	{
		getSym();
		if ((sym == varIdentSym) || (sym == writeSym) || (sym == endlSym) || (sym == ifSym) || (sym == whileSym))
			statement();
		else
			error(13);
	} while (sym == semicolonSym);
}

//*******************************************************************//
//*******************************************************************//
//
//						void mainProgSection(void)
//
//*******************************************************************//
//*******************************************************************//
void compiler::mainProgSection(void)
{
	// <mainProgSection> -> 'BEGIN' <statSequence> 'END'
	if (sym != beginSym)
		error(10);
	statementSequence();
	accept(endSym, 14);
}

//*******************************************************************//
//*******************************************************************//
//
//						void varDeclaration(void)
//
//*******************************************************************//
//*******************************************************************//
void compiler::varDeclaration(void)
{
	// <varDeclaration> -> 'DECLARE' <varIdent> { ',' <varIdent> } ';'
	getSym();
	if (sym != declareSym)
		error(9);
	do
	{
		getSym();
		if (sym != varIdentSym)
			error(7);
		else
		{
			enter();
			getSym();
		}
	} while (sym == commaSym);
	accept(semicolonSym, 12);
}

//*******************************************************************//
//*******************************************************************//
//
//						void printSymTab(void)
//
//*******************************************************************//
//*******************************************************************//
void compiler::printSymTab(void)
{
	int i;
	cout << endl;
	cout << "Symbol Table:" << endl;
	cout << " No.   VarName     VarAddress" << endl;
	for (i = 1; i <= lastEntry; i++)
	{
		int w = wLeng - strlen(symTab[i].name);
		cout << "  " << i << "     " << symTab[i].name;
		for (int count = 0; count < w; count++)
			cout << " ";
		cout << "       " << symTab[i].address << endl;
	}
}

//*******************************************************************//
//*******************************************************************//
//
//							void expression(void)
//
//*******************************************************************//
//*******************************************************************//
void compiler::expression(void)
{
	// <i-expression> -> <term> { ('+' | '-') <term> }
	symbols addOp;
	term();
	while (sym == plusSym || sym == minusSym)
	{
		addOp = sym;
		getSym();
		term();
		CGbinaryIntOp(addOp);
	}
}

//*******************************************************************//
//*******************************************************************//
//
//							void term(void)
//
//*******************************************************************//
//*******************************************************************//
void compiler::term(void)
{
	// <term> -> <factor> { ('*' | '/') <factor> }
	symbols mulOp;
	factor();
	while (sym == timesSym || sym == slashSym)
	{
		mulOp = sym;
		getSym();
		factor();
		CGbinaryIntOp(mulOp);
	}
}

//*******************************************************************//
//*******************************************************************//
//
//							void factor(void)
//
//*******************************************************************//
//*******************************************************************//
void compiler::factor(void)
{
	// <factor> -> <number> | <varIdent> | '(' <i-expression> ')'
	int varIdLoc = 0;
	switch (sym)
	{
	case varIdentSym:
		searchIdLoc(varIdLoc);
		if (varIdLoc > tableMax) error(11);
		CGloadAddress(varIdLoc);
		CGdereference();
		getSym(); break;
	case leftParenSym:
		getSym();	expression();	accept(rightParenSym, 2);	break;
	case numberSym:
		CGloadConstant(number);	getSym();	break;
	default: error(6);
	}
}

//*******************************************************************//
//*******************************************************************//
//
//						void gen(opCodes op, int arg)
//
//*******************************************************************//
//*******************************************************************//
void compiler::gen(opCodes op, int arg)
{
	pCode[nextCode].op = op;
	pCode[nextCode].arg = arg;
	nextCode++;
}

//*******************************************************************//
//*******************************************************************//
//
//						void dumpCode(void)
//
//*******************************************************************//
//*******************************************************************//
void compiler::dumpCode(void)
{
	for (int i = 0; i < nextCode; i++)
	{
		codeFile << setw(10) << i << setw(5) << mnemonic[pCode[i].op];
		if (pCode[i].op == ldi || pCode[i].op == inc || pCode[i].op == lda || pCode[i].op == jmp || pCode[i].op == jmz)
			codeFile << setw(5) << pCode[i].arg << endl;
		else
			codeFile << endl;
	}
}

//*******************************************************************//
//*******************************************************************//
//
//					void CGBinaryIntOp(symbols op)
//
//*******************************************************************//
//*******************************************************************//
void compiler::CGbinaryIntOp(symbols op)
{
	switch (op)
	{
	case plusSym:   gen(add, 0); break;
	case timesSym:  gen(mul, 0); break;
	case minusSym:  gen(sub, 0); break;
	case slashSym:  gen(dvd, 0); break;
	}
}

/*=============================================================*/



