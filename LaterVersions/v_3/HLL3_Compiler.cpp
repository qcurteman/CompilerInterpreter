#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
/*	PROGRAM HLL3_Compiler

This program accepts a sentence in the language HLL3 and translates it into a semantically
equivalent sentence in the language IL3.

Grammer of HLL3:
<HLL3-sentence>   -> <varDeclSection> <mainProgSection> '.'
<varDeclSection>  -> 'DECLARE' <varIdent> { ',' <varIdent> } ';'
<varIdent>        -> <letter> { <letter> | <digit> }
<mainProgSection> -> 'BEGIN' <statSequence> 'END'
<statSequence>    -> <statement> { ';' <statement> }
<writeStat>       -> 'WRITELN' ( <varIdent> | <number> )
<i-assignStat>    -> <varIdent> ':=' <i-expression>
<i-expression>    -> <term> { ( '+' | '-') <term> }
<term>            -> <factor> { ( '*' | '/' ) <factor> }
<factor>          -> <number> | <varIdent> | '(' <i-expression> ')'
<number>          -> <digit> { <digit> }
<digit>           -> '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9'
<letter>          -> 'a' | 'b' | ... | 'z' | 'A' | 'B' | ... | 'Z'

The semantic rules which augment the grammar rules above are:
(1) All variable identifiers in an HLL3 sentence must be declared first before they are used in the 
	<mainProgSection>;
(2) All declared variables must be distinct.


Grammer of IL2:
<IL2-sentence>  -> <p-instruction> { <p-instruction> } 'HLT'
<p-instruction> -> <p-mnemonic> [ <argument> ]
<p-mnemonic>    -> 'ADD' | 'SUB' | 'MUL' | 'DVD' | 'LDI' | 'PRN' | 'LDA' | 'LDV' | 'STO' | 'INT' |
				       'PRC' | 'PRS' | 'NLN' 
<argument>      -> <number>

*/


#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>

using namespace std;

#define lineMax  80
#define MaxInt   32767
#define tableMax 100
#define wLeng    8			//length of longest varIdent or HLL3 reserved word
#define resWords 4		//Number of reserved words in HLL3

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
		beginSym, endSym, writeSym, commaSym
	};

	enum opCodes { add, sub, mul, dvd, ldi, lda, ldv, prc, prs, nln, prn, sto, inc, hlt };
	
	ifstream sourceFile;
	ofstream codeFile;
	
	
	int number, nextCode, lineLen, charCount, lastEntry;		
	bool hasError = false;
	char ch, strBuff[30], line[lineMax], tableIndex[tableMax];

	symbols sym, resSymList[resWords + 1];
	typedef char shortString[4];
	typedef char alfa[wLeng];
	alfa id;
	alfa resWordList[resWords + 1];
	shortString mnemonic[hlt + 1]; //NUMBER HAS TO BE 1 GREATER THAN THE NUMBER OF opCodes
	struct symTabRec { alfa name; int address; };
	symTabRec symTab[tableMax];


	void prologue(void);
	void epilogue(void);
	void getSourceFile(void);
	void getCodeFile(void);
	void gen(opCodes op, int arg);
	void CGbinaryIntOp(symbols op);
	void CGprintNumOp(void)			  { gen(prn, 0); }
	void CGdoCRLF(void)				  { gen(nln, 0); }
	void CGloadConstant(int num)	  { gen(ldi, num); }
	void CGloadAddress(int addr)	  { gen(lda, addr); }
	void CGdereference(void)		  { gen(ldv, 0); }
	void CGincrementStack(int offset) { gen(inc, offset); }
	void CGassignment(void)           { gen(sto, 0); }
	void CGHalt(void)				  { gen(hlt, 0); }
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

	bool isDigit(void);
	bool isLetter(void);
}; // class compiler

/*==================================================================*/
/*==================================================================*/

//-----------//
//CONSTRUCTOR//
//-----------//
compiler::compiler(void) { prologue(); compile(); epilogue(); }


//*******************************************************************//
//*******************************************************************//
//
//						void getSourceFile(void)
//
//*******************************************************************//
//*******************************************************************//
void compiler::getSourceFile(void)
{
	do
	{
		cout << "SOURCE FILE   : ";
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
		cout << "OBJ-CODE FILE : ";
		cin.getline(strBuff, 20, '\n');
		codeFile.open(strBuff);
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
	cout << endl << " ===  HLL3 Compiler  === " << endl << endl
		<< "This program accepts as input an HLL3 sentence of arthmetic expressions" << endl
		<< "given in infix notation. It translates it into an equivalent IL3 " << endl
		<< "sentence of p-code instructions in postfix ordering." << endl << endl;

	getSourceFile();
	getCodeFile();

	bs = 8;		bell = 7;	ch = ' '; 

	//list of HLL3 reserved words, listed in ascending order
	strcpy_s(resWordList[1], "BEGIN");
	strcpy_s(resWordList[2], "DECLARE");
	strcpy_s(resWordList[3], "END");
	strcpy_s(resWordList[4], "WRITELN");

	//grammar-symbol list of those reserved words
	resSymList[1] = beginSym;
	resSymList[2] = declareSym;
	resSymList[3] = endSym;
	resSymList[4] = writeSym;

	// mnemonics of IL3 p-code
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
		case 13: cout << "An identifier or WRITELN is expected."; break;
		case 14: cout << "The END is expected."; break;
		case 15: cout << "Identifier not declared."; break;
		case 16: cout << "Mamma mia, no re-declaration please."; break;
		}
		hasError = true;
		cout << endl << endl << "Program exectuion haulted." << endl;
		epilogue();
	}
	// exit(1); // panic-driven compiler: hault as soon as 1st error occurs
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
		line[lineLen-1] = ' ';
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
	case ':': GetCh();
		if (ch == '=') sym = assignSym;
		else error(8);
		GetCh(); break;
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
	// <HLL3-sentence> -> <varDeclaration> <vainProgSection> '.'
	cout << endl << "  Compile Listing:  " << endl;
	lastEntry = 0;
	varDeclaration();
	CGincrementStack(lastEntry);
	mainProgSection();
	//do
	//{
	//	getSym(); assignStat(); //expression();
	//	if (sym == semicolonSym)
	//		CGprintNumOp();
	//} while (sym == semicolonSym);

	if (sym != periodSym)
		error(5);
	else
	{
		//CGprintNumOp();  
		CGHalt();
		printSymTab();
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

	//for (int count = 0; count < strlen(id); count++)
	//	symTab[0].name[count] = id[count];

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
	// <writeStat> -> 'WRITELN' ( <varIdent> | <number> )
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
	}
	CGdoCRLF();
	getSym();
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
	// <statement> -> <i-assignStat> | <writeStat>
	switch (sym)
	{
	case varIdentSym: assignStat(); break;
	case writeSym:    writeStat(); break;
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
		if ((sym == varIdentSym) || (sym == writeSym))
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
	// <varDeclaration> -> 'BEGIN' <statSequence> 'END'
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
	} while(sym == commaSym);
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
	//Change: Might have a problem with "i" and it's location/where it "says" it is
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
	codeFile << setw(10) << nextCode << setw(5) << mnemonic[op];
	if (op == ldi || op == inc || op == lda)
		codeFile << setw(5) << arg << endl;
	else
		codeFile << endl;
	nextCode++;
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

void main() { compiler myCompiler; cin.get(); } // construct a compiler!