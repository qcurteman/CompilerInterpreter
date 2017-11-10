#include "HLL6_Compiler.h"
#include "ILL5_Interpreter.h"

void main()
{
	bool again = true;
	char user_input;
	while (again == true)
	{
		compiler myCompiler;
		interpreter myInterpreter;

		cout << "\n\nWould you like to test another file? Y/N: ";
		cin >> user_input;
		if (user_input == 'N' || user_input == 'n')
			again = false;
	}
}