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

extern void initFPRegStack();
extern void ReleaseTempFPRegister(AMODE *);
/*
int tmpregs[] = {3,4,5,6,7,8,9,10};
int regstack[8];
int rsp=7;
int regmask=0;

int tmpbregs[] = {3,4,5,6,7,8};
int bregstack[6];
int brsp=5;
int bregmask = 0;
*/
static unsigned short int next_reg;
static unsigned short int next_vreg;
static unsigned short int next_vmreg;
static short int next_breg;
#define MAX_REG 4			/* max. scratch data	register (D2) */
#define	MAX_REG_STACK	30

// Only registers 5,6,7 and 8 are used for temporaries
static short int reg_in_use[256];	// 0 to 15
static short int breg_in_use[16];	// 0 to 15
static short int save_reg_in_use[256];
static short int vreg_in_use[256];	// 0 to 15
static short int save_vreg_in_use[256];
static short int vmreg_in_use[256];	// 0 to 15

static struct {
    enum e_am mode;
    short int reg;
	union {
		char isPushed;	/* flags if pushed or corresponding reg_alloc * number */
		char allocnum;
	} f;
} 
	reg_stack[MAX_REG_STACK + 1],
	reg_alloc[MAX_REG_STACK + 1],
	save_reg_alloc[MAX_REG_STACK + 1],
	stacked_regs[MAX_REG_STACK + 1],
	breg_stack[MAX_REG_STACK + 1],
	breg_alloc[MAX_REG_STACK + 1],
	vreg_stack[MAX_REG_STACK + 1],
	vreg_alloc[MAX_REG_STACK + 1],
	vmreg_stack[MAX_REG_STACK + 1],
	vmreg_alloc[MAX_REG_STACK + 1],
	save_vreg_alloc[MAX_REG_STACK + 1],
	save_vmreg_alloc[MAX_REG_STACK + 1],
	stacked_vregs[MAX_REG_STACK + 1],
	stacked_vmregs[MAX_REG_STACK + 1]
;

static short int reg_stack_ptr;
static short int reg_alloc_ptr;
static short int save_reg_alloc_ptr;
static short int vreg_stack_ptr;
static short int vreg_alloc_ptr;
static short int save_vreg_alloc_ptr;
static short int vmreg_stack_ptr;
static short int vmreg_alloc_ptr;
static short int save_vmreg_alloc_ptr;
static short int breg_stack_ptr;
static short int breg_alloc_ptr;

char tmpregs[] = {1,2,3,4,5,6,7,8,9,10};
char tmpvregs[] = {1,2,3,4,5,6,7,8,9,10};
char tmpvmregs[] = {1,2,3};
char tmpbregs[] = {5,6,7};
char regstack[18];
char bregstack[18];
int rsp=17;
int regmask=0;
int brsp=17;
int bregmask=0;

int next_areg, next_dreg;
int unused_areg[1024], unused_dreg[1024];
int free_areg, free_dreg;

void initRegStack()
{
	int i;
	SYM *sym = currentFn;

	next_areg = 0;
	free_areg = 0;
	next_dreg = 0;
	free_dreg = 0;
	next_reg = sym->IsLeaf ? 1 : 3;
	next_vreg = sym->IsLeaf ? 1 : 3;
	next_vmreg = 1;
    next_breg = 5;
	//for (rsp=0; rsp < 3; rsp=rsp+1)
	//	regstack[rsp] = tmpregs[rsp];
	//rsp = 0;
	for (i = 0; i <= 255; i++) {
		reg_in_use[i] = -1;
		vreg_in_use[i] = -1;
		vmreg_in_use[i] = -1;
		breg_in_use[i&15] = -1;
	}
    reg_stack_ptr = 0;
    reg_alloc_ptr = 0;
    vreg_stack_ptr = 0;
    vreg_alloc_ptr = 0;
    vmreg_stack_ptr = 0;
    vmreg_alloc_ptr = 0;
    breg_stack_ptr = 0;
    breg_alloc_ptr = 0;
//    act_scratch = 0;
    memset(reg_stack,0,sizeof(reg_stack));
    memset(reg_alloc,0,sizeof(reg_alloc));
    memset(vreg_stack,0,sizeof(vreg_stack));
    memset(vreg_alloc,0,sizeof(vreg_alloc));
    memset(vmreg_stack,0,sizeof(vmreg_stack));
    memset(vmreg_alloc,0,sizeof(vmreg_alloc));
    memset(breg_stack,0,sizeof(breg_stack));
    memset(breg_alloc,0,sizeof(breg_alloc));
    memset(stacked_regs,0,sizeof(stacked_regs));
    memset(save_reg_alloc,0,sizeof(save_reg_alloc));
    memset(stacked_vregs,0,sizeof(stacked_vregs));
    memset(save_vreg_alloc,0,sizeof(save_vreg_alloc));
    memset(save_vmreg_alloc,0,sizeof(save_vmreg_alloc));
}

void GenerateTempRegPush(int reg, int rmode, int number, int stkpos)
{
	AMODE *ap1;
    ap1 = allocAmode();
    ap1->preg = reg;
    ap1->mode = rmode;

	GenerateDiadic(op_move,'l',ap1,make_adec(7));
	TRACE(printf("pushing r%d\r\n", reg);)
    reg_stack[reg_stack_ptr].mode = (enum e_am)rmode;
    reg_stack[reg_stack_ptr].reg = reg;
    reg_stack[reg_stack_ptr].f.allocnum = number;
    if (reg_alloc[number].f.isPushed=='T')
		fatal("GenerateTempRegPush(): register already pushed");
    reg_alloc[number].f.isPushed = 'T';
	if (++reg_stack_ptr > MAX_REG_STACK)
		fatal("GenerateTempRegPush(): register stack overflow");
}

void GenerateTempRegPop(int reg, int rmode, int number, int stkpos)
{
	AMODE *ap1;
 
    if (reg_stack_ptr-- == -1)
		fatal("GenerateTempRegPop(): register stack underflow");
    /* check if the desired register really is on stack */
    if (reg_stack[reg_stack_ptr].f.allocnum != number)
		fatal("GenerateTempRegPop()/2");
	if (reg_in_use[reg] >= 0)
		fatal("GenerateTempRegPop():register still in use");
	TRACE(printf("popped r%d\r\n", reg);)
	reg_in_use[reg] = number;
	ap1 = allocAmode();
	ap1->preg = reg;
	ap1->mode = rmode;
	GenerateDiadic(op_move,'l',make_ainc(7),ap1);
    reg_alloc[number].f.isPushed = 'F';
}

void initstack()
{
	initRegStack();
	//initFPRegStack();
}

AMODE *GetTempRegister()
{
	AMODE *ap;
    SYM *sym = currentFn;

	if (reg_in_use[next_reg] >= 0) {
//		if (isThor)	
//			GenerateTriadic(op_addui,0,makereg(regSP),makereg(regSP),make_immed(-8));
		GenerateTempRegPush(next_reg, am_dreg, reg_in_use[next_reg],0);
	}
	TRACE(printf("GetTempRegister:r%d\r\n", next_reg);)
    reg_in_use[next_reg] = reg_alloc_ptr;
    ap = allocAmode();
    ap->mode = am_dreg;
    ap->preg = next_reg;
    ap->deep = reg_alloc_ptr;
    reg_alloc[reg_alloc_ptr].reg = next_reg;
    reg_alloc[reg_alloc_ptr].mode = am_dreg;
    reg_alloc[reg_alloc_ptr].f.isPushed = 'F';
    if (is816) {
        if (next_reg++ >= 10*4+128)
    		next_reg = 3*4+128;		/* wrap around */
    }
    else {
        if (next_reg++ >= 10)
			next_reg = sym->IsLeaf ? 1 : 3;		/* wrap around */
    }
    if (reg_alloc_ptr++ == MAX_REG_STACK)
		fatal("GetTempRegister(): register stack overflow");
	return ap;
}

AMODE *GetTempAddressReg()
{
	AMODE *ap;
    SYM *sym = currentFn;

	TRACE(printf("GetTempAddressRegister:r%d\r\n", next_reg);)
    ap = allocAmode();
    ap->mode = am_areg;
	if (free_areg > 0) {
		ap->preg = unused_areg[free_areg];
		free_areg--;
	}
	else {
		ap->preg = next_areg;
		next_areg++;
	}
	return (ap);
}

AMODE *GetTempDataReg()
{
	AMODE *ap;
    SYM *sym = currentFn;

	TRACE(printf("GetTempAddressRegister:r%d\r\n", next_reg);)
    ap = allocAmode();
    ap->mode = am_dreg;
	if (free_dreg > 0) {
		ap->preg = unused_dreg[free_dreg];
		free_dreg--;
	}
	else {
		ap->preg = next_dreg;
		next_dreg++;
	}
	return (ap);
}

AMODE *GetTempFPRegister()
{
	AMODE *ap;
    SYM *sym = currentFn;
    
	if (reg_in_use[next_reg] >= 0) {
//		if (isThor)	
//			GenerateTriadic(op_addui,0,makereg(regSP),makereg(regSP),make_immed(-8));
		GenerateTempRegPush(next_reg, am_fpreg, reg_in_use[next_reg],0);
	}
	TRACE(printf("GetTempRegister:r%d\r\n", next_reg);)
    reg_in_use[next_reg] = reg_alloc_ptr;
    ap = allocAmode();
    ap->mode = am_fpreg;
    ap->preg = next_reg;
    ap->deep = reg_alloc_ptr;
	ap->type = stddouble.GetIndex();
    reg_alloc[reg_alloc_ptr].reg = next_reg;
    reg_alloc[reg_alloc_ptr].mode = am_fpreg;
    reg_alloc[reg_alloc_ptr].f.isPushed = 'F';
    if (next_reg++ >= 10)
    	next_reg = sym->IsLeaf ? 1 : 3;		/* wrap around */
    if (reg_alloc_ptr++ == MAX_REG_STACK)
		fatal("GetTempRegister(): register stack overflow");
	return ap;
}
//void RestoreTempRegs(int rgmask)
//{
//	int nn;
//	int rm;
//	int i;
//
//	nn = 0;
// 
//	for(nn = rgmask; nn > 0; nn--)
//		GenerateTempRegPop(0,0,0);
//	//if (rgmask != 0) {
//	//	for (nn = 1, rm = rgmask; nn <= 15; nn = nn + 1)
//	//		if ((rm>>nn) & 1) {
//	//			GenerateMonadic(op_pop,0,makereg(nn));
//	//			reg_in_use[nn] = 0;
//	//		}
//	//}
//}

//void RestoreTempBrRegs(int brgmask)
//{
//	int nn;
//	int rm;
//	int i;
//
//	nn = 0;
// 
//	for(nn = brgmask; nn > 0; nn--)
//		GenerateTempBrRegPop(0,0,0);
//}

/*
 * this routines checks if all allocated registers were freed
 */
void checkstack()
{
    int i;
    SYM *sym = currentFn;

    for (i=1; i<= 10; i++)
        if (reg_in_use[i] != -1)
            fatal("checkstack()/1");
	if (next_reg != sym->IsLeaf ? 1 : 3) {
		//printf("Nextreg: %d\r\n", next_reg);
        fatal("checkstack()/3");
	}
    if (reg_stack_ptr != 0)
        fatal("checkstack()/5");
    if (reg_alloc_ptr != 0)
        fatal("checkstack()/6");
}

void checkbrstack()
{
    int i;
    for (i=5; i<= 8; i++)
        if (breg_in_use[i] != -1)
            fatal("checkbstack()/1");
	if (next_breg != 5) {
		//printf("Nextreg: %d\r\n", next_breg);
        fatal("checkbstack()/3");
	}
    if (breg_stack_ptr != 0)
        fatal("checkbstack()/5");
    if (breg_alloc_ptr != 0)
        fatal("checkbstack()/6");
}

/*
 * validate will make sure that if a register within an address mode has been
 * pushed onto the stack that it is popped back at this time.
 */
void validate(AMODE *ap)
{
    SYM *sym = currentFn;
	unsigned int frg = sym->IsLeaf ? 1 : 3;

	if (ap->type!=stdvector.GetIndex())
    switch (ap->mode) {
	case am_dreg:
		if ((ap->preg >= frg && ap->preg <= 10) && reg_alloc[ap->deep].f.isPushed == 'T' ) {
			GenerateTempRegPop(ap->preg, am_dreg, (int) ap->deep, 0);
//			if (isThor)
//				GenerateTriadic(op_addui,0,makereg(regSP),makereg(regSP),make_immed(8));
		}
		break;
	case am_areg:
		if ((ap->preg >= frg && ap->preg <= 10) && reg_alloc[ap->deep].f.isPushed == 'T' ) {
			GenerateTempRegPop(ap->preg, am_areg, (int) ap->deep, 0);
//			if (isThor)
//				GenerateTriadic(op_addui,0,makereg(regSP),makereg(regSP),make_immed(8));
		}
		break;
    case am_indx2:
		if ((ap->preg >= frg && ap->preg <= 10) && reg_alloc[ap->deep].f.isPushed == 'T') {
			GenerateTempRegPop(ap->preg, am_areg, (int) ap->deep, 0);
//			if (isThor)
//				GenerateTriadic(op_addui,0,makereg(regSP),makereg(regSP),make_immed(8));
		}
		if ((ap->sreg >= frg && ap->sreg <= 10) && reg_alloc[ap->deep2].f.isPushed  == 'T') {
			GenerateTempRegPop(ap->sreg, am_areg, (int) ap->deep2, 0);
//			if (isThor)
//				GenerateTriadic(op_addui,0,makereg(regSP),makereg(regSP),make_immed(8));
		}
		break;
    case am_indx3:
		if ((ap->sreg >= frg && ap->sreg <= 10) && reg_alloc[ap->deep2].f.isPushed == 'T') {
			GenerateTempRegPop(ap->sreg, am_areg, (int) ap->deep2, 0);
//			if (isThor)
//				GenerateTriadic(op_addui,0,makereg(regSP),makereg(regSP),make_immed(8));
		}
		goto common;
    case am_ind:
    case am_indx:
    case am_ainc:
    case am_adec:
common:
		if ((ap->preg >= frg && ap->preg <= 10) && reg_alloc[ap->deep].f.isPushed == 'T') {
			GenerateTempRegPop(ap->preg, am_areg, (int) ap->deep, 0);
//			if (isThor)
//				GenerateTriadic(op_addui,0,makereg(regSP),makereg(regSP),make_immed(8));
		}
		break;
    }
}


/*
 * release any temporary registers used in an addressing mode.
 */
void ReleaseTempRegister(AMODE *ap)
{
    int number;
    SYM *sym = currentFn;
	unsigned int frg = sym->IsLeaf ? 1 : 3;

	TRACE(printf("ReleaseTempRegister:r%d r%d\r\n", ap->preg, ap->sreg);)

	if (ap==NULL) {
		printf("DIAG - NULL pointer in ReleaseTempRegister\r\n");
		return;
	}

	validate(ap);
	if (ap->type==stdvector.GetIndex()) {
		switch (ap->mode) {
		case am_ind:
		case am_indx:
		case am_ainc:
		case am_adec:
		case am_dreg:
		case am_areg:
	commonv:
			if (ap->preg >= frg && ap->preg <= 10) {
				if (vreg_in_use[ap->preg]==-1)
					return;
				if (next_vreg-- <= frg)
					next_vreg = 10;
				number = vreg_in_use[ap->preg];
				vreg_in_use[ap->preg] = -1;
				break;
			}
			return;
		case am_indx2:
		case am_indx3:
			if (ap->sreg >= frg && ap->sreg <= 10) {
				if (vreg_in_use[ap->sreg]==-1)
					return;
				if (next_vreg-- <= frg)
					next_vreg = 10;
				number = vreg_in_use[ap->sreg];
				vreg_in_use[ap->sreg] = -1;
				//break;
			}
			goto commonv;
		default:
			return;
		}
		if (vreg_alloc_ptr-- == 0)
			fatal("ReleaseTempRegister(): no registers are allocated");
	  //  if (reg_alloc_ptr != number)
			//fatal("ReleaseTempRegister()/3");
		if (vreg_alloc[number].f.isPushed=='T')
			fatal("ReleaseTempRegister(): register on stack");
		return;
	}
	else
    switch (ap->mode) {
	case am_ind:
	case am_indx:
	case am_ainc:
	case am_adec:
	case am_areg:
	case am_dreg:
common:
		if (ap->preg >= frg && ap->preg <= 10) {
			if (reg_in_use[ap->preg]==-1)
				return;
			if (next_reg-- <= frg)
				next_reg = 10;
			number = reg_in_use[ap->preg];
			reg_in_use[ap->preg] = -1;
			break;
		}
		return;
    case am_indx2:
	case am_indx3:
		if (ap->sreg >= frg && ap->sreg <= 10) {
			if (reg_in_use[ap->sreg]==-1)
				return;
			if (next_reg-- <= frg)
				next_reg = 10;
			number = reg_in_use[ap->sreg];
			reg_in_use[ap->sreg] = -1;
			//break;
		}
		goto common;
    default:
		return;
    }
 //   /* some consistency checks */
	//if (number != ap->deep) {
	//	printf("number %d ap->deep %d\r\n", number, ap->deep);
	//	//fatal("ReleaseTempRegister()/1");
	//}
	if (reg_alloc_ptr-- == 0)
		fatal("ReleaseTempRegister(): no registers are allocated");
  //  if (reg_alloc_ptr != number)
		//fatal("ReleaseTempRegister()/3");
    if (reg_alloc[number].f.isPushed=='T')
		fatal("ReleaseTempRegister(): register on stack");
}

void ReleaseTempVectorMaskRegister()
{
}

void ReleaseTempVectorRegister()
{
}

// The following is used to save temporary registers across function calls.
// Save the list of allocated registers and registers in use.
// Go through the allocated register list and generate a push instruction to
// put the register on the stack if it isn't already on the stack.

int TempInvalidate()
{
    int i;
	int sp;

	sp = 0;
	TRACE(printf("TempInvalidate()\r\n");)
	save_reg_alloc_ptr = reg_alloc_ptr;
	memcpy(save_reg_alloc, reg_alloc, sizeof(save_reg_alloc));
	memcpy(save_reg_in_use, reg_in_use, sizeof(save_reg_in_use));
	for (sp = i = 0; i < reg_alloc_ptr; i++) {
        if (reg_in_use[reg_alloc[i].reg] != -1) {
    		if (reg_alloc[i].f.isPushed == 'F') {
    			GenerateTempRegPush(reg_alloc[i].reg, reg_alloc[i].mode, i, sp);
    			stacked_regs[sp].reg = reg_alloc[i].reg;
    			stacked_regs[sp].mode = reg_alloc[i].mode;
    			stacked_regs[sp].f.allocnum = i;
    			sp++;
    			// mark the register void
    			reg_in_use[reg_alloc[i].reg] = -1;
    		}
        }
	}
	return sp;
}

// Pop back any temporary registers that were pushed before the function call.
// Restore the allocated and in use register lists.

void TempRevalidate(int sp)
{
	int nn;

	for (nn = sp-1; nn >= 0; nn--)
		GenerateTempRegPop(stacked_regs[nn].reg, stacked_regs[nn].mode, stacked_regs[nn].f.allocnum,sp-nn-1);
	reg_alloc_ptr = save_reg_alloc_ptr;
	memcpy(reg_alloc, save_reg_alloc, sizeof(reg_alloc));
	memcpy(reg_in_use, save_reg_in_use, sizeof(reg_in_use));
}

/*
void initRegStack()
{
	for (rsp=0; rsp < 8; rsp=rsp+1)
		regstack[rsp] = tmpregs[rsp];
	for (brsp = 0; brsp < 6; brsp++)
		bregstack[brsp] = tmpbregs[brsp];
	rsp = 0;
	brsp = 0;
}

void ReleaseTempRegister(AMODE *ap)
{
	if (ap==NULL) {
		printf("DIAG - NULL pointer in ReleaseTempRegister\r\n");
		return;
	}
	if( ap->mode == am_immed || ap->mode == am_direct )
        return;         // no registers used
	if (ap->mode == am_breg || ap->mode==am_brind) {
		if (ap->preg < 9 && ap->preg >= 3)
			PushOnBrstk(ap->preg);
		return;
	}
	if(ap->preg < 11 && ap->preg >= 3)
		PushOnRstk(ap->preg);
}
*/
AMODE *GetTempReg(int type)
{
	if (type==stddouble.GetIndex())
		return (GetTempFPRegister());
	else
		return (GetTempRegister());
}

void ReleaseTempFPRegister(AMODE *ap)
{
     ReleaseTempRegister(ap);
}

void ReleaseTempReg(AMODE *ap)
{
	if (ap==nullptr)
		return;
	if (ap->mode==am_dreg) {
		unused_dreg[free_dreg] = ap->preg;
		free_dreg++;
		return;
	}
	if (ap->mode==am_areg) {
		unused_areg[free_areg] = ap->preg;
		free_areg++;
		return;
	}
	if (ap->type==stdvectormask->GetIndex())
		ReleaseTempVectorMaskRegister();
	else if (ap->type==stdvector.GetIndex())
		ReleaseTempVectorRegister();
	else if (ap->type==stddouble.GetIndex())
		ReleaseTempFPRegister(ap);
	else
		ReleaseTempRegister(ap);
}

