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

extern int lstackptr;
extern int options(char *);
extern int openfiles(char *);
extern void summary();
extern void ParseGlobalDeclarations();
extern void makename(char *s, int, char *e);
extern char *errtext(int errnum);
extern std::string *classname;
extern char *rtrim(char *);

Compiler::Compiler()
{
	throwlab = -1;
}

int Compiler::main2(int argc, char **argv)
{
	uctran_off = 0;
	optimize =1;
	exceptions=1;
	dfs.printf("c64 starting...\n");
	while(--argc) {
        if( **++argv == '-')
            options(*argv);
		else {
			if (PreprocessFile(*argv) == -1)
				break;
			if( openfiles(*argv)) {
				lineno = 0;
				initsym();
				lstackptr = 0;
				lastst = 0;
				NextToken();
				compile();
				summary();
//				MBlk::ReleaseAll();
//				ReleaseGlobalMemory();
				CloseFiles();
			}
        }
    }
	//getchar();
	return 0;
}

// Builds the debugging log as an XML document
//
void Compiler::compile()
{
	//GlobalDeclaration *gd;

	dfs.printf("<compile>\n");
	typenum = 1;
	symnum = 257;
	classname = nullptr;
	ZeroMemory(&gsyms[0],sizeof(gsyms));
	ZeroMemory(&defsyms,sizeof(defsyms));
	ZeroMemory(&tagtable,sizeof(tagtable));
	ZeroMemory(&symbolTable,sizeof(symbolTable));
	ZeroMemory(&typeTable,sizeof(typeTable));
	AddStandardTypes();

	decls = GlobalDeclaration::Make();
	gd = decls;
	lastst = tk_nop;

	getch();
	lstackptr = 0;
	lastst = 0;
	NextToken();
	try {
		while(lastst != my_eof)
		{
			if (gd==nullptr)
				break;
			dfs.printf("<Parsing GlobalDecl>\n");
			gd->Parse();
			dfs.printf("</Parsing GlobalDecl>\n");
			if( lastst != my_eof) {
				NextToken();
				gd->next = (Declaration *)GlobalDeclaration::Make();
				gd = (GlobalDeclaration*)gd->next;
			}
		}
		dfs.printf("</compile>\n");
	}
	catch (C64PException * ex) {
		dfs.printf(errtext(ex->errnum));
 		dfs.printf("</compile>\n");
	}
	dumplits();
}

int Compiler::PreprocessFile(char *nm)
{
	static char outname[1000];
	static char sysbuf[500];

	strcpy_s(outname, sizeof(outname), nm);
	makename(outname,sizeof(outname)-1,".fpp");
	sprintf_s(sysbuf, sizeof(sysbuf), "fpp -b %s %s", nm, outname);
	return system(sysbuf);
}

void Compiler::CloseFiles()
{    
	lfs.close();
	ofs.close();
	dfs.close();
	ifs->close();
}

void Compiler::AddStandardTypes()
{
	TYP *p, *pchar, *pint;

	p = TYP::Make(bt_short,1);
	stdint = p;
	pint = p;
	p->precision = 16;
  
	p = TYP::Make(bt_short,1);
	p->isUnsigned = true;
	p->precision = 16;
	stduint = p;
  
	p = TYP::Make(bt_long,2);
	p->precision = 32;
	stdlong = p;
  
	p = TYP::Make(bt_long,2);
	p->isUnsigned = true;
	p->precision = 32;
	stdulong = p;
  
	p = TYP::Make(bt_short,1);
	p->precision = 16;
	stdshort = p;
  
	p = TYP::Make(bt_short,1);
	p->isUnsigned = true;
	p->precision = 16;
	stdushort = p;
  
	p = TYP::Make(bt_char,1);
	stdchar = p;
	p->precision = 16;
	pchar = p;
  
	p = TYP::Make(bt_char,1);
	p->isUnsigned = true;
	p->precision = 16;
	stduchar = p;
  
	p = TYP::Make(bt_byte,1);
	stdbyte = p;
	p->precision = 8;
	pchar = p;
  
	p = TYP::Make(bt_ubyte,1);
	p->isUnsigned = true;
	p->precision = 8;
	stdubyte = p;
  
	p = TYP::alloc();
	p->type = bt_pointer;
	p->typeno = bt_pointer;
	p->val_flag = 1;
	p->size = 1;
	p->btp = pchar->GetIndex();
	p->bit_width = -1;
	p->precision = 16;
	stdstring = p;
  
	p = TYP::alloc();
	p->type = bt_double;
	p->typeno = bt_double;
	p->size = 4;
	p->bit_width = -1;
	p->precision = 64;
	stddbl = p;
	stddouble = p;
  
	p = TYP::alloc();
	p->type = bt_triple;
	p->typeno = bt_triple;
	p->size = 6;
	p->bit_width = -1;
	p->precision = 96;
	stdtriple = p;
  
	p = TYP::alloc();
	p->type = bt_quad;
	p->typeno = bt_quad;
	p->size = 8;
	p->bit_width = -1;
	p->precision = 128;
	stdquad = p;
  
	p = TYP::alloc();
	p->type = bt_float;
	p->typeno = bt_float;
	p->size = 2;
	p->bit_width = -1;
	p->precision = 32;
	stdflt = p;
  
	p = TYP::Make(bt_func,0);
	p->btp = pint->GetIndex();
	stdfunc = p;

	p = TYP::alloc();
	p->type = bt_exception;
	p->typeno = bt_exception;
	p->size = 1;
	p->isUnsigned = true;
	p->precision = 16;
	p->bit_width = -1;
	stdexception = p;

	p = TYP::alloc();
	p->type = bt_short;
	p->typeno = bt_short;
	p->val_flag = 1;
	p->size = 1;
	p->bit_width = -1;
	p->precision = 16;
	stdconst = p;
}
