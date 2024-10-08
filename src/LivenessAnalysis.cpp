﻿/* Autor: Kristina Mladenović Datum: 05.06.2024. */

#include "LivenessAnalysis.h"

// Konstruktor klase LivenessAnalysis. Inicijalizuje analizu na osnovu sintaksnog stabla
LivenessAnalysis::LivenessAnalysis(SyntaxAnalysis& syntax) :
	err(false), reg_vars(syntax.getRegs()), mem_vars(syntax.getMem()),
	instrs(syntax.getInstructions()), interferenceGraph()
{
	setPredAndSucc();
	setUseAndDef();
	for (int j = 0; j < (int)reg_vars.size(); ++j)
	{
		interferenceGraph.push_back({});
		for (int i = 0; i < (int)reg_vars.size(); ++i)
			interferenceGraph[j].push_back(0);
	}
}

//Izvršava analizu
bool LivenessAnalysis::Do()
{
	liveness();
	setGraph();
	resourceAllocation();

	return !err;
}

// Implementacija algoritma iterativnog računanja živosti instrukcija.
void LivenessAnalysis::liveness()
{
	bool prevGood = true;
	bool done = false;

	for (int counter = 0; !done && counter < 10; ++counter)
	{
		for (Instructions::reverse_iterator rit = instrs.rbegin(); rit != instrs.rend(); ++rit)
		{
			Instruction& curr = **rit;
			Variables savedIn = curr.getIn();
			Variables savedOut = curr.getOut();

			Variables& in = curr.getIn();
			Variables& out = curr.getOut();

			out.clear();
			out = curr.getSuccIns();

			in.clear();
			in = curr.getUseWithOutWithoutDef();

			if (prevGood)
				if (savedIn != in || savedOut != out)
				{
					prevGood = false;
				}
		}
		done = prevGood;
		prevGood = true;
		std::cout << ">>>>>=====-----\n"
			<< "| Iteration " << counter + 1 << ":\n"
			<< ">>>>>=====-----\n";
		print(instrs);
	}
}

// Formiranje grafa interferencije izlaznih varijabli instrukcija.
void LivenessAnalysis::setGraph()
{
	for (Instructions::iterator it = instrs.begin(); it != instrs.end(); ++it)
	{
		Instruction& i = **it;
		Variables& out = i.getOut();
		Variables& def = i.getDef();
		for (Variables::iterator it = def.begin(); it != def.end(); ++it)
		{
			Variable* definedVar = *it;
			if (contains(out, definedVar))
				for (Variable* v : out)
					if (v != definedVar)
						setInterference(v->getPos(), definedVar->getPos());
		}
	}
}

// Izvršava alociranje resursa (registara).
void LivenessAnalysis::resourceAllocation()
{
	std::stack<Variable*> simplificationStack = createSimplificationStack();

	Variable* in_use;
	Variable* prev = nullptr;

	while (!simplificationStack.empty()) {
		in_use = simplificationStack.top();
		simplificationStack.pop();

		vars.push_back(in_use);

		if (prev == nullptr) {
			int counter = 1;
			in_use->getAssignment() = (Regs)counter;
		}
		else {
			int color = getColor(in_use);
			if (color == -1) {
				err = true;
				return;
			}
			else {
				in_use->getAssignment() = (Regs)color;
			}
		}
		prev = in_use;
	}
}


// Uklanja element iz matrice na određenoj poziciji.
void removeElementOfMatrix(int element, std::vector<std::vector<int>>& matrix)
{
	std::vector<std::vector<int>>::iterator mit = matrix.begin();
	for (int i = 0; i < element; ++i)
		++mit;
	matrix.erase(mit);

	std::vector<int>::iterator mmit;
	for (mit = matrix.begin(); mit != matrix.end(); ++mit)
	{
		mmit = mit->begin();
		for (int i = 0; i < element; ++i)
			++mmit;
		mit->erase(mmit);
	}
}

// Pronalazi element sa najvećim rangu u matrici.
int findElementWithHighestRang(std::vector<std::vector<int>>& matrix)
{
	std::vector<int> count;
	int size = (int)matrix.size();
	for (int i = 0; i < size; ++i)
		count.push_back(0);

	for (int i = 0; i < size; ++i)
		for (int j = 0; j < i; ++j)
			if (matrix[j][i] == 1)
			{
				count[i]++;
				count[j]++;
			}

	int max = -1;
	for (int i = 0; i < size; ++i)
		if (count[i] < __REG_NUMBER__)
		{
			max = i;
			continue;
		}
	if (max == -1)
		throw std::runtime_error("Not enough registers!");

	for (int i = 0; i < size; ++i)
		if (count[max] < count[i] && count[i] < __REG_NUMBER__)
			max = i;

	return max;
}

// Kreira stek simplifikacije na osnovu grafa interferencije.
std::stack<Variable*> LivenessAnalysis::createSimplificationStack()
{
	std::stack<Variable*> result;

	Matrix matrixToWorkOn = interferenceGraph;
	Variables notYetTaken = reg_vars;
	Variables::iterator found;

	int curr;
	for (int i = 0; i < (int)interferenceGraph.size(); ++i)
	{
		curr = findElementWithHighestRang(matrixToWorkOn);
		removeElementOfMatrix(curr, matrixToWorkOn);

		found = notYetTaken.begin();
		for (int j = 0; j < curr; ++j)
			++found;

		result.push(*found);
		notYetTaken.erase(found);
	}

	return result;
}

// Dodeljuje boje (registre) varijablama na osnovu graf bojenja.
int LivenessAnalysis::getColor(Variable* var) {
	std::list<int> allReg;
	for (int i = 1; i <= __REG_NUMBER__; ++i)
		allReg.push_back(i);

	for (Variable* other : vars)
		if (interferenceGraph[var->getPos()][other->getPos()] == 1)
			allReg.remove((int)other->getAssignment());

	if (allReg.empty())
		return -1;
	else
		return allReg.front();
}

// Postavlja prethodnike i sledbenike instrukcija.
void LivenessAnalysis::setPredAndSucc()
{
	Instructions::iterator currentInstruction = instrs.begin();
	Instructions::iterator prevInstruction = currentInstruction++;

	Instruction* labeledInstruction;
	bool shouldAddToNext = true; 
	while (currentInstruction != instrs.end())
	{
		Instruction& curr = **currentInstruction;
		Instruction& prev = **prevInstruction;

		if (!prev.isFunc() && shouldAddToNext)
		{
			addEachother(curr, prev);
		}
		if (!shouldAddToNext)
			shouldAddToNext = true;

		switch (curr.getType())
		{
		case I_B:
			shouldAddToNext = false;
			labeledInstruction = findInstructionWithLabel(curr.getSrc().back(), instrs);
			if (labeledInstruction->isFunc())
				labeledInstruction = findInstructionAfterFunc(labeledInstruction, instrs);
			if (labeledInstruction != nullptr)
				addEachother(*labeledInstruction, curr);
			break;
		case I_BLTZ:
			labeledInstruction = findInstructionWithLabel(curr.getSrc().back(), instrs);
			if (labeledInstruction->isFunc())
				labeledInstruction = findInstructionAfterFunc(labeledInstruction, instrs);
			if (labeledInstruction != nullptr)
				addEachother(*labeledInstruction, curr);
		
		}
		prevInstruction = currentInstruction;
		++currentInstruction;
		if (currentInstruction != instrs.end())
			if ((*currentInstruction)->isFunc())
				++currentInstruction;
	}
}

// Postavlja upotrebu (use) i definiciju (def) varijabli u instrukcijama.
void LivenessAnalysis::setUseAndDef()
{
	for (Instruction* i : instrs)
	{
		i->setDef();
		i->setUse();
	}
}

// Ispisuje informacije o registrima.
void LivenessAnalysis::printRegisters()
{
	std::cout << ">>>>>=====-----\n"
		<< "|  Registers : \n"
		<< ">>>>>=====-----\n";
	print(reg_vars);
}

// Ispisuje graf interferencije.
void LivenessAnalysis::printGraph()
{
	std::cout << "=---===============---=\n"
		<< "| Interference Matrix |\n"
		<< "=---===============---=\n";
	for (int j = 0; j < (int)interferenceGraph.size(); ++j)
	{
		std::cout << "[";
		for (std::vector<int>::iterator it = interferenceGraph[j].begin();
		it != interferenceGraph[j].end();
			++it)
		{
			std::cout << ' ' << *it;
		}
		std::cout << " ]\n";
	}
}

// Postavlja interferenciju između dve varijable u grafu interferencije.
void LivenessAnalysis::setInterference(int x, int y)
{
	interferenceGraph[y][x] = 1;
	interferenceGraph[x][y] = 1;
}

// Upisuje generisanu asemblersku datoteku.
void LivenessAnalysis::writeToFile(std::string& nameOfOutputFile)
{
	std::ofstream file(nameOfOutputFile);
	if (!file.is_open())
		throw std::runtime_error("\nException! Wasn\'t able to create the output file!");

	file << ".globl " << instrs.front()->getLabel()->get() << "\n\n";

	file << ".data" << std::endl;
	for (Variable* v : mem_vars)
		file << v->get() << ":\t.word " << v->getValue() << std::endl;
	file << "\n";

	file << ".text" << std::endl;
	for (Instruction* i : instrs)
		file << *i << std::endl;

	file << "\tjr $ra";
	file.close();
}
