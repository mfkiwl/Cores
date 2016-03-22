// ============================================================================
//        __
//   \\__/ o\    (C) 2012-2016  Robert Finch, Stratford
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

TYP *head = (TYP *)NULL;
TYP *tail = (TYP *)NULL;
std::string declid;
//char *Declaration::declid = (char *)NULL;
TABLE tagtable;
TYP stdconst;
Stringx names[20];
int nparms = 0;
int funcdecl = 0;		//0,1, or 2
int nfc = 0;
int isFirstCall = 0;
int catchdecl = FALSE;
int isTypedef = FALSE;
int isUnion = FALSE;
int isUnsigned = FALSE;
int isSigned = FALSE;
int isVolatile = FALSE;
int isVirtual = FALSE;
int isIO = FALSE;
int isConst = FALSE;
int missingArgumentName = FALSE;
int disableSubs;
int parsingParameterList = FALSE;
int unnamedCnt = 0;
int needParseFunction = FALSE;
int isStructDecl = FALSE;
int worstAlignment = 0;
char *stkname = 0;
std::string classname;
bool isPrivate = true;
std::string undeclid;
SYM *currentClass;

/* variable for bit fields */
static int		bit_max;	// largest bitnumber
int bit_offset;	/* the actual offset */
int      bit_width;	/* the actual width */
int bit_next;	/* offset for next variable */

int declbegin(int st);
void dodecl(int defclass);
SYM *ParseDeclarationSuffix(SYM *);
void declstruct(int ztype);
void structbody(TYP *tp, int ztype);
void ParseEnumDeclaration(TABLE *table);
void enumbody(TABLE *table);
extern int ParseClassDeclaration(int ztype);
extern ENODE *ArgumentList(ENODE *hidden,int*,int);
TYP *nameref2(std::string name, ENODE **node,int nt,bool alloc,int*);
SYM *search2(std::string na,TABLE *tbl,int *typearray);
SYM *gsearch2(std::string na, int *typearray);
extern TYP *CopyType(TYP *src);

int     imax(int i, int j)
{       return (i > j) ? i : j;
}


char *my_strdup(char *s)
{
	char *p;
	int n = strlen(s);
	int m = sizeof(char);
	p = (char *)allocx(sizeof(char)*(n+1));
	memcpy(p,s,sizeof(char)*(n));
	p[n] = '\0';
  return p;
}

void Declaration::SetType(SYM *sp)
{
  if (head) {
  	if (bit_width == -1)
  		sp->tp = head;
  	else {
  		sp->tp = allocTYP();
  		*(sp->tp) = *head;
  		sp->tp->type = bt_bitfield;
  		sp->tp->size = head->size;//tp_int.size;
  		sp->tp->bit_width = bit_width;
  		sp->tp->bit_offset = bit_offset;
  	}
  }
  else {
		sp->tp = allocTYP();
		sp->tp->type = bt_long;
		sp->tp->size = 8;
  }
}

// Ignore const
void Declaration::ParseConst()
{
	isConst = TRUE;
	NextToken();
}

void Declaration::ParseTypedef()
{
	isTypedef = TRUE;
	NextToken();
}

void Declaration::ParseNaked()
{
	isNocall = TRUE;
	head = (TYP *)TYP::Make(bt_oscall,8);
	tail = head;
	NextToken();
}

void Declaration::ParseLong()
{
	NextToken();
	if (lastst==kw_int) {
		NextToken();
	}
	else if (lastst==kw_float) {
		head = (TYP *)TYP::Make(bt_double,8);
		tail = head;
		NextToken();
	}
	else {
		if (isUnsigned) {
			head =(TYP *)TYP::Make(bt_ulong,8);
			tail = head;
		}
		else {
			head = (TYP *)TYP::Make(bt_long,8);
			tail = head;
		}
	}
	//NextToken();
	if (lastst==kw_task) {
		isTask = TRUE;
		NextToken();
	}
	if (lastst==kw_oscall) {
		isOscall = TRUE;
		NextToken();
	}
	else if (lastst==kw_nocall || lastst==kw_naked) {
		isNocall = TRUE;
		NextToken();
	}
	head->isUnsigned = isUnsigned;
	head->isVolatile = isVolatile;
	head->isIO = isIO;
	head->isConst = isConst;
	bit_max = 64;
}

void Declaration::ParseInt()
{
//printf("Enter ParseInt\r\n");
	if (isUnsigned) {
		head = TYP::Make(bt_ulong,8);
		tail = head;
	}
	else {
		head = TYP::Make(bt_long,8);
		tail = head;
  }
	if (head==nullptr)
    return;
	head->isUnsigned = isUnsigned;
	head->isVolatile = isVolatile;
	head->isIO = isIO;
	head->isConst = isConst;
	NextToken();
	if (lastst==kw_task) {
		isTask = TRUE;
		NextToken();
	}
	if (lastst==kw_oscall) {
		isOscall = TRUE;
		NextToken();
	}
	else if (lastst==kw_nocall || lastst==kw_naked) {
		isNocall = TRUE;
		NextToken();
	}
	bit_max = 64;
//printf("Leave ParseInt\r\n");
}

void Declaration::ParseInt32()
{
	if (isUnsigned) {
		head = (TYP *)TYP::Make(bt_ushort,4);
		tail = head;
	}
	else {
		head = (TYP *)TYP::Make(bt_short,4);
		tail = head;
	}
	bit_max = 32;
	NextToken();
	if( lastst == kw_int )
		NextToken();
	head->isUnsigned = isUnsigned;
	head->isVolatile = isVolatile;
	head->isIO = isIO;
	head->isConst = isConst;
	head->isShort = TRUE;
}

void Declaration::ParseInt8()
{
	if (isUnsigned) {
		head = (TYP *)TYP::Make(bt_ubyte,1);
		tail = head;
	}
	else {
		head =(TYP *)TYP::Make(bt_byte,1);
		tail = head;
	}
	head->isUnsigned = isUnsigned;
	head->isVolatile = isVolatile;
	head->isIO = isIO;
	head->isConst = isConst;
	NextToken();
	if (lastst==kw_oscall) {
		isOscall = TRUE;
		NextToken();
	}
	if (lastst==kw_nocall || lastst==kw_naked) {
		isNocall = TRUE;
		NextToken();
	}
	bit_max = 8;
}

void Declaration::ParseByte()
{
	if (isUnsigned) {
		head = (TYP *)TYP::Make(bt_ubyte,1);
		tail = head;
  }
	else {
		head =(TYP *)TYP::Make(bt_byte,1);
		tail = head;
  }
	NextToken();
	head->isUnsigned = !isSigned;
	head->isVolatile = isVolatile;
	head->isIO = isIO;
	head->isConst = isConst;
	bit_max = 8;
}

SYM *Declaration::ParseId()
{
	SYM *sp;

	sp = tagtable.Find(lastid,false);//gsyms[0].Find(lastid);
	if (sp==nullptr)
		sp = gsyms[0].Find(lastid,false);
	if (sp) {
		dfs.printf("Actually found type.\r\n");
		if (sp->storage_class==sc_typedef || sp->storage_class==sc_type) {
			NextToken();
			head = tail = sp->tp;
		}
		else
			head = tail = sp->tp;
//					head = tail = maketype(bt_long,4);
	}
	else {
		head = (TYP *)TYP::Make(bt_long,8);
		tail = head;
		bit_max = 64;
	}
	return sp;
}

// Parse a specifier. This is the first part of a declaration.
// Returns:
// 0 usually, 1 if only a specifier is present
//
int Declaration::ParseSpecifier(TABLE *table)
{
	SYM *sp;
	char *idsave;

dfs.printf("Enter ParseSpecifier\r\n");
	isUnsigned = FALSE;
	isSigned = FALSE;
	isVolatile = FALSE;
	isVirtual = FALSE;
	isIO = FALSE;
	isConst = FALSE;
dfs.printf("A");
	for (;;) {
		switch (lastst) {
				
			case kw_const:		ParseConst();	break;
			case kw_typedef:	ParseTypedef(); break;
			case kw_nocall:
			case kw_naked:		ParseNaked();	break;

			case kw_oscall:
				isOscall = TRUE;
				head = tail = (TYP *)TYP::Make(bt_oscall,8);
				NextToken();
				goto lxit;

			case kw_interrupt:
				isInterrupt = TRUE;
				head = (TYP *)TYP::Make(bt_interrupt,8);
				tail = head;
				NextToken();
				if (lastst==openpa) {
                    NextToken();
                    if (lastst!=id) 
                       error(ERR_IDEXPECT);
                    needpunc(closepa,49);
                    stkname = my_strdup(lastid);
                }
				goto lxit;

      case kw_virtual:
        dfs.printf("virtual");
        isVirtual = TRUE;
        NextToken();
        break;

			case kw_kernel:
				isKernel = TRUE;
				head =(TYP *) TYP::Make(bt_kernel,8);
				tail = head;
				NextToken();
				goto lxit;

			case kw_pascal:
				isPascal = TRUE;
				head = (TYP *)TYP::Make(bt_pascal,8);
				tail = head;
				NextToken();
				break;

			// byte and char default to unsigned unless overridden using
			// the 'signed' keyword
			//
			case kw_byte:   ParseByte(); goto lxit;
			
			case kw_char:
				if (isUnsigned) {
					head =(TYP *) TYP::Make(bt_uchar,2);
					tail = head;
				}
				else {
					head = (TYP *)TYP::Make(bt_char,2); 
					tail = head;
        }
				NextToken();
				head->isUnsigned = !isSigned;
				head->isVolatile = isVolatile;
				head->isIO = isIO;
				head->isConst = isConst;
				bit_max = 16;
				goto lxit;

			case kw_int16:
				if (isUnsigned) {
					head =(TYP *) TYP::Make(bt_uchar,2);
					tail = head;
				}
				else {
					head = (TYP *)TYP::Make(bt_char,2);
					tail = head;
				}
				NextToken();
				head->isUnsigned = isUnsigned;
				head->isVolatile = isVolatile;
				head->isIO = isIO;
				head->isConst = isConst;
				bit_max = 16;
				goto lxit;

			case kw_int32:
			case kw_short:	ParseInt32();	goto lxit;
			case kw_long:	ParseLong();	goto lxit;	// long, long int
			case kw_int64:
			case kw_int:	ParseInt();		goto lxit;

            case kw_task:
                isTask = TRUE;
                NextToken();
				break;

			case kw_int8: ParseInt8(); goto lxit;

			case kw_signed:
				isSigned = TRUE;
				NextToken();
				break;

			case kw_unsigned:
				NextToken();
				isUnsigned = TRUE;
				break;

			case kw_volatile:
				NextToken();
				if (lastst==kw_inout) {
                    NextToken();
                    isIO = TRUE;
                }
				isVolatile = TRUE;
				break;

			case ellipsis:
				head = (TYP *)TYP::Make(bt_ellipsis,8);
				tail = head;
				head->isVolatile = isVolatile;
				head->isIO = isIO;
				head->isConst = isConst;
				NextToken();
				bit_max = 32;
				goto lxit;

			case id:	sp = ParseId();	goto lxit;

			case kw_float:
				head = (TYP *)TYP::Make(bt_float,4);
				tail = head;
				head->isVolatile = isVolatile;
				head->isIO = isIO;
				head->isConst = isConst;
				NextToken();
				bit_max = 32;
				goto lxit;

			case kw_double:
				head = (TYP *)TYP::Make(bt_double,8);
				tail = head;
				head->isVolatile = isVolatile;
				head->isIO = isIO;
				head->isConst = isConst;
				NextToken();
				bit_max = 64;
				goto lxit;

			case kw_triple:
				head = (TYP *)TYP::Make(bt_triple,12);
				tail = head;
				head->isVolatile = isVolatile;
				head->isIO = isIO;
				head->isConst = isConst;
				NextToken();
				bit_max = 96;
				goto lxit;

			case kw_void:
				head = (TYP *)TYP::Make(bt_void,0);
				tail = head;
				head->isVolatile = isVolatile;
				head->isIO = isIO;
				head->isConst = isConst;
				NextToken();
				if (lastst==kw_interrupt) {
					isInterrupt = TRUE;
					NextToken();
				}
				if (lastst==kw_nocall || lastst==kw_naked) {
					isNocall = TRUE;
					NextToken();
				}
				bit_max = 0;
				goto lxit;

			case kw_enum:
				NextToken();
				ParseEnumDeclaration(table);
				bit_max = 16;
				goto lxit;

			case kw_class:
				ClassDeclaration::Parse(bt_class);
				goto lxit;

			case kw_struct:
				NextToken();
				if (StructDeclaration::Parse(bt_struct))
					return 1;
				goto lxit;

			case kw_union:
				NextToken();
				if (StructDeclaration::Parse(bt_union))
					return 1;
				goto lxit;

      case kw_exception:
				head = (TYP *)TYP::Make(bt_exception,8);
				tail = head;
				head->isVolatile = isVolatile;
				head->isIO = isIO;
				head->isConst = isConst;
				NextToken();
				bit_max = 64;
				goto lxit;
				
      case kw_inline:
        NextToken();
        break;

			default:
				goto lxit;
			}
	}
lxit:;
	return 0;
}

SYM *Declaration::ParsePrefix(char isUnion)
{   
	TYP *temp1, *temp2, *temp3, *temp4;
	SYM *sp, *parent, *sym;
	int nn;
	static char namebuf[1000];
	bool gotDouble = false;
	dfs.printf("ParseDeclPrefix(%d)\n",lastst);

	sp = nullptr;
j2:
	switch (lastst) {
		case kw_const:
			isConst = TRUE;
			NextToken();
			goto j2;

//		case ellipsis:
    case id:
dfs.printf("A");            
      declid = std::string(lastid);
      undeclid = std::string(lastid);
dfs.printf("B|%s|",(char *)declid.c_str());
      sp = allocSYM();
      strncpy(namebuf,lastid,990);
dfs.printf("C"); 
		  if (funcdecl==1) {
		    if (nparms > 19)
		      error(ERR_TOOMANY_PARAMS);
        else {
				  names[nparms].str = declid;
				  nparms++;
			  }
			}
dfs.printf("D"); 
      NextToken();
      while (lastst==double_colon) {
        gotDouble = true;
			  sym = tagtable.Find(lastid,false);
			  if (sym)
				   sp->parent = sym->GetIndex();//gsearch(lastid);
			  else {
			     sp->parent = 0;
			     break;
        }
        NextToken();
        if (lastst != id) {
          error(ERR_IDEXPECT);
          break;
        }
      }
      if (gotDouble)
        NextToken();
			currentClass = sp->GetParentPtr();
			if (sp->parent)
					dfs.printf("Setting parent:%s|\r\n",(char *)sp->GetParentPtr()->name->c_str());
			declid = my_strdup(lastid);
      sp->name = new std::string(declid);
dfs.printf("E"); 
			if (lastst == colon) {
				NextToken();
				bit_width = GetIntegerExpression((ENODE **)NULL);
				if (isUnion)
					bit_offset = 0;
				else
					bit_offset = bit_next;
				if (bit_width < 0 || bit_width > bit_max) {
					error(ERR_BITFIELD_WIDTH);
					bit_width = 1;
				}
				if (bit_width == 0 || bit_offset + bit_width > bit_max)
					bit_offset = 0;
				bit_next = bit_offset + bit_width;
//					SetType(sp);
dfs.printf("F"); 
				break;	// no ParseDeclarationSuffix()
			}
				//if (lastst==closepa) {
				//	return 1;
				//}
	//			if (lastst==closepa) {
	//				sp = search(lastid,&gsyms[0]);
	//				if (strcmp(lastid,"getchar")==0)
	//					printf("found");
	//				if (sp) {
	//					if (sp->storage_class==sc_typedef)
	//						head = tail = sp->tp;
	//					else
	//						head = tail = sp->tp;
	////					head = tail = maketype(bt_long,4);
	//				}
	//				else {
	//					head = tail = maketype(bt_long,8);
	//					bit_max = 64;
	//				}
	//				break;
	//			}
//				SetType(sp);
        sp->name = new std::string(declid);
				sp = ParseSuffix(sp);
        break;
      case star:
                temp1 = (TYP *)TYP::Make(bt_pointer,8);
                temp1->btp = head->GetIndex();
                head = temp1;
                if(tail == NULL)
                        tail = head;
                NextToken();
				//if (lastst==closepa) {	// (*)
				//	sprintf(buf,"_unnamed%d", unnamedCnt);
				//	unnamedCnt++;
				//	declid = my_strdup(buf);
				//	NextToken();
				//	ParseDeclarationSuffix();
				//	return 2;
				//}
				// Loop back to process additional prefix info.
				goto j2;
                //return ParseDeclarationPrefix(isUnion);
                break;
        case openpa:
                NextToken();
                temp1 = head;
                temp2 = tail;
                head = tail = (TYP *)NULL;	// It might be a typecast following.
				// Do we have (getchar)()
				sp = ParsePrefix(isUnion); 
				/*if (nn==1) {
					head = temp1;
					tail = temp2;
					goto j1;
				}*/
				//else if (nn == 2) {
				//	head = temp1;
				//	tail = temp2;
				//	NextToken();
				//	ParseDeclarationSuffix();
				//	break;
				//}
                needpunc(closepa,20);
                temp3 = head;
                temp4 = tail;
                head = temp1;
                tail = temp2;
//				SetType(sp);
                sp = ParseSuffix(sp);
				// (getchar)() returns temp4 = NULL
				if (temp4!=NULL) {
					temp4->btp = head->GetIndex();
					if(temp4->type == bt_pointer && temp4->val_flag != 0 && head != NULL)
						temp4->size *= head->size;
	                head = temp3;
				}
//				SetType(sp);
				//if (head==NULL)
				//	head = tail = maketype(bt_long,8);
                break;
        default:
                sp = ParseSuffix(sp);
                break;
        }
	return sp;
}


// Take care of trailing [] in a declaration. These indicate array indicies.

void Declaration::ParseSuffixOpenbr()
{
	TYP *temp1;
	long sz2;

  NextToken();
  temp1 = (TYP *)TYP::Make(bt_pointer,0);
  temp1->val_flag = 1;
  temp1->isArray = TRUE;
  temp1->btp = head->GetIndex();
  if(lastst == closebr) {
    temp1->size = 0;
    NextToken();
  }
  else if(head != NULL) {
    sz2 = GetIntegerExpression((ENODE **)NULL);
    temp1->size = sz2 * head->size;
	  temp1->alignment = head->alignment;
		needpunc(closebr,21);
  }
  else {
    sz2 = GetIntegerExpression((ENODE **)NULL);
	  temp1->size = sz2;
	  needpunc(closebr,22);
	}
  head = temp1;
  if( tail == NULL)
    tail = head;
}

// Take care of following open parenthesis (). These indicate a function
// call. There may or may not be following parameters. A following '{' is
// looked for and if found a flag is set to parse the function body.

void Declaration::ParseSuffixOpenpa(SYM *sp)
{
	TYP *temp1;
	TYP *tempHead, *tempTail;
	int fd;
	std::string odecl;
	int isd;
	int nump;
	bool isFuncPtr = false;

  dfs.printf("<openpa>\n");
  dfs.printf("****************************\n");
  dfs.printf("****************************\n");
  dfs.printf("Function: %s\n", (char *)sp->name->c_str());
  dfs.printf("****************************\n");
  dfs.printf("****************************\n");
  NextToken();
  isFuncPtr = head->type==bt_pointer;
  temp1 =(TYP *) TYP::Make(bt_func,0/*isFuncPtr ? bt_func : bt_ifunc,0*/);
  temp1->val_flag = 1;
  dfs.printf("o ");
  if (isFuncPtr) {
    temp1->btp = head->btp;
    head->btp = temp1->GetIndex();
  }
  else {
	  temp1->btp= head->GetIndex();
	  head = temp1;
  }
  dfs.printf("p ");
  if (tail==NULL) {
	  if (temp1->GetBtp())
		  tail = temp1->GetBtp();
	  else
		  tail = temp1;
  }
  dfs.printf("q ");
  needParseFunction = TRUE;
  sp->params.Clear();
  if(lastst == closepa) {
    NextToken();
	  if(lastst == begin) {
      temp1->type = bt_ifunc;
	  }
	  else {
      temp1->type = bt_func;
		  needParseFunction = FALSE;
		  dfs.printf("Set false\n");
	  }
	  currentFn = sp;
	  sp->parent = currentClass->GetIndex();
	  sp->NumParms = 0;
  }
  else {
    dfs.printf("r");
	  currentFn = sp;
	  sp->parent = currentClass->GetIndex();
    temp1->type = bt_func;
  	// Parse the parameter list for a function pointer passed as a
  	// parameter.
  	// Parse parameter list for a function pointer defined within
  	// a structure.
  	if (parsingParameterList || isStructDecl) {
      dfs.printf("s ");
  		int pacnt;
  		int nn;
  		fd = funcdecl;
  		needParseFunction = FALSE;
  	  dfs.printf("Set false\n");
  		odecl = declid;
  		tempHead = head;
  		tempTail = tail;
  		isd = isStructDecl;
  		//ParseParameterDeclarations(10);	// parse and discard
  		funcdecl = 10;
  //				SetType(sp);
  		sp->BuildParameterList(&nump);
  		needParseFunction = FALSE;
  	  dfs.printf("Set false\n");
  //				sp->parms = sym;
  		sp->NumParms = nump;
  		isStructDecl = isd;
  		head = tempHead;
  		tail = tempTail;
  		declid = odecl;
  		funcdecl = fd;
  		needpunc(closepa,23);
  
  		if (lastst==begin) {
  		  needParseFunction = TRUE;
  		  dfs.printf("Set true1\n");
  			if (sp->params.GetHead() && sp->proto.GetHead()) {
  			  dfs.printf("Matching parameter types to prototype.\n");
  			  if (!sp->ParameterTypesMatch(sp))
  			     error(ERR_PARMLIST_MISMATCH);
  		  }
  			temp1->type = bt_ifunc;
  		}
  		// If the delaration is ending in a semicolon then it was really
  		// a function prototype, so move the parameters to the prototype
  		// area.
  		else if (lastst==semicolon) {
  			sp->params.CopyTo(&sp->proto);
      }
  	  else {
  	    error(ERR_SYNTAX);
  	  }
      dfs.printf("Z\r\n");
//				if (isFuncPtr)
//					temp1->type = bt_func;
//				if (lastst != begin)
//					temp1->type = bt_func;
//				if (lastst==begin) {
//					ParseFunction(sp);
//				}
    }
	  sp->PrintParameterTypes();
  }
}


// Take care of the () or [] trailing part of a declaration.
// There could be multiple sets of [] so a loop is formed to accomodate
// this. There will be only a single set of () indicating parameters.

SYM *Declaration::ParseSuffix(SYM *sp)
{
	dfs.printf("<ParseDeclSuffix>\n");

  while(true) {
    switch (lastst) {

    case openbr:
      ParseSuffixOpenbr();  
      break;                // We want to loop back for more brackets
  
    case openpa:
    	// The declaration doesn't have to have an identifier name; it could
    	// just be a type chain. so sp incoming might be null. We need a place
    	// to stuff the parameter / protoype list so we may as well create
    	// the symbol here if it isn't yet defined.
    	if (sp==nullptr)
    		sp = allocSYM();
      ParseSuffixOpenpa(sp);
      goto lxit;
      
    default:
      goto lxit;
    }
  }
lxit:
  dfs.printf("</ParseDeclSuffix>\n");
	return sp;
}

// Get the natural alignment for a given type.

int alignment(TYP *tp)
{
	//printf("DIAG: type NULL in alignment()\r\n");
	if (tp==NULL)
		return AL_BYTE;
	switch(tp->type) {
	case bt_byte:	case bt_ubyte:	return AL_BYTE;
	case bt_char:   case bt_uchar:  return AL_CHAR;
	case bt_short:  case bt_ushort: return AL_SHORT;
	case bt_long:   case bt_ulong:  return AL_LONG;
  case bt_enum:           return AL_CHAR;
  case bt_pointer:
            if(tp->val_flag)
                return alignment(tp->GetBtp());
            else
				return AL_POINTER;
  case bt_float:          return AL_FLOAT;
  case bt_double:         return AL_DOUBLE;
  case bt_triple:         return AL_TRIPLE;
	case bt_class:
  case bt_struct:
  case bt_union:          
    return (tp->alignment) ?  tp->alignment : AL_STRUCT;
  default:                return AL_CHAR;
  }
}


// Figure out the worst alignment required.

int walignment(TYP *tp)
{
	SYM *sp;

	//printf("DIAG: type NULL in alignment()\r\n");
	if (tp==NULL)
		return imax(AL_BYTE,worstAlignment);
	switch(tp->type) {
	case bt_byte:	case bt_ubyte:		return imax(AL_BYTE,worstAlignment);
	case bt_char:   case bt_uchar:     return imax(AL_CHAR,worstAlignment);
	case bt_short:  case bt_ushort:    return imax(AL_SHORT,worstAlignment);
	case bt_long:   case bt_ulong:     return imax(AL_LONG,worstAlignment);
    case bt_enum:           return imax(AL_CHAR,worstAlignment);
    case bt_pointer:
            if(tp->val_flag)
                return imax(alignment(tp->GetBtp()),worstAlignment);
            else
				return imax(AL_POINTER,worstAlignment);
    case bt_float:          return imax(AL_FLOAT,worstAlignment);
    case bt_double:         return imax(AL_DOUBLE,worstAlignment);
    case bt_triple:         return imax(AL_TRIPLE,worstAlignment);
	case bt_class:
    case bt_struct:
    case bt_union:          
		sp =(SYM *) sp->GetPtr(tp->lst.GetHead());
        worstAlignment = tp->alignment;
		while(sp != NULL) {
            if (sp->tp && sp->tp->alignment) {
                worstAlignment = imax(worstAlignment,sp->tp->alignment);
            }
            else
     			worstAlignment = imax(worstAlignment,walignment(sp->tp));
			sp = sp->GetNextPtr();
        }
		return worstAlignment;
    default:                return imax(AL_CHAR,worstAlignment);
    }
}

int roundAlignment(TYP *tp)
{
	worstAlignment = 0;
	if (tp->type == bt_struct || tp->type == bt_union || tp->type==bt_class) {
		return walignment(tp);
	}
	return alignment(tp);
}


// Round the size of the type up according to the worst alignment.

int roundSize(TYP *tp)
{
	int sz;
	int wa;

	worstAlignment = 0;
	if (tp->type == bt_struct || tp->type == bt_union || tp->type == bt_class) {
		wa = walignment(tp);
		sz = tp->size;
		if (sz == 0)
			return 0;
		while(sz % wa)
			sz++;
		return sz;
	}
	return tp->size;
}

// When going to insert a class method, check the base classes to see if it's
// a virtual function override. If it's an override, then add the method to
// the list of overrides for the virtual function.

void InsertMethod(SYM *sp)
{
  int nn;
  SYM *parentSym;

  dfs.puts("<InsertMethod>");
  sp->GetParentPtr()->tp->lst.insert(sp);
  nn = sp->GetParentPtr()->tp->lst.FindRising(*sp->name);
  if (nn) {
    dfs.puts("Found method:");
    parentSym = sp->FindExactMatch(TABLE::matchno);
    if (parentSym != sp->GetParentPtr()) {         // ignore entry just added
      dfs.puts("Found in a base class:");
      if (parentSym->IsVirtual) {
        dfs.printf("Found virtual:");
        parentSym->AddDerived(sp);
      }
    }
  }
  dfs.printf("</InsertMethod>\n");
}

/*
 *      process declarations of the form:
 *
 *              <type>  <specifier>, <specifier>...;
 *
 *      leaves the declarations in the symbol table pointed to by
 *      table and returns the number of bytes declared. al is the
 *      allocation type to assign, ilc is the initial location
 *      counter. if al is sc_member then no initialization will
 *      be processed. ztype should be bt_struct for normal and in
 *      structure ParseSpecifierarations and sc_union for in union ParseSpecifierarations.
 */
int Declaration::declare(SYM *parent,TABLE *table,int al,int ilc,int ztype)
{ 
	SYM *sp, *sp1, *sp2;
  TYP *dhead, *tp1, *tp2;
	ENODE *ep1, *ep2;
	int op;
	int fd;
	int fn_doneinit = 0;
	int bcnt;
	bool flag;
	int parentBytes = 0;
  char buf[20];
 
    static long old_nbytes;
    int nbytes;

	dfs.printf("Enter declare()\r\n");
	nbytes = 0;
dfs.printf("A");
	classname = "";
	if (ParseSpecifier(table))
		goto xit1;
dfs.printf("B");
    dhead = head;
    for(;;) {
        declid = "";
dfs.printf("b");
        bit_width = -1;
        sp = ParsePrefix(ztype==bt_union);
		// If a function declaration is taking place and just the type is
		// specified without a parameter name, assign an internal compiler
		// generated name.
		if (funcdecl>0 && funcdecl != 10 && declid.length()==0) {
			sprintf(buf, "_p%d", nparms);
			declid = my_strdup(buf);
			if (nparms > 19) {
			  error(ERR_TOOMANY_PARAMS);
	    }
	    else {
			  names[nparms].str = declid;
			  nparms++;
		  }
			missingArgumentName = TRUE;
		}
dfs.printf("C");
        if( declid.length() > 0 || classname.length()!=0) {      /* otherwise just struct tag... */
			if (sp == nullptr) {
	            sp = allocSYM();
			}
			SetType(sp);
			if (sp->parent==nullptr)
				sp->parent = parent->GetIndex();
			if (al==sc_member)
				sp->IsPrivate = isPrivate;
			else
				sp->IsPrivate = false;
            sp->SetName(classname.length() > 0 ? classname : declid);
			sp->shortname = new std::string(undeclid);
dfs.printf("D");
			classname = "";
			sp->IsVirtual = isVirtual;
      sp->storage_class = al;
      sp->isConst = isConst;
			if (bit_width > 0 && bit_offset > 0) {
				// share the storage word with the previously defined field
				nbytes = old_nbytes - ilc;
			}
			old_nbytes = ilc + nbytes;
			if (al != sc_member) {
//							sp->isTypedef = isTypedef;
				if (isTypedef)
					sp->storage_class = sc_typedef;
				isTypedef = FALSE;
			}
dfs.printf("E");
			if ((ilc + nbytes) % roundAlignment(head)) {
				if (al==sc_thread)
					tseg();
				else
					dseg();
            }
            bcnt = 0;
            while( (ilc + nbytes) % roundAlignment(head)) {
                ++nbytes;
                bcnt++;
            }
            if( al != sc_member && al != sc_external && al != sc_auto) {
                if (bcnt > 0)
                    genstorage(bcnt);
            }

dfs.printf("F");
			if (sp->parent) {
dfs.printf("f:%d",sp->parent);
        if (sp->GetParentPtr()->tp==nullptr) {
dfs.printf("f:%d",sp->parent);
           dfs.printf("null type pointer.\n");
           parentBytes = 0;
        }
        else {
				  parentBytes = sp->GetParentPtr()->tp->size;
				  dfs.printf("ParentBytes=%d\n",parentBytes);
			  }
			}
			else
				parentBytes = 0;

			// Set the struct member storage offset.
			if( al == sc_static || al==sc_thread) {
				sp->value.i = nextlabel++;
			}
			else if( ztype == bt_union) {
                sp->value.i = ilc + parentBytes;
			}
            else if( al != sc_auto ) {
                sp->value.i = ilc + nbytes + parentBytes;
			}
			// Auto variables are referenced negative to the base pointer
			// Structs need to be aligned on the boundary of the largest
			// struct element. If a struct is all chars this will be 2.
			// If a struct contains a pointer this will be 8. It has to
			// be the worst case alignment.
			else {
                sp->value.i = -(ilc + nbytes + roundSize(head) + parentBytes);
			}

//			SetType(sp);

dfs.printf("G");
			if (isConst)
				sp->tp->isConst = TRUE;
            if((sp->tp->type == bt_func) && sp->storage_class == sc_global )
                sp->storage_class = sc_external;

			// Increase the storage allocation by the type size.
            if(ztype == bt_union)
                nbytes = imax(nbytes,roundSize(sp->tp));
			else if(al != sc_external) {
				// If a pointer to a function is defined in a struct.
				if (isStructDecl) {
					if (sp->tp->type==bt_func) {
						nbytes += 8;
					}
					else if (sp->tp->type != bt_ifunc) {
						nbytes += roundSize(sp->tp);
					}
				}
				else {
					nbytes += roundSize(sp->tp);
				}
			}

dfs.printf("H");
      //dfs.printf("Table:%p, sp:%p Fn:%p\r\n", table, sp, currentFn);
      if (sp->parent)
         sp1 = sp->GetParentPtr()->tp->lst.Find(*sp->name,false);
      else
			    sp1 = table->Find(*sp->name,false);
dfs.printf("h");
      if (sp->tp) {
dfs.printf("h1");
			  if (sp->tp->type == bt_ifunc || sp->tp->type==bt_func) {
dfs.printf("h2");
				  sp1 = sp->FindExactMatch(TABLE::matchno);
dfs.printf("i");
			  }
		  }
			else {
dfs.printf("j");
				if (TABLE::matchno)
					sp1 = TABLE::match[TABLE::matchno-1];
				else
					sp1 = nullptr;
			}
dfs.printf("k");
			flag = false;
			if (sp1) {
			  if (sp1->tp) {
dfs.printf("l");
				   flag = sp1->tp->type == bt_func;
	      }
			}
dfs.printf("I");
			if (sp->tp->type == bt_ifunc && flag)
			{
dfs.printf("Ia");
				dfs.printf("bt_ifunc\r\n");
				sp1->tp = sp->tp;
				sp1->storage_class = sp->storage_class;
        sp1->value.i = sp->value.i;
				sp1->IsPrototype = sp->IsPrototype;
				sp1->IsVirtual = sp->IsVirtual;
				sp1->parent = sp->parent;
				sp1->params = sp->params;
				sp1->proto = sp->proto;
				sp1->lsyms = sp->lsyms;
				sp = sp1;
            }
			else {
dfs.printf("Ib");
				// Here the symbol wasn't found in the table.
				if (sp1 == nullptr) {
dfs.printf("Ic");
          if ((sp->tp->type==bt_class || sp->tp->type == bt_struct || sp->tp->type==bt_union)
             && (sp->storage_class == sc_type || sp->storage_class==sc_typedef))
            ; // Do nothing. The class was already entered in the tag table.
          else {
            dfs.printf("insert type: %d\n", sp->tp->type);
					  dfs.printf("***Inserting:%s into %p\n",(char *)sp->name->c_str(), (char *) table);
					  if (sp->parent) {
					    InsertMethod(sp);
			      }
			      else
					    table->insert(sp);
				  }
				}
			}
dfs.printf("J");
			if (needParseFunction) {
				needParseFunction = FALSE;
				fn_doneinit = ParseFunction(sp);
				if (sp->tp->type != bt_pointer)
					return nbytes;
			}
   //         if(sp->tp->type == bt_ifunc) { /* function body follows */
   //             ParseFunction(sp);
   //             return nbytes;
   //         }
dfs.printf("K");
            if( (al == sc_global || al == sc_static || al==sc_thread) && !fn_doneinit &&
                    sp->tp->type != bt_func && sp->tp->type != bt_ifunc && sp->storage_class!=sc_typedef)
                    doinit(sp);
        }
		if (funcdecl>0) {
			if (lastst==comma || lastst==semicolon) {
				break;
			}
			if (lastst==closepa) {
				goto xit1;
			}
		}
		else if (catchdecl==TRUE) {
			if (lastst==closepa)
				goto xit1;
		}
		else if (lastst == semicolon) {
			if (sp) {
				if (sp->tp) {
					if (sp->tp->type==bt_class) {
						int typearray[20];
						int typearray2[20];
		/*
						// First see if there is a ctor. If there are no ctors there's
						// nothing to do.
						memset(typearray,0,sizeof(typearray));
						sp1 = search2(sp->tp->sname,&sp->tp->lst,typearray);
						if (sp1) {
							// Build an expression that references the ctor.
							tp1 = nameref2(sp->tp->sname,&ep1,TRUE,false,typearray);
							// Create a function call node for the ctor.
							if (tp1!=nullptr) {
								memcpy(typearray,GetParameterTypes(sp),sizeof(typearray));
								ep1 = makenode(en_fcall, ep1, nullptr);
							}
							sp->initexp = ep1;
						}
		*/
					}
				}
			}
			break;
		}
		else if (lastst == assign) {
			tp1 = nameref(&ep1,TRUE);
            op = en_assign;
//            NextToken();
            tp2 = asnop(&ep2);
            if( tp2 == 0 || !IsLValue(ep1) )
                  error(ERR_LVALUE);
            else    {
                    tp1 = forcefit(&ep1,tp1,&ep2,tp2);
                    ep1 = makenode(op,ep1,ep2);
                    }
			sp->initexp = ep1;
			if (lastst==semicolon)
				break;
		}
j1:
        needpunc(comma,24);
        if(declbegin(lastst) == 0)
                break;
        head = dhead;
    }
    NextToken();
xit1:
//	printf("Leave declare()\r\n");
    return nbytes;
}

int declbegin(int st)
{
	return st == star || st == id || st == openpa || st == openbr; 
}

void GlobalDeclaration::Parse()
{
	dfs.printf("Enter ParseGlobalDecl");
 for(;;) {
    currentClass = nullptr;
    currentFn = nullptr;
    currentStmt = nullptr;
		worstAlignment = 0;
		funcdecl = 0;
		switch(lastst) {
		case ellipsis:
		case id:
        case kw_kernel:
		case kw_interrupt:
        case kw_task:
		case kw_cdecl:
		case kw_pascal:
		case kw_naked:
		case kw_nocall:
		case kw_oscall:
		case kw_typedef:
    case kw_virtual:
		case kw_volatile: case kw_const:
        case kw_exception:
		case kw_int8: case kw_int16: case kw_int32: case kw_int64:
		case kw_byte: case kw_char: case kw_int: case kw_short: case kw_unsigned: case kw_signed:
        case kw_long: case kw_struct: case kw_union: case kw_class:
        case kw_enum: case kw_void:
        case kw_float: case kw_double:
                lc_static += declare(NULL,&gsyms[0],sc_global,lc_static,bt_struct);
				break;
        case kw_thread:
				NextToken();
                lc_thread += declare(NULL,&gsyms[0],sc_thread,lc_thread,bt_struct);
				break;
		case kw_register:
				NextToken();
                error(ERR_ILLCLASS);
                lc_static += declare(NULL,&gsyms[0],sc_global,lc_static,bt_struct);
				break;
		case kw_private:
        case kw_static:
                NextToken();
				lc_static += declare(NULL,&gsyms[0],sc_static,lc_static,bt_struct);
                break;
    case kw_extern:
        NextToken();
				if (lastst==kw_pascal) {
					isPascal = TRUE;
					NextToken();
				}
				if (lastst==kw_kernel) {
					isKernel = TRUE;
					NextToken();
				}
				else if (lastst==kw_oscall || lastst==kw_interrupt || lastst==kw_nocall || lastst==kw_naked)
					NextToken();
          ++global_flag;
          declare(NULL,&gsyms[0],sc_external,0,bt_struct);
          --global_flag;
          break;
    case kw_inline:
      NextToken();
      break;
    default:
      goto xit;
		}
	}
xit:
	dfs.printf("Leave ParseGlobalDecl");
	;
}

void AutoDeclaration::Parse(SYM *parent, TABLE *ssyms)
{
	SYM *sp;

//	printf("Enter ParseAutoDecls\r\n");
	funcdecl = 0;
    for(;;) {
		worstAlignment = 0;
		switch(lastst) {
		case kw_cdecl:
    case kw_kernel:
		case kw_interrupt:
		case kw_naked:
		case kw_nocall:
		case kw_oscall:
		case kw_pascal:
		case kw_typedef:
                error(ERR_ILLCLASS);
	            lc_auto += declare(parent,ssyms,sc_auto,lc_auto,bt_struct);
				break;
		case ellipsis:
		case id: //return;
        dfs.printf("Found %s\n", lastid);
				sp = tagtable.Find(lastid,false);
				if (sp)
				   dfs.printf("Found in tagtable");
				if (sp==nullptr)
					sp = gsyms[0].Find(lastid,false);
				if (sp) {
				  dfs.printf("sp okay sc=%d\n", sp->storage_class);
					if (sp->storage_class==sc_typedef || sp->storage_class==sc_type) {
					  dfs.printf("Declaring var of type\n");
			            lc_auto += declare(parent,ssyms,sc_auto,lc_auto,bt_struct);
						break;
					}
				}
				goto xit;
        case kw_register:
                NextToken();
        case kw_exception:
		case kw_volatile: case kw_const:
		case kw_int8: case kw_int16: case kw_int32: case kw_int64:
		case kw_byte: case kw_char: case kw_int: case kw_short: case kw_unsigned: case kw_signed:
        case kw_long: case kw_struct: case kw_union: case kw_class:
        case kw_enum: case kw_void:
        case kw_float: case kw_double:
            lc_auto += declare(parent,ssyms,sc_auto,lc_auto,bt_struct);
            break;
        case kw_thread:
                NextToken();
				lc_thread += declare(parent,ssyms,sc_thread,lc_thread,bt_struct);
				break;
        case kw_static:
                NextToken();
				lc_static += declare(parent,ssyms,sc_static,lc_static,bt_struct);
				break;
        case kw_extern:
                NextToken();
				if (lastst==kw_oscall || lastst==kw_interrupt || lastst == kw_nocall || lastst==kw_naked || lastst==kw_kernel)
					NextToken();
                ++global_flag;
                declare(NULL,&gsyms[0],sc_external,0,bt_struct);
                --global_flag;
                break;
        default:
                goto xit;
		}
	}
xit:
	;
//	printf("Leave ParseAutoDecls\r\n");
}

int ParameterDeclaration::Parse(int fd)
{
	int ofd;

	nparms = 0;
	dfs.printf("Enter ParseParmDecls");
	worstAlignment = 0;
	ofd = funcdecl;
	funcdecl = fd;
	missingArgumentName = FALSE;
	parsingParameterList++;
    for(;;) {
dfs.printf("A(%d)",lastst);
		switch(lastst) {
		case kw_cdecl:
    case kw_kernel:
		case kw_interrupt:
		case kw_naked:
		case kw_nocall:
		case kw_oscall:
		case kw_pascal:
		case kw_typedef:
dfs.printf("B");
      error(ERR_ILLCLASS);
      declare(NULL,&currentFn->params,sc_auto,0,bt_struct);
			break;
		case ellipsis:
		case id:
		case kw_volatile: case kw_const:
        case kw_exception:
		case kw_int8: case kw_int16: case kw_int32: case kw_int64:
		case kw_byte: case kw_char: case kw_int: case kw_short: case kw_unsigned: case kw_signed:
    case kw_long: case kw_struct: case kw_union: case kw_class:
    case kw_enum: case kw_void:
    case kw_float: case kw_double:
dfs.printf("C");
    declare(NULL,&currentFn->params,sc_auto,0,bt_struct);
	            break;
        case kw_thread:
                NextToken();
                error(ERR_ILLCLASS);
				lc_thread += declare(NULL,&gsyms[0],sc_thread,lc_thread,bt_struct);
				break;
        case kw_static:
                NextToken();
                error(ERR_ILLCLASS);
				lc_static += declare(NULL,&gsyms[0],sc_static,lc_static,bt_struct);
				break;
        case kw_extern:
dfs.printf("D");
                NextToken();
                error(ERR_ILLCLASS);
				if (lastst==kw_oscall || lastst==kw_interrupt || lastst == kw_nocall || lastst==kw_naked || lastst==kw_kernel)
					NextToken();
                ++global_flag;
                declare(NULL,&gsyms[0],sc_external,0,bt_struct);
                --global_flag;
                break;
		case kw_register:
				NextToken();
				break;
        default:
				goto xit;
		}
dfs.printf("E");
	}
xit:
	parsingParameterList--;
	funcdecl = ofd;
	dfs.printf("Leave ParseParmDecls");
	return nparms;
}

GlobalDeclaration *GlobalDeclaration::Make()
{
  GlobalDeclaration *p = (GlobalDeclaration *)allocx(sizeof(GlobalDeclaration));
  return p;
}

void compile()
{
}
