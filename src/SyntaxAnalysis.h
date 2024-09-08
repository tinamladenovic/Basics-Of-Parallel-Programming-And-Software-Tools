/* Autor: Kristina Mladenović Datum: 05.06.2024. */

#pragma once

#ifndef SYNTAX_ANALYSIS_H
#define SYNTAX_ANALYSIS_H

#include <list>

#include "LexicalAnalysis.h"
#include "IR.h"

/**
* Class that analyses tokens gotten from lexical analysis
*/
class SyntaxAnalysis
{
public:
	// Enumeracija SyntaxError koja predstavlja moguće greške u sintaksi
	enum SyntaxError
	{
		MEMORY_VAR_EXISTS,    // Greška: Promenljiva već postoji kao memorijska
		REGISTER_VAR_EXISTS,  // Greška: Promenljiva već postoji kao registarska
		LABEL_EXISTS,         // Greška: Oznaka već postoji
		FUNCTION_EXISTS,      // Greška: Funkcija već postoji
		WRONG_TOKEN,          // Greška: Pogrešan token
		VARIABLE_DOESNT_EXIST,// Greška: Promenljiva ne postoji
		LABEL_DOESNT_EXIST,   // Greška: Oznaka ne postoji
		NO_MAIN_FUNC,         // Greška: Glavna funkcija ne postoji
		TOO_MANY_FUNCS        // Greška: Previše funkcija
	};

	/**
	* Constructor which prepares the object to do syntax analysis
	* [in] lexer - results gotten form the lexical analysis
	*/
	SyntaxAnalysis(LexicalAnalysis& lexer);

	/**
	* Destructor that ensures that all pointers are deleted when syntax
	* analysis object goes out of scope
	*/
	~SyntaxAnalysis();

	/**
	* Method which does syntax analysis
	* [out] return - boolean value if the operation was done without a problem
	*/
	bool Do();

	/**
	* instructions gotten from syntax analysis
	*/
	void printInstructions();
	/**
	* Print variables gotten from syntax analysis
	*/
	void printVariables();

	/**
	* Returns a reference to the list of register variables
	* [out] return - list of variables by reference
	*/
	Variables& getRegs();
	/**
	* Returns a reference to the list of memory variables
	* [out] return - list of variables by reference
	*/
	Variables& getMem();
	/**
	* Returns a reference to the list of instructions
	* [out] return - list of instructions by reference
	*/
	Instructions& getInstructions();

private:
	/**
	* Private method which moves the iterator to the next token
	(eats the upcoming one) and return if there has been an error
	* [in] token - expected token type to which to compare the current token the iterator is pointing at
	*/
	void eat(TokenType token);

	/**
	* Private method which checks if the iterator to the next token
	points to the correct type of token to be able to read it
	* [in] token - expected token type to which to compare the current token the iterator is pointing at
	*/
	void glance(TokenType token);

	/**
	* Check if register variable with the given name already exists and raise error if it does
	* [in] name - string to which to compare register variable names to
	*/
	void regVariableExists(std::string& name);
	/**
	* Check if memory variable with the given name already exists and raise error if it does
	* [in] name - string to which to compare memory variable names to
	*/
	void memVariableExists(std::string& name);
	/**
	* Check if label with the given name already exists and raise error if it does
	* [in] name - string to which to compare names of existing labels
	*/
	void labelExists(std::string& name);

	/**
	* Method which looks at the next token and turns it into a correct type of variable
	* [out] return - pointer to the created variable
	*/
	Variable* createVariable();
	/**
	* Method that returns a pointer to the variable with the same name as the current token looked at
	* [out] return - pointer to the found variable
	*/
	Variable* findVariable();
	/**
	* Method that returns a pointer to a constant variable
	* [in]  value - intiger number that represent the passed in value
	* [out] return - pointer to the wanted variable
	*/
	Variable* constVariable(int value);
	/**
	* Method that returns a pointer to the variable with the same name as the string given to it
	*
	* To create a constant variable pass in ("c" + value) as the argument
	* (creates a constant/immediate if it doesn't already exist, this is done so that all
	* constants in the program are saved as variables, but also so that if the same number is
	* used multiple times there is only one copy of it)
	*
	* [in]  name - string of the variable you are trying to find
	* [out] return - pointer to the found variable
	*/
	Variable* findVariable(std::string& name);
	/**
	* Method that returns a pointer to the variable with the same name as the string given to it
	* [in]  name - string of the variable you are trying to find
	* [out] return - pointer to the found variable
	*/
	Variable* findLabel(std::string& name);
	/**
	* Method that is used to raise an error at the end if a jump/branching was called
	* to a label that doesn't exist (is connected to nothing)
	*/
	void checkLabels();
	/**
	* Method for checking if there are multiple functions in the program and if the first instruction
	* is a function from which the assembly program can start from
	*/
	void checkFunctions();

	
	void Q();                   // Funkcija Q

	void S();                   // Funkcija S

	void L();                   // Funkcija S

	void E();                   // Funkcija E

	LexicalAnalysis& lex;       // Referenca na rezultate leksičke analize
	TokenList::iterator currentToken; // Trenutni token koji se analizira
	Instructions instrs;        // Lista instrukcija
	Variables reg_vars;         // Lista registarskih promenljivih
	Variables mem_vars;         // Lista promenljivih za memorijske adrese
	Variables label_vars;       // Lista oznaka
	Variables const_vars;       // Lista promenljivih koje drže konstantne vrednosti
	bool err;                   // Booleova vrednost koja pokazuje da li je došlo do greške
	bool eof;                   // Booleova vrednost koja predstavlja da li je pročitan EOF token
	bool next_instruction_has_label;  // Booleova vrednost koja, ako je tačna, govori da sledeća instrukcija treba da ima oznaku

};

/**
* Function that prints what type of syntax error was gotten
* [in] e - type of syntax error
*/
void printError(SyntaxAnalysis::SyntaxError e);

#endif

