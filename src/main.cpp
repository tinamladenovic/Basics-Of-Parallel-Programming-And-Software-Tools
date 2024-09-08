/* Autor: Kristina Mladenović Datum: 05.06.2024. */

#include <iostream>
#include <exception>

#include "LivenessAnalysis.h"

using namespace std;

/*
Glavna funkcija programa koja pokreće analizu leksičkih, sintaksnih i živih analiza,
kao i alokaciju resursa. U slučaju bilo kakve greške, hvataju se izuzeci i ispisuje
se odgovarajuća poruka o grešci.
*/
int main()
{
	try
	{
		vector<string> fileNames =
		{
			".\\..\\examples\\simple.mavn",
			".\\..\\examples\\multiply.mavn",
			".\\..\\examples\\xor.mavn"
		};
		string outputFile = ".\\..\\examples\\out.s";
		bool retVal = false;

		LexicalAnalysis lex;

		// Učitavanje ulaznih fajlova
		if (!lex.readInputFile(fileNames[0]))
			throw runtime_error("\nException! Failed to open input file!\n");

		lex.initialize();

		// Pokretanje analize leksičkog programa
		retVal = lex.Do();

		if (retVal)
		{
			cout << "Lexical analysis finished successfully!" << endl;
			lex.printTokens();
		}
		else
		{
			lex.printLexError();
			throw runtime_error("\nException! Lexical analysis failed!\n");
		}

		SyntaxAnalysis syn(lex);
		retVal = syn.Do();
		if (retVal)
		{
			cout << "\nSyntax analysis finished successfully!" << endl;
			syn.printInstructions();
			syn.printVariables();
		}
		else
		{
			throw runtime_error("\nException! Syntax analysis failed!\n");
		}

		LivenessAnalysis la(syn);
		retVal = la.Do();
		if (retVal)
		{
			cout << "\nLiveness analysis and resource alocation finished successfully!" << endl;
			la.printGraph();
			la.printRegisters();
			la.writeToFile(outputFile);
		}
		else
		{
			throw runtime_error("\nException! Liveness analysis and resource alocation failed!\n");
		}
	}
	catch (runtime_error e)
	{
		cout << e.what() << endl;
		return 1;
	}
	catch (SyntaxAnalysis::SyntaxError e)
	{
		printError(e);
		return 1;
	}

	return 0;
}
