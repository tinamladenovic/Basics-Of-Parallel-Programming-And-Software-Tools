/* Autor: Kristina Mladenović Datum: 05.06.2024. */

#ifndef __TYPES__
#define __TYPES__

#include "Constants.h"

#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <stack>

/**
* Supported token types.
*/
enum TokenType
{
	T_NO_TYPE,

	T_ID,			// abcd...
	T_M_ID,			// m123...
	T_R_ID,			// r123...
	T_NUM,			// 123...
	T_WHITE_SPACE,

	// reserved words
	T_MEM,			// _mem
	T_REG,			// _reg
	T_FUNC,			// _func
	T_ADD,			// add
	T_ADDI,			// addi
	T_SUB,			// sub
	T_LA,			// la
	T_LI,			// li
	T_LW,			// lw
	T_SW,			// sw
	T_BLTZ,			// bltz
	T_B,			// b
	T_NOP,			// nop

	// operators
	T_COMMA,		//,
	T_L_PARENT,		//(
	T_R_PARENT,		//)
	T_COL,			//:
	T_SEMI_COL,		//;

	// utility
	T_COMMENT,
	T_END_OF_FILE,
	T_ERROR,

	T_AND,      // and (logička operacija AND)
	T_OR,       // or (logička operacija OR)
	T_NOT,      // not (logička operacija NOT)
	T_BNE,      // bne (uslovni skok nejednakosti) - Instrukcija koja uslovno skoči na drugu lokaciju u programu ako dva operanda nisu jednaki.


};


/**
 * Instruction type.
 */
enum InstructionType
{
	I_NO_TYPE = 0,
	I_ADD,
	I_ADDI,
	I_SUB,
	I_LA,
	I_LI,
	I_LW,
	I_SW,
	I_BLTZ,
	I_B,
	I_NOP,
	I_AND,      // AND operacija - Izvršava logičku operaciju AND nad operandom.
	I_OR,       // OR operacija - Izvršava logičku operaciju OR nad operandima.
	I_NOT,      // NOT operacija - Izvršava logičku operaciju NOT nad operandom.
	I_BNE       // BNE instrukcija - Uslovni skok nejednakosti. Skoči na drugu lokaciju u programu ako dva operanda nisu jednaki.

};

/**
 * Reg names.
 */
enum Regs
{
	no_assign = 0,
	t0,
	t1,
	t2,
	t3
};

#endif
