// ============================================================================
//        __
//   \\__/ o\    (C) 2012-2017  Robert Finch, Waterloo
//    \  __ /    All rights reserved.
//     \/_//     robfinch<remove>@finitron.ca
//       ||
//
// C64 - 'C' derived language compiler
//  - 64 bit CPU
//
// This source file is free software: you can redistribute it and/or modify 
// it under the terms of the GNU Lesser General Public License as published 
// by the Free Software Foundation, either version 3 of the License, or     
// (at your option) any later version.                                      
//                                                                          
// This source file is distributed in the hope that it will be useful,      
// but WITHOUT ANY WARRANTY; without even the implied warranty of           
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            
// GNU General Public License for more details.                             
//                                                                          
// You should have received a copy of the GNU General Public License        
// along with this program.  If not, see <http://www.gnu.org/licenses/>.    
//                                                                          
// ============================================================================
//
#include "stdafx.h"
/*
 *	68000 C compiler
 *
 *	Copyright 1984, 1985, 1986 Matthew Brandt.
 *  all commercial rights reserved.
 *
 *	This compiler is intended as an instructive tool for personal use. Any
 *	use for profit without the written consent of the author is prohibited.
 *
 *	This compiler may be distributed freely for non-commercial use as long
 *	as this notice stays intact. Please forward any enhancements or questions
 *	to:
 *
 *		Matthew Brandt
 *		Box 920337
 *		Norcross, Ga 30092
 */

/*      global definitions      */

CPU cpu;
int maxPn = 15;
int gCpu = 7;
int regPC = 254;
int regSP = 7;
int regBP = 6;
int regLR = 29;
int regXLR = 13;
int regGP = 27;
int regCLP = 25;                // class pointer
int regZero = 0;
int regFirstRegvar = 11;
int regLastRegvar = 17;
int farcode = 0;
int wcharSupport = 1;
int verbose = 0;
int use_gp = 0;
int address_bits = 32;
int maxVL = 64;

int sizeOfWord = 4;
int sizeOfFP = 8;
int sizeOfFPS = 4;
int sizeOfFPD = 8;
int sizeOfFPT = 12;
int sizeOfFPQ = 16;
int sizeOfPtr = 4;

std::ifstream *ifs;
txtoStream ofs;
txtoStream lfs;
txtoStream dfs;
/*
FILE            *input = 0,
                *list = 0,
                *output = 0;*/
FILE			*outputG = 0;
int incldepth = 0;
int             lineno = 0;
int             nextlabel = 0;
int             lastch = 0;
int             lastst = 0;
char            lastid[128] = "";
char            lastkw[128] = "";
char            laststr[MAX_STRLEN + 1] = "";
int64_t			ival = 0;
double          rval = 0.0;
Float128		rval128;
char float_precision = 't';
//FloatTriple     FAC1,FAC2;
//FLOAT           rval = {0,0,0,0,0,0};
int parseEsc = TRUE;

TABLE           gsyms[257];// = {0,0},
	           
SYM             *lasthead = (SYM *)NULL;
Float128		*quadtab = nullptr;
struct slit     *strtab = (struct slit *)NULL;
struct clit		*casetab = (struct clit *)NULL;
int             lc_static = 0;
int             lc_auto = 0;
int				lc_thread = 0;
Statement    *bodyptr = 0;
int             global_flag = 1;
TABLE           defsyms;
int64_t         save_mask = 0;          /* register save mask */
int64_t         fpsave_mask = 0;
TYP             tp_int, tp_econst;
bool dogen = true;
int isKernel = FALSE;
int isPascal = FALSE;
int isOscall = FALSE;
int isInterrupt = FALSE;
int isTask = FALSE;
int isNocall = FALSE;
int optimize = TRUE;
int opt_noregs = FALSE;
int opt_nopeep;
int opt_noexpr = FALSE;
int opt_nocgo = FALSE;
int exceptions = FALSE;
int mixedSource = FALSE;
SYM *currentFn = (SYM *)NULL;
int callsFn = FALSE;

char nmspace[20][100];



