/* Autor: Kristina Mladenović Datum: 05.06.2024. */

#ifndef __IR__
#define __IR__

#include "Types.h"


/**
 * This class represents one variable from program code.
 */
class Variable
{
public:
	enum VariableType
	{
		MEM_VAR,    // Tip promenljive za memorijsku varijablu
		REG_VAR,    // Tip promenljive za registarsku varijablu
		LABEL_VAR,  // Tip promenljive za oznaku (labelu)
		CONST_VAR,  // Tip promenljive za konstantu
		NO_TYPE     // Bez tipa, odnosno neodređeno
	};

	//Variable() : m_type(NO_TYPE), m_name(""), m_position(-1), m_assignment(no_assign) {}
	//Variable(std::string name, int pos) : m_type(NO_TYPE), m_name(name), m_position(pos), m_assignment(no_assign) {}

	Variable() : m_type(NO_TYPE), m_name(""), m_position(-1), m_assignment(no_assign), value(-1) {}
	Variable(VariableType type, std::string name, int val = 0) : m_type(type), m_name(name), m_assignment(no_assign), value(val)
	{
		m_position = m_type == REG_VAR ? counter++ : -1;
	}

	// Metod za dobijanje imena promenljive
	std::string& getName();

	// Metod za dobijanje tipa promenljive
	VariableType& getType();

	// Metod za dobijanje vrednosti promenljive
	int& getValue();

	// Metod za dobijanje dodele promenljive
	Regs& getAssignment();

	// Konstantni metod za dobijanje pozicije promenljive
	int getPos() const;

	// Metod za dobijanje promenljive u formatu stringa
	std::string get();

	// Operator za ispisivanje promenljive u tok izlaza
	friend std::ostream& operator<<(std::ostream& out, Variable& var);

	// Prijateljska funkcija za ispisivanje liste promenljivih
	friend void print(std::list<Variable*>& vars);


private:
	VariableType m_type;
	std::string m_name;
	int m_position;
	Regs m_assignment;

	// Staticka promenljiva koja broji koliko je kreirano instanci klase
	static unsigned counter;

	// Vrednost promenljive
	int value;

	// Metod za ispisivanje tipa promenljive u formatu stringa
	std::string printType();

	// Metod za ispisivanje tabele (trenutnih vrednosti) promenljivih
	void printTable();

};


/**
 * This type represents list of variables from program code.
 */
typedef std::list<Variable*> Variables;


/**
 * This class represents one instruction in program code.
 */
class Instruction
{
public:
	//Instruction () : m_position(0), m_type(I_NO_TYPE) {}
	//Instruction (int pos, InstructionType type, Variables& dst, Variables& src) :
	//	m_position(pos), m_type(type), m_dst(dst), m_src(src) {}

	Instruction() : label(nullptr), m_position(counter++), m_type(I_NO_TYPE) {}
	Instruction(InstructionType type, Variable* lab = nullptr) :
		label(lab), m_position(counter++), m_type(type) {}

	// Dodaje labelu kao vezu za instrukciju
	void addLabel(Variable* lab);

	// Dodaje destinacionu promenljivu instrukcije
	void addDst(Variable* var);

	// Dodaje izvorne promenljive instrukcije
	void addSrc(Variable* var);

	// Dodaje prethodnu instrukciju kao prethodnika
	void addPred(Instruction* in);

	// Dodaje sledecu instrukciju kao sledbenika
	void addSucc(Instruction* in);

	// Postavlja da se promenljive koriste
	void setUse();

	// Postavlja da se promenljive definišu
	void setDef();

	// Vraća tip instrukcije
	InstructionType getType() const;

	// Vraća labelu instrukcije
	Variable* getLabel() const;

	// Vraća izvorne promenljive instrukcije
	Variables& getSrc();

	// Vraća promenljive ulaza instrukcije
	Variables& getIn();

	// Vraća promenljive izlaza instrukcije
	Variables& getOut();

	// Vraća promenljive definicije instrukcije
	Variables& getDef();

	// Vraća sledeće instrukcije
	Variables getSuccIns();

	// Vraća promenljive koje se koriste, ali se ne definišu
	Variables getUseWithOutWithoutDef();

	// Pronalazi instrukciju sa datom labelom
	friend Instruction* findInstructionWithLabel(Variable* lab, std::list<Instruction*>& ins);

	// Pronalazi instrukciju nakon funkcije
	friend Instruction* findInstructionAfterFunc(Instruction* in, std::list<Instruction*>& ins);

	// Dodaje vezu između instrukcija
	friend void addEachother(Instruction& successor, Instruction& predecessor);

	// Proverava da li je instrukcija funkcija
	bool isFunc();

	// Konvertuje instrukciju u string
	std::string toString();

	// Operator za ispis instrukcije
	friend std::ostream& operator<<(std::ostream& out, Instruction& in);

	// Ispisuje instrukcije
	friend void print(std::list<Instruction*>& ins);


private:
	int m_position;
	InstructionType m_type;
	
	Variables m_dst;
	Variables m_src;

	Variables m_use;
	Variables m_def;
	Variables m_in;
	Variables m_out;
	std::list<Instruction*> m_succ;
	std::list<Instruction*> m_pred;

	// Staticki brojač
	static unsigned counter;

	// Pokazivač na labelu
	Variable* label;

	// Ispisuje tabelu
	void printTable();

};


/**
 * This type represents list of instructions from program code.
 */
typedef std::list<Instruction*> Instructions;

// Definisanje tipa Instructions kao list<Instruction*>
typedef std::list<Instruction*> Instructions;

// Proverava da li lista vars sadrži var
bool contains(Variables& vars, Variable* var);

// Proverava da li lista ins sadrži in
bool contains(Instructions& ins, Instruction* in);


#endif
