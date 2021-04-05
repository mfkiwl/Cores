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

TYP *TYP::alloc()
{
//  printf("TYP::alloc()\n");
	TYP *tp = (TYP *)&compiler.typeTable[compiler.typenum];
	ZeroMemory(tp,sizeof(TYP));
	tp->sname = new std::string("");
	tp->bit_width = -1;
	//	printf("Leave TYP::alloc():%p\n",tp);
	compiler.typenum++;
	if (compiler.typenum > 32760) {
		dfs.printf("Too many types\n");
		throw new C64PException(ERR_TOOMANY_SYMBOLS,1);
	}
	return tp;
};

TYP *TYP::GetBtp() {
    if (btp==0)
		return nullptr;
    return &compiler.typeTable[btp];
};
TYP *TYP::GetPtr(int n) {
	if (n==0)
		return nullptr;
	return &compiler.typeTable[n];
};
int TYP::GetIndex() { return this - &compiler.typeTable[0]; };

TYP *TYP::Copy(TYP *src)
{
	TYP *dst = nullptr;
 
	dfs.printf("<TYP__Copy>\n");
	if (src) {
		dst = TYP::alloc();
//		if (dst==nullptr)
//			throw gcnew C64::C64Exception();
		memcpy(dst,src,sizeof(TYP));
		dfs.printf("A");
		if (src->btp && src->GetBtp()) {
  		dfs.printf("B");
			dst->btp = Copy(src->GetBtp())->GetIndex();
		}
		dfs.printf("C");
		// We want to keep any base type indicator so Clear() isn't called.
		dst->lst.head = 0;
		dst->lst.tail = 0;
		dst->sname = new std::string(*src->sname);
		dfs.printf("D");
		TABLE::CopySymbolTable(&dst->lst,&src->lst);
	}
	dfs.printf("</TYP__Copy>\n");
	return dst;
}

TYP *TYP::Make(int bt, int siz)
{
	TYP *tp;
	dfs.puts("<TYP__Make>\n");
	tp = TYP::alloc();
	if (tp == nullptr)
		return nullptr;
	tp->val_flag = 0;
	tp->isArray = FALSE;
	tp->size = siz;
	tp->type = (e_bt)bt;
	tp->typeno = bt;
	tp->precision = siz * 8;
	dfs.puts("</TYP__Make>\n");
	return tp;
}

// Given just a type number return the size

int TYP::GetSize(int num)
{
  if (num < 0 || num > 32767)
    return 0;
  return compiler.typeTable[num].size;
}

// Basic type is one of the built in types supported by the compiler.
// Returns the basic type number for the type. The basic type number does
// not include complex types like struct, union, or class. For a struct,
// union, or class one of bt_struct, bt_union or bt_class is returned.

int TYP::GetBasicType(int num)
{
  if (num < 0 || num > 32767)
    return 0;
  return compiler.typeTable[num].type;
}

int TYP::GetHash()
{
	int n;
	TYP *p, *p1;

	n = 0;
	p = this;
	do {
		if (p->type==bt_pointer)
			n+=20000;
		p1 = p;
		p = p->GetBtp();
	} while (p);
	n += p1->typeno;
	return n;
}

