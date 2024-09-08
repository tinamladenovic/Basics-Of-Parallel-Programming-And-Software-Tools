/* Autor: Kristina Mladenović Datum: 05.06.2024. */

#include "SyntaxAnalysis.h"
#include "Token.h"
#include "IR.h"

//  Konstruktor klase SyntaxAnalysis. Inicijalizuje promenljive i postavlja pokazivač na početak niske tokena.
SyntaxAnalysis::SyntaxAnalysis(LexicalAnalysis& lexer) :
	lex(lexer), currentToken(lexer.getTokenList().begin()),
	instrs(), reg_vars(), mem_vars(), label_vars(), const_vars(),
	err(false), eof(false), next_instruction_has_label(false) {}

// Destruktor klase SyntaxAnalysis. Oslobađa resurse alocirane za promenljive i instrukcije.
SyntaxAnalysis::~SyntaxAnalysis()
{
	for (Variables::iterator it = reg_vars.begin(); it != reg_vars.end(); ++it)
		delete* it;
	for (Variables::iterator it = mem_vars.begin(); it != mem_vars.end(); ++it)
		delete* it;
	for (Variables::iterator it = label_vars.begin(); it != label_vars.end(); ++it)
		delete* it;
	for (Variables::iterator it = const_vars.begin(); it != const_vars.end(); ++it)
		delete* it;
	for (Instructions::iterator it = instrs.begin(); it != instrs.end(); ++it)
		delete* it;
}

// Metoda koja pokreće sintaksnu analizu. Proverava tokene i poziva odgovarajuće metode za obradu instrukcija.
bool SyntaxAnalysis::Do()
{
	currentToken = lex.getTokenList().begin();

	Q();
	checkLabels();
	checkFunctions();

	return !err;
}

// Metoda za ispis instrukcija.
void SyntaxAnalysis::printInstructions()
{
	std::cout << ">>>>>======------\n"
		<< "| Instructions :\n"
		<< ">>>>>======------\n";
	print(instrs);
}

// Metoda za ispis promenljivih.
void SyntaxAnalysis::printVariables()
{
	std::cout << ">>>>>=====-----\n"
		<< "|  Variables : \n"
		<< ">>>>>=====-----\n";
	print(mem_vars);
	print(reg_vars);
	print(const_vars);
}

// Metoda koja vraća referencu na registre.
Variables& SyntaxAnalysis::getRegs()
{
	return reg_vars;
}

// Metoda koja vraća referencu na memoriju.
Variables& SyntaxAnalysis::getMem()
{
	return mem_vars;
}

// Metoda koja vraća referencu na instrukcije.
Instructions& SyntaxAnalysis::getInstructions()
{
	return instrs;
}

// Metoda koja konzumira očekivani token. U slučaju da token nije očekivani tip, baca izuzetak.
void SyntaxAnalysis::eat(TokenType token)
{
	if (token == T_END_OF_FILE)
	{
		eof = true;
	}

	if (currentToken->getType() == token)
	{
		++currentToken;
	}
	else {
		err = true;
		std::cerr << "Expected token is " <<
			tokenTypeToString(token) << " but " <<
			tokenTypeToString(currentToken->getType()) << " was given!" << std::endl;
		throw WRONG_TOKEN;
	}
}

// Metoda koja proverava tip sledećeg tokena. U slučaju da tip tokena nije očekivan, baca izuzetak.
void SyntaxAnalysis::glance(TokenType token)
{
	if (token != currentToken->getType())
	{
		err = true;
		std::cerr << "Wrong token type " <<
			tokenTypeToString(currentToken->getType()) <<
			", unable to read its contents for conversion!" << std::endl;
		throw WRONG_TOKEN;
	}
}

// Metoda koja proverava postojanje registarske promenljive sa datim imenom.
void SyntaxAnalysis::regVariableExists(std::string& name)
{
	for (Variables::iterator it = reg_vars.begin(); it != reg_vars.end(); ++it)
	{
		if ((*it)->getName() == name)
		{
			err = true;
			std::cerr << "Register variable with the same name already exists!" << std::endl;
			throw REGISTER_VAR_EXISTS;
		}
	}
}

// Metoda koja proverava postojanje memorijske promenljive sa datim imenom.
void SyntaxAnalysis::memVariableExists(std::string& name)
{
	for (Variables::iterator it = mem_vars.begin(); it != mem_vars.end(); ++it)
	{
		if ((*it)->getName() == name)
		{
			err = true;
			std::cerr << "Memory variable with the same name already exists!" << std::endl;
			throw MEMORY_VAR_EXISTS;
		}
	}
}

// Metoda koja proverava postojanje labele sa datim imenom.
void SyntaxAnalysis::labelExists(std::string& name)
{
	for (Variables::iterator it = label_vars.begin(); it != label_vars.end(); ++it)
	{
		if ((*it)->getName() == name && (*it)->getValue() == 1)
		{
			err = true;
			std::cerr << "Label with the same name already exists!" << std::endl;
			throw LABEL_EXISTS;
		}
	}
}

// Metoda koja kreira promenljivu na osnovu trenutnog tokena.
Variable* SyntaxAnalysis::createVariable()
{
	Variable* var;
	std::string name;
	switch (currentToken->getType())
	{
	case T_M_ID:
		name = currentToken->getValue();
		memVariableExists(name);
		eat(T_M_ID);

		glance(T_NUM);
		var = new Variable(Variable::MEM_VAR, name, stoi(currentToken->getValue()));
		eat(T_NUM);

		break;
	case T_R_ID:
		name = currentToken->getValue();
		regVariableExists(name);
		eat(T_R_ID);

		var = new Variable(Variable::REG_VAR, name);

		break;
	case T_ID:
		name = currentToken->getValue();
		labelExists(name);
		eat(T_ID);

		var = new Variable(Variable::LABEL_VAR, name, 1);

		break;
	default:
		err = true;
		std::cerr << "Expected a type of an ID token!" << std::endl;
		throw WRONG_TOKEN;
	}
	return var;
}

// Metoda koja pronalazi promenljivu na osnovu imena.
Variable* SyntaxAnalysis::findVariable()
{
	return findVariable(currentToken->getValue());
}

// Metoda koja kreira konstantnu promenljivu sa zadatom vrednošću.
Variable* SyntaxAnalysis::constVariable(int value)
{
	return findVariable("c" + std::to_string(value));
}

// Metoda koja pronalazi promenljivu na osnovu imena.
Variable* SyntaxAnalysis::findVariable(std::string& name)
{
	switch (name[0])
	{
	case 'r':
		for (Variables::iterator it = reg_vars.begin(); it != reg_vars.end(); ++it)
			if ((*it)->getName() == name)
				return *it;
		break;
	case 'm':
		for (Variables::iterator it = mem_vars.begin(); it != mem_vars.end(); ++it)
			if ((*it)->getName() == name)
				return *it;
		break;
	case 'c':
		for (Variables::iterator it = const_vars.begin(); it != const_vars.end(); ++it)
			if ((*it)->getName() == name)
				return *it;
		Variable* var = new Variable(Variable::CONST_VAR, "c" + name, stoi(name.replace(0, 1, "")));
		const_vars.push_back(var);
		return var;
	}
	err = true;
	std::cerr << "Variable not found!" << std::endl;
	throw VARIABLE_DOESNT_EXIST;
}

//  Metoda koja pronalazi labelu na osnovu imena.
Variable* SyntaxAnalysis::findLabel(std::string& name)
{
	for (Variables::iterator it = label_vars.begin(); it != label_vars.end(); ++it)
		if ((*it)->getName() == name)
			return *it;
	Variable* var = new Variable(Variable::LABEL_VAR, name, 0);
	label_vars.push_back(var);
	return var;
}

//  Metoda koja proverava postojanje labela.
void SyntaxAnalysis::checkLabels()
{
	for (Variables::iterator it = label_vars.begin(); it != label_vars.end(); ++it)
		if ((*it)->getValue() != 1)
		{
			err = true;
			std::cerr << "Label: " << (*it)->getName() << " doesn\'t exist!" << std::endl;
			throw LABEL_DOESNT_EXIST;
		}
}

// Metoda koja proverava postojanje funkcija.
void SyntaxAnalysis::checkFunctions()
{
	if (!(instrs.front()->isFunc()))
	{
		err = true;
		std::cerr << "No beginning!" << std::endl;
		throw NO_MAIN_FUNC;
	}
	int count = 0;
	for (Instruction* i : instrs)
		if (i->isFunc())
			++count;
	if (count != 1)
	{
		err = true;
		std::cerr << "There are more than one functions in your program!" << std::endl;
		throw TOO_MANY_FUNCS;
	}
}

// Funkcija Q
void SyntaxAnalysis::Q()
{
	next_instruction_has_label = false;

	S();  
	eat(T_SEMI_COL);  
	L();  
}

// Funkcija S
void SyntaxAnalysis::S()
{
	switch (currentToken->getType())
	{
	case T_MEM:
		eat(T_MEM);
		mem_vars.push_back(createVariable());
		break;
	case T_REG:
		eat(T_REG);
		reg_vars.push_back(createVariable());
		break;
	case T_FUNC:
		eat(T_FUNC);
		label_vars.push_back(createVariable());
		instrs.push_back(new Instruction(I_NO_TYPE, label_vars.back()));
		break;
	case T_ID:
		label_vars.push_back(createVariable());
		next_instruction_has_label = true;
		eat(T_COL);
		E();
		break;
	default:
		E();
	}
}

// Funkcija S
void SyntaxAnalysis::L()
{
	if (currentToken->getType() == T_END_OF_FILE) {
		eat(T_END_OF_FILE); 
	}
	else {
		Q();  
	}
}

// Funkcija E
void SyntaxAnalysis::E()
{
	Instruction* i;
	Variable* src1;
	Variable* src2;
	Variable* dst;
	switch (currentToken->getType())
	{
	case T_ADD:
		eat(T_ADD);
		i = new Instruction(I_ADD);

		glance(T_R_ID);
		dst = findVariable();
		eat(T_R_ID);
		eat(T_COMMA);

		glance(T_R_ID);
		src1 = findVariable();
		eat(T_R_ID);
		eat(T_COMMA);

		glance(T_R_ID);
		src2 = findVariable();
		eat(T_R_ID);

		i->addDst(dst);
		i->addSrc(src1);
		i->addSrc(src2);
		break;
	case T_ADDI:
		eat(T_ADDI);
		i = new Instruction(I_ADDI);

		glance(T_R_ID);
		dst = findVariable();
		eat(T_R_ID);
		eat(T_COMMA);

		glance(T_R_ID);
		src1 = findVariable();
		eat(T_R_ID);
		eat(T_COMMA);

		glance(T_NUM);
		src2 = constVariable(stoi(currentToken->getValue()));
		eat(T_NUM);

		i->addDst(dst);
		i->addSrc(src1);
		i->addSrc(src2);
		break;
	case T_SUB:
		eat(T_SUB);
		i = new Instruction(I_SUB);

		glance(T_R_ID);
		dst = findVariable();
		eat(T_R_ID);
		eat(T_COMMA);

		glance(T_R_ID);
		src1 = findVariable();
		eat(T_R_ID);
		eat(T_COMMA);

		glance(T_R_ID);
		src2 = findVariable();
		eat(T_R_ID);

		i->addDst(dst);
		i->addSrc(src1);
		i->addSrc(src2);
		break;
	case T_LA:
		eat(T_LA);
		i = new Instruction(I_LA);

		glance(T_R_ID);
		dst = findVariable();
		eat(T_R_ID);
		eat(T_COMMA);

		glance(T_M_ID);
		src1 = findVariable();
		eat(T_M_ID);

		i->addDst(dst);
		i->addSrc(src1);
		break;
	case T_LW:
		eat(T_LW);
		i = new Instruction(I_LW);

		glance(T_R_ID);
		dst = findVariable();
		eat(T_R_ID);
		eat(T_COMMA);

		glance(T_NUM);
		src1 = constVariable(stoi(currentToken->getValue()));
		eat(T_NUM);
		eat(T_L_PARENT);

		glance(T_R_ID);
		src2 = findVariable();
		eat(T_R_ID);
		eat(T_R_PARENT);

		i->addDst(dst);
		i->addSrc(src1);
		i->addSrc(src2);
		break;
	case T_LI:
		eat(T_LI);
		i = new Instruction(I_LI);

		glance(T_R_ID);
		dst = findVariable();
		eat(T_R_ID);
		eat(T_COMMA);

		glance(T_NUM);
		src1 = constVariable(stoi(currentToken->getValue()));
		eat(T_NUM);

		i->addDst(dst);
		i->addSrc(src1);
		break;
	case T_SW:
		eat(T_SW);
		i = new Instruction(I_SW);

		glance(T_R_ID);
		src1 = findVariable();
		eat(T_R_ID);
		eat(T_COMMA);

		glance(T_NUM);
		src2 = constVariable(stoi(currentToken->getValue()));
		eat(T_NUM);
		eat(T_L_PARENT);

		glance(T_R_ID);
		dst = findVariable();
		eat(T_R_ID);
		eat(T_R_PARENT);

		i->addSrc(src1);
		i->addSrc(src2);
		i->addSrc(dst);
		break;
	case T_B:
		eat(T_B);
		i = new Instruction(I_B);

		glance(T_ID);
		src1 = findLabel(currentToken->getValue());
		eat(T_ID);

		i->addSrc(src1);
		break;
	case T_BLTZ:
		eat(T_BLTZ);
		i = new Instruction(I_BLTZ);

		glance(T_R_ID);
		src1 = findVariable();
		eat(T_R_ID);
		eat(T_COMMA);

		glance(T_ID);
		src2 = findLabel(currentToken->getValue());
		eat(T_ID);

		i->addSrc(src1);
		i->addSrc(src2);
		break;
	case T_NOP:
		eat(T_NOP);
		i = new Instruction(I_NOP);
		break;
	case T_BNE:
		eat(T_BNE);
		i = new Instruction(I_BNE);

		glance(T_R_ID);
		src1 = findVariable();
		eat(T_R_ID);
		eat(T_COMMA);

		glance(T_R_ID);
		src2 = findVariable();
		eat(T_R_ID);
		eat(T_COMMA);

		glance(T_ID);
		dst = findLabel(currentToken->getValue());
		eat(T_ID);

		i->addSrc(src1);
		i->addSrc(src2);
		i->addSrc(dst);
		break;
	default:
		err = true;
		std::cerr << "No valid token found!" << std::endl;
		throw(WRONG_TOKEN);
	}
	if (next_instruction_has_label)
		i->addLabel(label_vars.back());
	instrs.push_back(i);
}

// Metoda koja ispisuje grešku sintakse.
void printError(SyntaxAnalysis::SyntaxError e)
{
	std::cout << "Error: ";
	switch (e)
	{
	case SyntaxAnalysis::MEMORY_VAR_EXISTS:
		std::cout << "Memory variable already exists" << std::endl;
		break;
	case SyntaxAnalysis::REGISTER_VAR_EXISTS:
		std::cout << "Register variable already exists" << std::endl;
		break;
	case SyntaxAnalysis::LABEL_EXISTS:
		std::cout << "Label already exists" << std::endl;
		break;
	case SyntaxAnalysis::WRONG_TOKEN:
		std::cout << "Wrong token found" << std::endl;
		break;
	case SyntaxAnalysis::VARIABLE_DOESNT_EXIST:
		std::cout << "Variable doesn\'t exist" << std::endl;
		break;
	case SyntaxAnalysis::LABEL_DOESNT_EXIST:
		std::cout << "Label doesn\'t exist" << std::endl;
		break;
	case SyntaxAnalysis::NO_MAIN_FUNC:
		std::cout << "No starting function" << std::endl;
		break;
	case SyntaxAnalysis::TOO_MANY_FUNCS:
		std::cout << "More than one function in code" << std::endl;
	}
}
