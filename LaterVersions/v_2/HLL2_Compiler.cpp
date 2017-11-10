#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
/*	PROGRAM HLL2_Compiler

This program accepts a sentence in the language HLL2 and translates it into a semantically
equivalent sentence in the language IL2.

Grammer of HLL2:
<HLL2-sentence> -> <i-expression> { ';' <i-expression> } '.'
<i-expression>  -> <term> { ( '+' | '-') <term> }
<term>          -> <factor> { ( '*' | '/' ) <factor> }
<factor>        -> <number> | '(' <i-expression> ')'
<number>        -> <digit> { <digit> }
<digit>         -> '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9'

Grammer of IL2:
<IL2-sentence>  -> <p-instruction> { <p-instruction> } 'HLT'
<p-instruction> -> <p-mnemonic> [ <argument> ]
<p-mnemonic>    -> 'ADD' | 'STUB' | 'MUL' | 'DVD' | 'LDI' | 'PRN'
<argument>      -> <number>

*/


#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>

using namespace std;

#define lineMax 80
#define MaxInt  32767

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
		rightParenSym, periodSym, semicolonSym, assignSym, varIdentSym
	};

	enum opCodes { add, sub, mul, dvd, ldi, prn, hlt, sto, inc };
	ifstream sourceFile;
	ofstream codeFile;
	char ch, strBuff[30], line[lineMax];
	int number, nextCode, lineLen, charCount;
	bool hasError = false;
	symbols sym;
	typedef char shortString[4];
	shortString mnemonic[inc + 1]; //NUMBER HAS TO BE GREATER THAN THE NUMBER OF opCodes

	void prologue(void);
	void epilogue(void);
	void getSourceFile(void);
	void getCodeFile(void);
	void gen(opCodes op, int arg);
	void CGbinaryIntOp(symbols op);
	void CGprintNumOp(void)			  { gen(prn, 0); }
	void CGloadConstant(int num)	  { gen(ldi, num); }
	void CGincrementStack(int offset) { gen(inc, offset); }
	void CGassignment(void)           { gen(sto, 0); }
	void CGHalt(void)				  { gen(hlt, 0); }
	void error(int n);
	void GetCh(void);
	void getSym(void);
	void compile(void);
	void parseSymbol(void);
	void accept(symbols expected, int errorNum);
	void AssignStat(void);
	void expression(void);
	void term(void);
	void factor(void);
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
	cout << endl << " ===  HLL2 Compiler  === " << endl << endl
		<< "This program accepts as input an HLL2 sentence of arthmetic expressions" << endl
		<< "given in infix notation. It translates it into an equivalent IL2 " << endl
		<< "sentence of p-code instructions in postfix ordering." << endl << endl;

	getSourceFile();
	getCodeFile();

	bs = 8;		bell = 7;	ch = ' ';

	lineLen = charCount = nextCode = 0;
	strcpy_s(mnemonic[add], "ADD");
	strcpy_s(mnemonic[sub], "SUB");
	strcpy_s(mnemonic[mul], "MUL");
	strcpy_s(mnemonic[dvd], "DVD");
	strcpy_s(mnemonic[ldi], "LDI");
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
		case 6: cout << "A number or \'(\' is expected."; break;
		case 7: cout << "A variable identifier is expected."; break;
		case 8: cout << "Assignment operator ':=' is expected."; break;
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
		sourceFile.getline(line, lineMax, '\n');
		while (line[lineLen++] != '\0') { /* do nothing */ }
		line[lineLen - 1] = ' ';
	}
	ch = line[charCount++];
}

//*******************************************************************//
//*******************************************************************//
//
//							void parseSymbol(void)
//
//*******************************************************************//
//*******************************************************************//
//Change: The input "a a" is taken as valid.
//Change: The input "aa" is taken as valid.
void compiler::parseSymbol(void)
{
	switch (ch)
	{
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
	case ':': GetCh();
		if (ch == '=') sym = assignSym;
		else error(8);
		GetCh(); break;
	case 'a': case 'b': case 'c': case 'A': case 'B': case 'C':
		sym = varIdentSym; GetCh(); break;
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
	do
	{
		getSym(); AssignStat(); //expression();
		if (sym == semicolonSym)
			CGprintNumOp();
	} while (sym == semicolonSym);

	if (sym != periodSym)
		error(5);
	else
	{
		CGprintNumOp();  CGHalt();
	}
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
//							void AssignStat(void)
//
//*******************************************************************//
//*******************************************************************//
//Change: take out the do while loop and make it so that "if a varIdentSym has already been accepted, the next one needs to be an assignSym
//Change: change the name of "variable" to AssignStat()
//accept(variable sym, 7)
//CGincrStack()
//accept(assgnSym,8)
//expression()
//CGassignment()
void compiler::AssignStat(void)
{
	accept(varIdentSym, 7);
	CGincrementStack(1);
	accept(assignSym, 8);
	expression();
	CGassignment();

	/*if (sym == varIdentSym)
	{
		int varNum = 0;
		do
		{
			accept(varIdentSym, 7);
			varNum++;
		} while (sym == varIdentSym);
		CGincrementStack(varNum);

		accept(assignSym, 8);
		expression();
		CGassignment();
	}
	else
		expression();*/
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
	// <factor> -> <number> | '(' <i-expression> ')'
	switch (sym)
	{
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
	if (op == ldi || op == inc)
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