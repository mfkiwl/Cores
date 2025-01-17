#include "types.h"
#include "proto.h"
#include <ft64/io.h>

not using gp;

extern __leaf int *memsetO(int *, int, int);
extern int *memsetW(int *, int, int);
extern rmemsetW(register int *, register int, register int);

// The text screen memory can only handle word transfers, hence the use
// of memsetW, memcpyW.
//#define DBGScreen	(__int32 *)0xFFD00000
#define DBGScreen	(int *)0xFFFFFFFFFFD00000
#define DBGCOLS		64
#define DBGROWS		33

extern int IOFocusNdx;
extern __int8 DBGCursorCol;
extern __int8 DBGCursorRow;
extern int DBGAttr;
extern void puthexnum(int num, int wid, int ul, char padchar);
extern __int8 tabstops[32];

void DBGClearScreen()
{
	int *p;
	int vc;

	__asm {
		ldi	r1,#$26
		stb	r1,LEDS
	}     
	p = DBGScreen;
	//vc = AsciiToScreen(' ') | DBGAttr;
	vc = ' ' | DBGAttr;
	memsetO(p, vc, DBGROWS*DBGCOLS); //2604);
	__asm {
		ldi	r1,#$27
		stb	r1,LEDS
	}     
}

naked inline void DBGSetVideoReg(register int regno, register int val)
{
   __asm {
     shl	$a0,$a0,#3
     stt	$a1,$FFFFFFFFFFDCC000[$a0]
   }
}

naked inline void DBGSetCursorPos(register int pos)
{
	__asm {
		stt		$a0,$FFFFFFFFFFD17F1C
	}
}

void DBGSetCursorRC(register int r, register int c)
{
  int pos;
  
  DBGCursorRow = r;
  DBGCursorCol = c;
	pos = __mulf(r, DBGCOLS) + c;
  DBGSetCursorPos(pos);
}

void DBGUpdateCursorPos()
{
	int pos;

	pos = __mulf(DBGCursorRow, DBGCOLS) + DBGCursorCol;
  DBGSetCursorPos(pos);
}

void DBGHomeCursor()
{
	DBGCursorCol = 0;
	DBGCursorRow = 0;
	DBGUpdateCursorPos();
}

pascal void DBGBlankLine(int row)
{
	int *p;
	int nn;
	int mx;
	int vc;

	p = DBGScreen;
	p = p + __mulf(row, DBGCOLS);
	vc = DBGAttr | ' ';
	memsetO(p, vc, DBGCOLS);
}

void DBGScrollUp()
{
	int *scrn = DBGScreen;
	int *scrn2 = DBGScreen + DBGCOLS;
	int nn;
	int count;

	count = __mulf(DBGROWS, DBGCOLS);
	for (nn = 0; nn < count; nn++)
		scrn[nn] = scrn2[nn];

	DBGBlankLine(DBGROWS-1);
}

void DBGIncrementCursorRow()
{
	if (DBGCursorRow < DBGROWS - 1) {
		DBGCursorRow++;
		DBGUpdateCursorPos();
		return;
	}
	DBGScrollUp();
	DBGCursorRow--;
}

void DBGIncrementCursorPos()
{
	if (DBGCursorCol < DBGCOLS) {
		DBGCursorCol++;
		DBGUpdateCursorPos();
		return;
	}
	DBGCursorCol = 0;
	DBGIncrementCursorRow();
}

pascal void DBGDisplayChar(char ch)
{
	int *p;
	int nn;

	switch(ch) {
	case '\r':
		DBGCursorCol = 0;
		DBGUpdateCursorPos();
		break;
	case '\n':
		DBGCursorCol = 0;
		DBGIncrementCursorRow();
		break;
	case 0x91:
    if (DBGCursorCol < DBGCOLS - 1) {
			DBGCursorCol++;
			DBGUpdateCursorPos();
    }
    break;
	case 0x90:
    if (DBGCursorRow > 0) {
			DBGCursorRow--;
			DBGUpdateCursorPos();
    }
    break;
	case 0x93:
    if (DBGCursorCol > 0) {
			DBGCursorCol--;
			DBGUpdateCursorPos();
    }
    break;
	case 0x92:
    if (DBGCursorRow < DBGROWS-1) {
			DBGCursorRow++;
			DBGUpdateCursorPos();
    }
    break;
	case 0x94:
    if (DBGCursorCol==0)
			DBGCursorRow = 0;
    DBGCursorCol = 0;
    DBGUpdateCursorPos();
    break;
	case 0x99:  // delete
    p = DBGScreen + __mulf(DBGCursorRow, DBGCOLS);
    for (nn = DBGCursorCol; nn < DBGCOLS-1; nn++) {
      p[nn] = p[nn+1];
    }
		p[nn] = DBGAttr | ' ';
    break;
	case 0x08: // backspace
    if (DBGCursorCol > 0) {
      DBGCursorCol--;
//	      p = DBGScreen;
  		p = DBGScreen + __mulf(DBGCursorRow, DBGCOLS);
      for (nn = DBGCursorCol; nn < DBGCOLS-1; nn++) {
        p[nn] = p[nn+1];
      }
      p[nn] = DBGAttr | ' ';
		}
    break;
	case 0x0C:   // CTRL-L
    DBGClearScreen();
    DBGHomeCursor();
    break;
	case '\t':
		for (nn = 0; nn < 32; nn++) {
			if (DBGCursorCol < tabstops[nn]) {
				DBGCursorCol = tabstops[nn];
				break;
			}
		}
    break;
	default:
	  p = DBGScreen;
	  nn = __mulf(DBGCursorRow, DBGCOLS) + DBGCursorCol;
	  //p[nn] = ch | DBGAttr;
	  p[nn] = ch | DBGAttr;
	  DBGIncrementCursorPos();
    break;
	}
}

void DBGCRLF()
{
	DBGDisplayChar('\r');
	DBGDisplayChar('\n');
}

pascal void DBGDisplayString(char *s)
{
	// Declaring ch here causes the compiler to generate shorter faster code
	// because it doesn't have to process another *s inside in the loop.
	char ch;
  while (ch = *s) { DBGDisplayChar(ch); s++; }
}

pascal void DBGDisplayAsciiString(unsigned __int8 *s)
{
	// Declaring ch here causes the compiler to generate shorter faster code
	// because it doesn't have to process another *s inside in the loop.
	unsigned __int8 ch;
  while (ch = *s) { DBGDisplayChar(ch); s++; }
}

pascal void DBGDisplayStringCRLF(char *s)
{
   DBGDisplayString(s);
   DBGCRLF();
}

pascal void DBGDisplayAsciiStringCRLF(unsigned __int8 *s)
{
   DBGDisplayAsciiString(s);
   DBGCRLF();
}

pascal void DBGHideCursor(int hide)
{
	if (hide) {
		__asm {
			ldi		$a0,#$FFFF
			stw		$a0,$FFFFFFFFFFD17F18
			;memdb
		}
	}
	else {
		__asm {
			ldi		$a0,#$00E7
			stw		$a0,$FFFFFFFFFFD17F18
			;memdb
		}
	}
}
