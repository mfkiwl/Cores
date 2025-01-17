// ============================================================================
//        __
//   \\__/ o\    (C) 2012-2021  Robert Finch, Waterloo
//    \  __ /    All rights reserved.
//     \/_//     robfinch<remove>@finitron.ca
//       ||
//
// CC64 - 'C' derived language compiler
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

void put_mask(int mask);
void align(int n);
void roseg();
bool renamed = false; 
int64_t genst_cumulative;

/*      variable initialization         */

enum e_gt { nogen, bytegen, chargen, halfgen, wordgen, longgen };
//enum e_sg { noseg, codeseg, dataseg, bssseg, idataseg };

int	       gentype = nogen;
int	       curseg = noseg;
int        outcol = 0;
static ENODE *agr;
struct nlit *numeric_tab = nullptr;

// Please keep table in alphabetical order.
// Instruction.cpp has the number of table elements hard-coded in it.
//
Instruction opl[362] =
{   
{ ";", op_remark },
{ ";asm",op_asm,300 },
{ ";empty",op_empty },
{ ";fname", op_fnname },
{ ";string", op_string },
{ "abs", op_abs,2,1,false,am_reg,am_reg,0,0 },
{ "add",op_add,1,1,false,am_reg,am_reg,am_reg|am_imm,0 },
{ "addi",op_addi,1,1,false,am_reg,am_reg,am_imm,0 },
{ "addis",op_addis,1,1,false,am_reg,am_reg,am_imm,0 },
{ "addu", op_addu,1,1 },
{ "and",op_and,1,1,false,am_reg,am_reg,am_reg|am_imm,0 },
{ "andcm",op_andcm,1,1,false,am_reg,am_reg,am_reg,0 },
{ "andi",op_andi,1,1,false,am_reg,am_reg,am_imm,0 },
{ "andis",op_andis,1,1,false,am_reg,am_reg,am_imm,0 },
{ "asl", op_asl,2,1,false,am_reg,am_reg,am_reg|am_ui6,0 },
{ "aslx", op_aslx,2,1,false,am_reg,am_reg,am_reg | am_ui6,0 },
{ "asr",op_asr,2,1,false,am_reg,am_reg,am_reg|am_ui6,0 },
{ "b",op_b,3,0,false,0,0,0,0 },
{ "bal", op_bal,4,2,false,am_reg,0,0,0 },
{ "band", op_band,2,0,false,am_reg,am_reg,0,0 },
{ "base", op_base,1,0,false,am_reg,am_reg,am_reg|am_ui6,0 },
{ "bbc", op_bbc,3,0,false,am_reg,am_ui6,0,0 },
{ "bbs", op_bbs,3,0,false,am_reg,am_ui6,0,0 },
{ "bchk", op_bchk,3,0 },
{ "beq", op_beq,3,0,false,am_reg,am_direct,0,0 },
{ "beqi", op_beqi,3,0,false,am_reg,am_imm,0,0 },
{ "beqz", op_beqz,3,0,false,am_reg,am_direct,0,0 },
{ "bex", op_bex,0,0,false,0,0,0,0 },
{ "bf", op_bf,3,0,false,am_reg,am_direct,0,0 },
{ "bfclr", op_bfclr,2,1,false,am_reg,am_reg|am_ui6,am_reg|am_ui6,0 },
{ "bfext", op_bfext,2,1,false,am_reg },
{ "bfextu", op_bfextu,2,1,false,am_reg, },
{ "bfins", op_bfins,2,1,false,am_reg },
{ "bfset", op_bfset,2,1,false,am_reg,am_reg | am_ui6,am_reg | am_ui6,0 },
{ "bge", op_bge,3,0,false,am_reg,am_direct,0,0 },
{ "bgeu", op_bgeu,3,0,false,am_reg,am_direct,0,0 },
{ "bgt", op_bgt,3,0,false,am_reg,am_direct,0,0 },
{ "bgtu", op_bgtu,3,0,false,am_reg,am_direct,0,0 },
{ "bhi",op_bhi,2,0, false, am_reg, am_direct,0,0 },
{ "bhs",op_bhs,2,0, false, am_reg, am_direct,0,0 },
{ "bit",op_bit,1,1,false,am_creg,am_reg,am_reg | am_imm,0 },
{ "bl", op_bl,4,2,false,am_reg,0,0,0 },
{ "ble", op_ble, 3,0,false,am_reg,am_direct,0,0 },
{ "bleu", op_bleu,3,0,false,am_reg,am_direct,0,0 },
{ "blo",op_blo,2,0,false,am_reg,am_direct,0,0 },
{ "blr", op_blr,1,0,0,0,0,0 },
{ "bls",op_bls,2,0,false,am_reg,am_direct,0,0 },
{ "blt", op_blt,3,0,false,am_reg,am_direct,0,0 },
{ "bltu", op_bltu,3,0,false,am_reg,am_direct,0,0 },
{ "bmi", op_bmi,2,0,false,am_reg,am_direct,0,0 },
{ "bnand", op_bnand,2,0,false,am_reg,am_reg,0,0 },
{ "bne", op_bne,3,0,false,am_reg,am_direct,0,0 },
{ "bnei", op_bnei,3,0,false,am_reg,am_imm,0,0 },
{ "bnez", op_bnez,3,0,false,am_reg,am_direct,0,0 },
{ "bnor", op_bnor,2,0,false,am_reg,am_reg,0,0 },
{ "bor", op_bor,3,0 },
{ "br",op_br,3,0,false,0,0,0,0 },
{ "bra",op_bra,3,0,false,0,0,0,0 },
{ "brk", op_brk,1,0 },
{ "bsr", op_bsr },
{ "bt", op_bt,3,0,false,am_reg,am_direct,0,0 },
{ "bun", op_bun,2,0 },
{ "bytndx", op_bytendx,1,1,false,am_reg,am_reg,am_reg|am_imm,0 },
{ "cache",op_cache,1,0 },
{ "call", op_call,4,1,false,0,0,0,0 },
{ "chk", op_chk,1,0 },
{ "cmovenz", op_cmovenz,1,1,false,am_reg,am_reg,am_reg,am_reg },
{ "cmp",op_cmp,1,1,false,am_reg,am_reg,am_reg|am_imm,0 },
{ "cmplw",op_cmplw,1,1,false,am_reg,am_reg,am_reg,0 },
{ "cmplwi",op_cmplwi,1,1,false,am_reg,am_reg,am_imm,0 },
{ "cmpu",op_cmpu,1,1,false,am_reg,am_reg,am_reg|am_imm,0 },
{ "cmpw",op_cmpw,1,1,false,am_reg,am_reg,am_reg,0 },
{ "cmpwi",op_cmpwi,1,1,false,am_reg,am_reg,am_imm,0 },
{ "com", op_com,2,1,false,am_reg,am_reg,0,0 },
{ "csrrw", op_csrrw,1,1,false,am_reg },
{ "dc",op_dc },
{ "dec", op_dec,4,0,true,am_i5 },
{ "defcat", op_defcat,12,1,false,am_reg,am_reg,0 ,0 },
{ "dep",op_dep,1,1,false,am_reg,am_reg,am_reg | am_imm,am_reg | am_imm },
{ "di", op_di,1,1,false,am_reg | am_ui6,0,0,0 },
{ "div", op_div,68,1,false,am_reg,am_reg,am_reg|am_imm,0 },
{ "divu",op_divu,68,1,false,am_reg,am_reg,am_reg|am_imm,0 },
{ "divw",op_divw,68,1,false,am_reg,am_reg,am_reg | am_imm,0 },
{ "dw", op_dw },
{ "enter",op_enter,10,1,true,am_imm,0,0,0 },
{ "enter_far",op_enter_far,12,1,true,am_imm,0,0,0 },
{ "eor",op_eor,1,1,false,am_reg,am_reg,am_reg|am_imm,0 },
{ "eq",op_eq },
{ "ext", op_ext,1,1,false,am_reg,am_reg,am_reg | am_imm | am_imm0, am_reg | am_imm | am_imm0 },
{ "extr", op_extr,1,1,false,am_reg,am_reg,am_reg | am_imm | am_imm0, am_reg|am_imm | am_imm0 },
{ "extsb",op_extsb,1,1,false,am_reg,am_reg,0,0 },
{ "extsh",op_extsh,1,1,false,am_reg,am_reg,0,0 },
{ "extsw",op_extsw,1,1,false,am_reg,am_reg,0,0 },
{ "extu", op_extu,1,1,false,am_reg,am_reg,am_reg | am_imm | am_imm0, am_reg | am_imm | am_imm0 },
{ "fadd", op_fadd, 6, 1, false, am_reg, am_reg, am_reg, 0 },
{ "fadd.d", op_fdadd,6,1,false,am_reg,am_reg,am_reg,0 },
{ "fadd.s", op_fsadd,6,1,false,am_reg,am_reg,am_reg,0 },
{ "fbeq", op_fbeq,3,0,false,am_reg,am_reg,0,0 },
{ "fbge", op_fbge,3,0,false,am_reg,am_reg,0,0 },
{ "fbgt", op_fbgt,3,0,false,am_reg,am_reg,0,0 },
{ "fble", op_fble,3,0,false,am_reg,am_reg,0,0 },
{ "fblt", op_fblt,3,0,false,am_reg,am_reg,0,0 },
{ "fbne", op_fbne,3,0,false,am_reg,am_reg,0,0 },
{ "fbor", op_fbor,3,0,false,am_reg,am_reg,0,0 },
{ "fbun", op_fbun,3,0,false,am_reg,am_reg,0,0 },
{ "fcmp", op_fcmp, 1,1,false,am_reg,am_reg,am_reg,0 },
{ "fcvtdq", op_fcvtdq,2,1,false,am_reg,am_reg,0,0 },
{ "fcvtsq", op_fcvtsq,2,1,false,am_reg,am_reg,0,0 },
{ "fcvttq", op_fcvttq,2,1,false,am_reg,am_reg,0,0 },
{ "fdiv", op_fdiv, 160, 1, false, am_reg, am_reg, am_reg, 0 },
{ "fdiv.s", op_fsdiv,80,1,false },
{ "fi2d", op_i2d,2,1,false },
{ "fix2flt", op_fix2flt },
{ "fldo", op_fldo, 4, 1, true, am_reg, am_mem, 0, 0 },
{ "flt2fix",op_flt2fix },
{ "fmov", op_fmov,1,1 },
{ "fmov.d", op_fdmov,1,1 },
{ "fmul", op_fdmul,10,1,false,am_reg,am_reg,am_reg,0 },
{ "fmul", op_fmul, 10, 1, false, am_reg, am_reg, am_reg, 0 },
{ "fmul.s", op_fsmul,10,1,false },
{ "fneg", op_fneg,2,1,false,am_reg,am_reg,0,0 },
{ "fs2d", op_fs2d,2,1,false,am_reg,am_reg,0,0 },
{ "fseq", op_fseq, 1, 1, false, am_creg, am_reg, am_reg, 0 },
{ "fsle", op_fsle, 1, 1, false, am_creg, am_reg, am_reg, 0 },
{ "fslt", op_fslt, 1, 1, false, am_creg, am_reg, am_reg, 0 },
{ "fsne", op_fsne, 1, 1, false, am_creg, am_reg, am_reg, 0 },
{ "fsto", op_fsto, 4, 0, true, am_reg, am_mem, 0, 0 },
{ "fsub", op_fdsub,6,1,false,am_reg,am_reg,am_reg,0 },
{ "fsub", op_fsub, 6, 1, false, am_reg, am_reg, am_reg, 0 },
{ "fsub.s", op_fssub,6,1,false },
{ "ftadd", op_ftadd },
{ "ftdiv", op_ftdiv },
{ "ftmul", op_ftmul },
{ "ftoi", op_ftoi, 2, 1, false, am_reg, am_reg, 0, 0 },
{ "ftsub", op_ftsub },
{ "gcsub",op_gcsub,1,1,false,am_reg,am_reg,am_reg | am_imm,0 },
{ "ge",op_ge },
{ "geu", op_geu },
{ "gt",op_gt },
{ "gtu",op_gtu },
{ "hint", op_hint,0 },
{ "hint2",op_hint2,0 },
{ "ibne", op_ibne,3,1 ,false,am_reg,am_reg,0,0 },
{ "inc", op_inc,4,0,true,am_i5,am_mem,0,0 },
{ "iret", op_iret,2,0,false,0,0,0,0 },
{ "isnull", op_isnullptr,1,1,false,am_reg,am_reg,0,0 },
{ "itof", op_itof, 2, 1, false, am_reg, am_reg, 0, 0 },
{ "itop", op_itop, 2, 1, false, am_reg, am_reg, 0, 0 },
{ "jal", op_jal,1,1,false },
{ "jmp",op_jmp,1,0,false,am_mem,0,0,0 },
{ "jsr", op_jsr,1,1,false },
{ "l",op_l,1,1,false,am_reg,am_imm,0,0 },
{ "la",op_la,1,1,false,am_reg,am_mem,0,0 },
{ "lb", op_lb,4,1,true,am_reg,am_mem,0,0 },
{ "lbu", op_lbu,4,1,true,am_reg,am_mem,0,0 },
{ "lbx", op_lbx,4,1,true,am_reg,am_mem,0,0 },
{ "ld", op_ld,4,1,true,am_reg,am_mem,0,0 },
{ "ldb", op_ldb,4,1,true,am_reg,am_mem,0,0 },
{ "ldbu", op_ldbu,4,1,true,am_reg,am_mem,0,0 },
{ "ldd", op_ldd,4,1,true,am_reg,am_mem,0,0 },
{ "lddr", op_lddr,4,1,true,am_reg,am_mem,0,0 },
{ "ldfd", op_ldfd,4,1,true, am_reg, am_mem,0,0 },
{ "ldft", op_ldft,4,1,true, am_reg, am_mem,0,0 },
{ "ldh", op_ldh,4,1,true,am_reg,am_mem,0,0 },
{ "ldi",op_ldi,1,1,false,am_reg,am_imm,0,0 },
{ "ldm", op_ldm,20,1,true,am_mem,0,0,0 },
{ "ldo", op_ldo,4,1,true,am_reg,am_mem,0,0 },
{ "ldou", op_ldou,4,1,true,am_reg,am_mem,0,0 },
{ "ldp", op_ldp,4,1,true,am_reg,am_mem,0,0 },
{ "ldpu", op_ldpu,4,1,true,am_reg,am_mem,0,0 },
{ "ldt", op_ldt,4,1,true,am_reg,am_mem,0,0 },
{ "ldtu", op_ldtu,4,1,true,am_reg,am_mem,0,0 },
{ "ldw", op_ldw,4,1,true,am_reg,am_mem,0,0 },
{ "ldwu", op_ldwu,4,1,true,am_reg,am_mem,0,0 },
{ "ldx", op_ldx,4,1,true,am_reg,am_mem,0,0 },
{ "le",op_le },
{ "lea",op_lea,1,1,false,am_reg,am_mem,0,0 },
{ "leave",op_leave,10,2,true, 0, 0, 0, 0 },
{ "leave_far",op_leave_far,12,2,true, 0, 0, 0, 0 },
{ "leu",op_leu },
{ "lh", op_lh,4,1,true,am_reg,am_mem,0,0 },
{ "lha", op_lha,4,1,true,am_reg,am_mem,0,0 },
{ "lhax", op_lhax,4,1,true,am_reg,am_mem,0,0 },
{ "lhu", op_lhu,4,1,true,am_reg,am_mem,0,0 },
{ "lhzx", op_lhzx,4,1,true,am_reg,am_mem,0,0 },
{ "li",op_li,1,1,false,am_reg,am_imm,0,0 },
{ "link",op_link,4,1,true,am_imm,0,0,0 },
{ "lis",op_lis,1,1,false,am_reg,am_imm,0,0 },
{ "lm", op_lm },
{ "loop", op_loop,1,0 },
{ "lsr", op_lsr,2,1,false,am_reg,am_reg,am_reg|am_ui6,0 },
{ "lt",op_lt },
{ "ltu", op_ltu },
{ "lv", op_lv,256,1 },
{ "lvbu", op_lvbu,4,1,true ,am_reg,am_mem,0,0 },
{ "lvcu", op_lvcu,4,1,true ,am_reg,am_mem,0,0 },
{ "lvhu", op_lvhu,4,1,true ,am_reg,am_mem,0,0 },
{ "lw", op_lw,4,1,true,am_reg,am_mem,0,0 },
{ "lws", op_ldds,4,1,true },
{ "lwu", op_lwu,4,1,true,am_reg,am_mem,0,0 },
{ "lwx", op_lwx,4,1,true,am_reg,am_reg,am_reg,0 },
{ "lwz", op_lwz,4,1,true,am_reg,am_mem,0,0 },
{ "lwzx", op_lwzx,4,1,true,am_reg,am_mem,0,0 },
{ "mfbase", op_mfbase,1,0,false,am_reg,am_reg | am_ui6,0,0 },
{ "mfcr", op_mfcr,1,1,false,am_reg,0,0,0 },
{ "mffp",op_mffp },
{ "mflr", op_mflr,1,1,false,am_reg,0,0,0 },
{ "mod", op_mod,68,1, false,am_reg,am_reg,am_reg|am_imm,0 },
{ "modu", op_modu,68,1,false,am_reg,am_reg,am_reg,0 },
{ "mov", op_mov,1,1,false,am_reg,am_reg,0,0 },
{ "move",op_move,1,1,false,am_reg,am_reg,0,0 },
{ "movs", op_movs },
{ "mr", op_mr,1,1,false,am_reg,am_reg,0,0 },
{ "mtbase", op_mtbase,1,0,false,am_reg,am_reg | am_ui6,0,0 },
{ "mtfp", op_mtfp },
{ "mtlr", op_mtlr,1,0,false,am_reg,0,0,0 },
{ "mul",op_mul,18,1,false,am_reg,am_reg,am_reg|am_imm,0 },
{ "mulf",op_mulf,1,1,false,am_reg,am_reg,am_reg | am_imm,0 },
{ "mullw", op_mullw, 10, 1, false, am_reg, am_reg, am_reg | am_imm, 0 },
{ "mulu", op_mulu, 10, 1, false, am_reg, am_reg, am_reg | am_imm, 0 },
{ "mv", op_mv,1,1,false,am_reg,am_reg,0,0 },
{ "nand",op_nand,1,1,false,am_reg,am_reg,am_reg,0 },
{ "ne",op_ne },
{ "neg",op_neg, 1, 1, false,am_reg,am_reg,0,0 },
{ "nop", op_nop,0,0,false },
{ "nor",op_nor,1,1,false,am_reg,am_reg,am_reg,0 },
{ "not", op_not,2,1,false,am_reg,am_reg,0,0 },
{ "not",op_not,2,1, false,am_reg, am_reg,0,0 },
{ "nr", op_nr },
{ "or",op_or,1,1,false,am_reg,am_reg,am_reg | am_imm,0 },
{ "orcm",op_orcm,1,1,false,am_reg,am_reg,am_reg,0 },
{ "ori",op_ori,1,1,false,am_reg,am_reg,am_imm,0 },
{ "oris",op_oris,1,1,false,am_reg,am_reg,am_imm,0 },
{ "padd", op_padd, 6, 1, false, am_reg, am_reg, am_reg, 0 },
{ "pdiv", op_pdiv, 10, 1, false, am_reg, am_reg, am_reg, 0 },
{ "pea", op_pea },
{ "pea",op_pea },
{ "pfi", op_pfi, 1, 1, false, 0, 0, 0, 0 },
{ "phi", op_phi },
{ "pldo", op_pldo,4,1,true,am_reg,am_mem,0,0 },
{ "pldt", op_pldt,4,1,true,am_reg,am_mem,0,0 },
{ "pldw", op_pldw,4,1,true,am_reg,am_mem,0,0 },
{ "pmul", op_pmul, 8, 1, false, am_reg, am_reg, am_reg, 0 },
{ "pop", op_pop,4,2,true,am_reg,am_reg,0,0 },
{ "popf", op_popf,4,2,true,am_reg,am_reg,0,0 },
{ "psto", op_psto,4,1,true,am_reg,am_mem,0,0 },
{ "pstt", op_pstt,4,1,true,am_reg,am_mem,0,0 },
{ "pstw", op_pstw,4,1,true,am_reg,am_mem,0,0 },
{ "psub", op_psub, 6, 1, false, am_reg, am_reg, am_reg, 0 },
{ "ptrdif",op_ptrdif,1,1,false,am_reg,am_reg,am_reg,am_imm },
{ "push",op_push,4,1,true,am_reg | am_imm,am_reg,0,0 },
{ "pushf",op_pushf,4,0,true,am_reg,0,0,0 },
{ "redor", op_redor,2,1,false,am_reg,am_reg,am_reg,0 },
{ "regs", op_reglist,1,1,false,am_imm,0,0,0 },
{ "rem", op_rem,68,1,false,am_reg,am_reg,am_reg | am_imm,0 },
{ "remu",op_remu,68,1,false,am_reg,am_reg,am_reg | am_imm,0 },
{ "ret", op_ret,1,0,am_imm,0,0,0 },
{ "rol", op_rol,2,1,false,am_reg,am_reg,am_reg | am_ui6,0 },
{ "ror", op_ror,2,1,false,am_reg,am_reg,am_reg | am_ui6,0 },
{ "rtd", op_rtd },
{ "rte", op_rte,2,0 },
{ "rti", op_rti,2,0 },
{ "rtl", op_rtl,1,0,am_imm,0,0,0 },
{ "rts", op_rts,1,0,am_imm,0,0,0 },
{ "rtx", op_rtx,1,0,0,0,0,0 },
{ "sand",op_sand,1,1,false,am_reg,am_reg,am_reg | am_imm,0 },
{ "sb",op_sb,4,0,true,am_reg,am_mem,0,0 },
{ "sd",op_sd,4,0,true,am_reg,am_mem,0,0 },
{ "sei", op_sei,1,0,false,am_reg|am_ui6,0,0,0 },
{ "seq", op_seq,1,1,false,am_reg,am_reg,am_reg | am_i26,0 },
{ "setwb", op_setwb, 1, 0 },
{ "sge",op_sge,1,1,false,am_reg,am_reg,am_reg | am_i26,0 },
{ "sgeu",op_sgeu,1,1,false,am_reg,am_reg,am_reg | am_i26,0 },
{ "sgt",op_sgt,1,1,false,am_reg,am_reg,am_reg | am_i26,0 },
{ "sgtu",op_sgtu,1,1,false,am_reg,am_reg,am_reg | am_i26,0 },
{ "sh",op_sh,4,0,true,am_reg,am_mem,0,0 },
{ "shl", op_stpl,2,1,false,am_reg,am_reg,am_reg | am_ui6,0 },
{ "shlu", op_stplu,2,1,false,am_reg,am_reg,am_reg | am_ui6,0 },
{ "shr", op_stpr,2,1,false,am_reg,am_reg,am_reg | am_ui6,0 },
{ "shru", op_stpru,2,1,false,am_reg,am_reg,am_reg | am_ui6,0 },
{ "sld", op_sld,2,1,false,am_reg,am_reg,am_reg,0 },
{ "sldi", op_sldi,2,1,false,am_reg,am_reg,am_ui6,0 },
{ "sle",op_sle,1,1,false,am_reg,am_reg,am_reg | am_i26,0 },
{ "sleu",op_sleu,1,1,false,am_reg,am_reg,am_reg | am_i26,0 },
{ "sll", op_sll,2,1,false,am_reg,am_reg,am_reg,0 },
{ "slt", op_slt,1,1,false,am_reg,am_reg,am_reg,0 },
{ "sltu", op_sltu,1,1,false,am_reg,am_reg,am_reg,0 },
{ "slw", op_slw,2,1,false,am_reg,am_reg,am_reg,0 },
{ "slwi", op_slwi,2,1,false,am_reg,am_reg,am_ui6,0 },
{ "sm",op_sm },
{ "sne",op_sne,1,1,false,am_reg,am_reg,am_reg | am_i26,0 },
{ "sor",op_sor,1,1,false,am_reg,am_reg,am_reg | am_imm,0 },
{ "spt", op_spt,4,0,true ,am_reg,am_mem,0,0 },
{ "sptr", op_sptr,4,0,true,am_reg,am_mem,0,0 },
{ "sra", op_sra,2,1,false,am_reg,am_reg,am_reg | am_ui6,0 },
{ "srad", op_srad,2,1,false,am_reg,am_reg,am_reg,0 },
{ "sradi", op_sradi,2,1,false,am_reg,am_reg,am_ui6,0 },
{ "sraw", op_sraw,2,1,false,am_reg,am_reg,am_reg,0 },
{ "srawi", op_srawi,2,1,false,am_reg,am_reg,am_ui6,0 },
{ "srl", op_srl,2,1,false,am_reg,am_reg,am_reg | am_ui6,0 },
{ "srwi", op_srwi,2,1,false,am_reg,am_reg,am_ui6,0 },
{ "stb",op_stb,4,0,true,am_reg,am_mem,0,0 },
{ "stbx",op_stbx,4,0,true,am_reg,am_mem,0,0 },
{ "std", op_std,4,0,true,am_reg,am_mem,0,0 },
{ "stdcr", op_stdc,4,0,true, am_reg, am_mem,0,0 },
{ "stdx", op_stdx,4,0,true,am_reg,am_mem,0,0 },
{ "stfd", op_stfd,4,0,true, am_reg, am_mem,0,0 },
{ "stft", op_stft,4,0,true, am_reg, am_mem,0,0 },
{ "sth", op_sth,4,0,true,am_reg,am_mem,0,0 },
{ "sthx", op_sthx,4,0,true,am_reg,am_mem,0,0 },
{ "sti", op_sti,1,0 },
{ "stm", op_stm,20,1,true,am_mem,0,0,0 },
{ "sto",op_sto,4,0,true,am_reg,am_mem,0,0 },
{ "stop", op_stop },
{ "stp",op_stp,4,0,true,am_reg,am_mem,0,0 },
{ "stt",op_stt,4,0,true,am_reg,am_mem,0,0 },
{ "stw",op_stw,4,0,true,am_reg,am_mem,0,0 },
{ "stwx",op_stwx,4,0,true,am_reg,am_mem,0,0 },
{ "sub",op_sub,1,1,false,am_reg,am_reg,am_reg | am_imm,0 },
{ "subf",op_subf,1,1,false,am_reg,am_reg,am_reg | am_imm,0 },
{ "subu", op_subu,1,1 },
{ "sv", op_sv,256,0 },
{ "sw",op_sw,4,0,true,am_reg,am_mem,0,0 },
{ "swap",op_stdap,1,1,false },
{ "swp", op_stdp, 8, false },
{ "sws", op_stds,4,0 },
{ "swx", op_swx,4,1,true,am_reg,am_reg,am_reg,0 },
{ "sxb",op_sxb,1,1,false,am_reg,am_reg,0,0 },
{ "sxc",op_sxc,1,1,false,am_reg,am_reg,0,0 },
{ "sxh",op_sxh,1,1,false,am_reg,am_reg,0,0 },
{ "sxo",op_sxo,1,1,false,am_reg,am_reg,0,0 },
{ "sxp",op_sxp,1,1,false,am_reg,am_reg,0,0 },
{ "sxt",op_sxt,1,1,false,am_reg,am_reg,0,0 },
{ "sxw",op_sxw,1,1,false,am_reg,am_reg,0,0 },
{ "tgt", op_calltgt,1 },
{ "tst",op_tst,1,1 },
{ "unlink",op_unlk,4,2,true },
{ "vadd", op_vadd,10,1,false, am_vreg,am_vreg,am_vreg,0 },
{ "vadds", op_vadds,10 },
{ "vdiv", op_vdiv,100,1,false, am_vreg,am_vreg,am_vreg,0 },
{ "vdivs", op_vdivs,100 },
{ "veins",op_veins,10 },
{ "ver", op_verbatium,0,1,false, 0,0,0,0 },
{ "vex", op_vex,10 },
{ "vmul", op_vmul,10,1,false, am_vreg,am_vreg,am_vreg,0 },
{ "vmuls", op_vmuls,10 },
{ "vseq", op_vseq,10,1,false, am_vreg,am_vreg,am_vreg,0 },
{ "vsge", op_vsge,10,1,false, am_vreg,am_vreg,am_vreg,0 },
{ "vsgt", op_vsgt,10,1,false, am_vreg,am_vreg,am_vreg,0 },
{ "vsle", op_vsle,10,1,false, am_vreg,am_vreg,am_vreg,0 },
{ "vslt", op_vslt,10,1,false, am_vreg,am_vreg,am_vreg,0 },
{ "vsne", op_vsne,10,1,false, am_vreg,am_vreg,am_vreg,0 },
{ "vsub", op_vsub,10,1,false, am_vreg,am_vreg,am_vreg,0 },
{ "vsubs", op_vsubs,10 },
{ "wydndx", op_wydendx,1,1,false,am_reg,am_reg,am_reg | am_imm,0 },
{ "xnor",op_xnor,1,1,false,am_reg,am_reg,am_reg,0 },
{ "xor",op_xor,1,1,false,am_reg,am_reg,am_reg|am_imm,0 },
{ "xori",op_xori,1,1,false,am_reg,am_reg,am_reg | am_imm,0 },
{ "xoris",op_xoris,1,1,false,am_reg,am_reg,am_reg | am_imm,0 },
{ "zxb",op_zxb,1,1,false,am_reg,am_reg,0,0 },
{ "zxt",op_zxt,1,1,false,am_reg,am_reg,0,0 },
{ "zxw",op_zxw,1,1,false,am_reg,am_reg,0,0 }
};

Instruction *GetInsn(int op)
{
	return (Instruction::Get(op));
}

/*
static char *segstr(int op)
{
	static char buf[20];

	switch(op & 0xff00) {
	case op_cs:
		return "cs";
	case op_ss:
		return "ss";
	case op_ds:
		return "ds";
	case op_bs:
		return "bs";
	case op_ts:
		return "ts";
	default:
		sprintf(buf, "seg%d", op >> 8);
		return buf;
	}
}
*/

// Output a friendly register moniker

char *RegMoniker(int regno)
{
	static char buf[4][20];
	static int n;
	int rg;

	n = (n + 1) & 3;
	if (rg = IsTempReg(regno)) {
		sprintf_s(&buf[n][0], 20, "r%d", tmpregs[rg-1]);
	}
	else
		if (regno==regCS)
			sprintf_s(&buf[n][0], 20, "$b15");
		else if (regno == regRS)
			sprintf_s(&buf[n][0], 20, "$b14");
		else if (regno==regFP)
			sprintf_s(&buf[n][0], 20, "r29");
		else if (regno == regAFP)
			sprintf_s(&buf[n][0], 20, "$afp");
		else if (regno==regGP)
		sprintf_s(&buf[n][0], 20, "r27");
		else if (regno == regGP1)
			sprintf_s(&buf[n][0], 20, "r28");
		else if (regno==regXLR)
		sprintf_s(&buf[n][0], 20, "$xlr");
	else if (regno==regPC)
		sprintf_s(&buf[n][0], 20, "$pc");
	else if (regno==regSP)
		sprintf_s(&buf[n][0], 20, "r1");
	else if (regno==regLR)
		sprintf_s(&buf[n][0], 20, "lr");
	else if (regno==114)
		sprintf_s(&buf[n][0], 20, "$cn");
	else if (regno==1)
		sprintf_s(&buf[n][0], 20, "$x%d", regno);
	else if (regno == 2)
			sprintf_s(&buf[n][0], 20, "$x%d", regno);
	else if (regno >= regFirstArg && regno <= regLastArg)
		sprintf_s(&buf[n][0], 20, "r%d", regno);
	else if (regno >= regFirstTemp && regno <= regLastTemp)
		sprintf_s(&buf[n][0], 20, "r%d", regno);
	else if (regno >= regFirstRegvar && regno <= regLastRegvar)
		sprintf_s(&buf[n][0], 20, "r%d", regno);
	else {
		if ((regno & 0x70)==0x020)
			sprintf_s(&buf[n][0], 20, "$f%d", regno & 0x1f);
		else if ((regno & 0x70) == 0x040)
			sprintf_s(&buf[n][0], 20, "$p%d", regno & 0x1f);
		else if ((regno & 0x70) == 0x070)
			sprintf_s(&buf[n][0], 20, "cr%d", regno & 0x3);
		else
			sprintf_s(&buf[n][0], 20, "$x%d", regno);
	}
	return &buf[n][0];
}

char *RegMoniker2(int regno)
{
	static char buf[4][20];
	static int n;
	int rg;

	n = (n + 1) & 3;
	if (rg = IsTempReg(regno)) {
		sprintf_s(&buf[n][0], 20, "r%d", tmpregs[rg - 1]);
	}
	else
		if (regno == regCS)
			sprintf_s(&buf[n][0], 20, "$b15");
		else if (regno == regRS)
			sprintf_s(&buf[n][0], 20, "$b14");
		else if (regno == regFP)
		sprintf_s(&buf[n][0], 20, "r29");
	else if (regno == regAFP)
		sprintf_s(&buf[n][0], 20, "$afp");
	else if (regno == regGP)
		sprintf_s(&buf[n][0], 20, "r27");
	else if (regno == regGP1)
		sprintf_s(&buf[n][0], 20, "r28");
	else if (regno == regXLR)
		sprintf_s(&buf[n][0], 20, "$xlr");
	else if (regno == regPC)
		sprintf_s(&buf[n][0], 20, "$pc");
	else if (regno == regSP)
		sprintf_s(&buf[n][0], 20, "r1");
	else if (regno == regLR)
		sprintf_s(&buf[n][0], 20, "lr");
	else if (regno == 1)
		sprintf_s(&buf[n][0], 20, "$x%d", regno);
	else if (regno == 2)
		sprintf_s(&buf[n][0], 20, "$x%d", regno);
	else if (regno >= regFirstArg && regno <= regLastArg)
		sprintf_s(&buf[n][0], 20, "r%d", regno);
	else if (regno >= regFirstTemp && regno <= regLastTemp)
		sprintf_s(&buf[n][0], 20, "r%d", regno);
	else if (regno >= regFirstRegvar && regno <= regLastRegvar)
		sprintf_s(&buf[n][0], 20, "r%d", regno);
	else
		if ((regno & 0x70) == 0x020)
			sprintf_s(&buf[n][0], 20, "$f%d", regno & 0x1f);
		else if ((regno & 0x70) == 0x040)
			sprintf_s(&buf[n][0], 20, "$p%d", regno & 0x1f);
		else if ((regno & 0x70) == 0x070)
			sprintf_s(&buf[n][0], 20, "cr%d", regno & 0x3);
		else
			sprintf_s(&buf[n][0], 20, "$x%d", regno);
	return &buf[n][0];
}


/*
 *      generate a register mask for restore and save.
 */
void put_mask(int mask)
{
	int nn;
	int first = 1;

	for (nn = 0; nn < 32; nn++) {
		if (mask & (1<<nn)) {
			if (!first)
				ofs.printf("/");
			ofs.printf("r%d",nn);
			first = 0;
		}
	}
//	fprintf(output,"#0x%04x",mask);

}

/*
 *      generate a register name from a tempref number.
 */
void putreg(int r)
{
	ofs.printf("x%d", r);
}

/*
 *      generate a named label.
 */
void gen_strlab(char *s)
{       ofs.printf("%s:\n",s);
}

/*
 *      output a compiler generated label.
 */
char *gen_label(int lab, char *nm, char *ns, char d, int sz, int algn)
{
	static char buf[500];
	static char nam[200];

	if (algn == 0)
		algn = 8;
	sprintf_s(nam, sizeof(nam), "%.400s_%d", ns, lab);
	sprintf_s(buf, sizeof(buf), "\t.type %s,@object\n\t.size %s,%d\n\t.align %d\n%s:\n", nam, nam, sz, algn, nam);
	if (nm == NULL || strlen(nm) == 0)
		sprintf(&buf[strlen(buf)], "\n");
	else
		sprintf(&buf[strlen(buf)], " #%s\n", nm);
	return (buf);
}

char *put_labels(char *buf)
{
	ofs.printf("%s", buf);
	return (buf);
}

char *put_label(int lab, char *nm, char *ns, char d, int sz, int algn)
{
  static char buf[500];

	if (lab < 0) {
		buf[0] = '\0';
		return buf;
	}
	if (algn == 0)
		algn = 8;
	if (d == 'C') {
//		sprintf_s(buf, sizeof(buf), ".C%05d", lab);
		sprintf_s(buf, sizeof(buf), "%s_%05d", currentFn->sym->mangledName->c_str(), lab);
		if (nm == NULL)
			ofs.printf("%s:\n", buf);
		else if (strlen(nm) == 0) {
			ofs.printf("%s:\n", buf);
		}
		else {
			//sprintf_s(buf, sizeof(buf), "%s_%s:\n", nm, ns);
			ofs.printf("%s:	; %s\n", buf, nm);
		}
	}
	else {
		ofs.write("\t.type ");
		sprintf_s(buf, sizeof(buf), "%.400s_%d", ns, lab);
		ofs.printf("%s,@object\n", buf);
		ofs.write("\t.size ");
		ofs.printf("%s,", buf);
		ofs.printf("%d\n", sz);
		ofs.printf("\t.align %d\n", algn);
		ofs.printf("%s:", buf);
		if (nm == NULL || strlen(nm) == 0)
			ofs.printf("\n");
		else
			ofs.printf(" #%s\n", nm);
	}
	return (buf);
}


void GenerateByte(int64_t val)
{
	if( gentype == bytegen && outcol < 60) {
        ofs.printf(",%d",(int)val & 0x00ff);
        outcol += 4;
    }
    else {
        nl();
        ofs.printf("\t.byte\t%d",(int)val & 0x00ff);
        gentype = bytegen;
        outcol = 19;
    }
	genst_cumulative += 1;
}

void GenerateChar(int64_t val)
{
	if( gentype == chargen && outcol < 60) {
        ofs.printf(",%d",(int)val & 0xffff);
        outcol += 6;
    }
    else {
        nl();
        ofs.printf("\t.2byte\t%d",(int)val & 0xffff);
        gentype = chargen;
        outcol = 21;
    }
	genst_cumulative += 2;
}

void GenerateHalf(int64_t val)
{
	if( gentype == halfgen && outcol < 60) {
        ofs.printf(",%ld",(long)(val & 0xffffffffLL));
        outcol += 10;
    }
    else {
        nl();
        ofs.printf("\t.4byte\t%ld",(long)(val & 0xffffffffLL));
        gentype = halfgen;
        outcol = 25;
    }
	genst_cumulative += 4;
}

void GenerateWord(int64_t val)
{
	if( gentype == wordgen && outcol < 58) {
        ofs.printf(",%I64d",val);
        outcol += 18;
    }
    else {
        nl();
        ofs.printf("\t.8byte\t%I64d",val);
        gentype = wordgen;
        outcol = 33;
    }
	genst_cumulative += 8;
}

void GenerateLong(int64_t val)
{ 
	if( gentype == longgen && outcol < 56) {
                ofs.printf(",%I64d",val);
                outcol += 10;
                }
        else    {
                nl();
                ofs.printf("\t.8byte\t%I64d",val);
                gentype = longgen;
                outcol = 25;
                }
		genst_cumulative += 8;
}

void GenerateFloat(Float128 *val)
{ 
	if (val==nullptr)
		return;
	ofs.printf("\r\n\t.align 2\r\n");
	ofs.printf("\t.4byte\t%s",val->ToString(64));
  gentype = longgen;
  outcol = 65;
	genst_cumulative += 8;
}

void GenerateQuad(Float128 *val)
{ 
	if (val==nullptr)
		return;
	ofs.printf("\r\n\t.align 2\r\n");
	ofs.printf("\t.4byte\t%s",val->ToString(128));
  gentype = longgen;
  outcol = 65;
	genst_cumulative += 16;
}

void GeneratePosit(Posit64 val)
{
	ofs.printf("\r\n\t.align 8\r\n");
	ofs.printf("\t.8byte\t%s", val.ToString());
	gentype = longgen;
	outcol = 65;
	genst_cumulative += 8;
}

void GenerateReference(SYM *sp,int64_t offset)
{
	char sign;
  if( offset < 0) {
    sign = '-';
    offset = -offset;
  }
  else
    sign = '+';
  if( gentype == longgen && outcol < 55 - (int)sp->name->length()) {
        if( sp->storage_class == sc_static) {
			ofs.printf(",");
			ofs.printf(GetNamespace());
			ofs.printf("_%lld", sp->value.i);
			ofs.putch(sign);
			ofs.printf("%lld", offset);
//                fprintf(output,",%s_%ld%c%d",GetNamespace(),sp->value.i,sign,offset);
		}
        else if( sp->storage_class == sc_thread) {
			ofs.printf(",");
			ofs.printf(GetNamespace());
			ofs.printf("_%lld", sp->value.i);
			ofs.putch(sign);
			ofs.printf("%lld", offset);
//                fprintf(output,",%s_%ld%c%d",GetNamespace(),sp->value.i,sign,offset);
		}
		else {
			if (offset==0) {
                ofs.printf(",%s",(char *)sp->name->c_str());
			}
			else {
                ofs.printf(",%s",(char *)sp->name->c_str());
				ofs.putch(sign);
				ofs.printf("%lld",offset);
			}
		}
        outcol += (11 + sp->name->length());
    }
    else {
        nl();
        if(sp->storage_class == sc_static) {
			ofs.printf("\tdco\t%s",GetNamespace());
			ofs.printf("_%lld",sp->value.i);
			ofs.putch(sign);
			ofs.printf("%lld",offset);
//            fprintf(output,"\tdw\t%s_%ld%c%d",GetNamespace(),sp->value.i,sign,offset);
		}
        else if(sp->storage_class == sc_thread) {
//            fprintf(output,"\tdw\t%s_%ld%c%d",GetNamespace(),sp->value.i,sign,offset);
			ofs.printf("\tdco\t%s",GetNamespace());
			ofs.printf("_%lld",sp->value.i);
			ofs.putch(sign);
			ofs.printf("%lld",offset);
		}
		else {
			if (offset==0) {
				ofs.printf("\tdco\t%s",(char *)sp->name->c_str());
			}
			else {
				ofs.printf("\tdco\t%s",(char *)sp->name->c_str());
				ofs.putch(sign);
				ofs.printf("%lld", offset);
//				fprintf(output,"\tdw\t%s%c%d",sp->name,sign,offset);
			}
		}
        outcol = 26 + sp->name->length();
        gentype = longgen;
    }
}

void genstorageskip(int nbytes)
{
	char buf[200];
	int64_t nn;

	nl();
	nn = (nbytes + 7) >> 3;
	if (nn) {
		sprintf_s(buf, sizeof(buf), "\talign\t8\r\n\tdd\t0x%I64X\r\n", nn | 0xFFF0200000000000LL);
		ofs.printf("%s", buf);
	}
}

std::streampos genstorage(int64_t nbytes)
{
	static int nn = 1;

	std::streampos pos = ofs.tellp();
	nl();
	if (nbytes) {
		ofs.printf("\t.byte 0\n");
//		ofs.printf("\t.lcomm %s,%lld                    \n", nam, nbytes);
	}
	genst_cumulative += nbytes;
	nn++;
	return (pos);
}

void GenerateLabelReference(int n, int64_t offset)
{ 
	char buf[200];

	if( gentype == longgen && outcol < 58) {
		if (offset==0)
			sprintf_s(buf, sizeof(buf), ",%s_%d", GetNamespace(), n);
		else
			sprintf_s(buf, sizeof(buf), ",%s_%d+%lld", GetNamespace(), n, offset);
		ofs.printf(buf);
        outcol += 6;
    }
    else {
        nl();
				if (offset == 0)
					sprintf_s(buf, sizeof(buf), "\tdw\t%s_%d", GetNamespace(), n);
				else
					sprintf_s(buf, sizeof(buf), "\tdw\t%s_%d+%lld", GetNamespace(), n, offset);
				ofs.printf(buf);
        outcol = 22;
        gentype = longgen;
    }
}

/*
 *      make s a string literal and return it's label number.
 */
int stringlit(char *s)
{      
	struct slit *lp;

	lp = (struct slit *)allocx(sizeof(struct slit));
	lp->label = nextlabel++;
	lp->str = my_strdup(s);
	lp->nmspace = my_strdup(GetNamespace());
	if (strtab == nullptr) {
		strtab = lp;
		strtab->tail = lp;
	}
	else {
		strtab->tail->next = lp;
		strtab->tail = lp;
	}
	lp->isString = true;
	lp->pass = pass;
	return (lp->label);
}

int litlist(ENODE *node)
{
	struct slit *lp;
	ENODE *ep;

	lp = strtab;
	while (lp) {
		if (lp->isString) {
			lp = lp->next;
			continue;
		}
		ep = (ENODE *)lp->str;
		if (node->IsEqual(node, ep)) {
			return (lp->label);
		}
		lp = lp->next;
	}
	lp = (struct slit *)allocx(sizeof(struct slit));
	lp->label = nextlabel++;
	lp->str = (char *)node;
	lp->nmspace = my_strdup(GetNamespace());
	if (strtab == nullptr) {
		strtab = lp;
		strtab->tail = lp;
	}
	else {
		strtab->tail->next = lp;
		strtab->tail = lp;
	}
	lp->isString = false;
	lp->pass = pass;
	return (lp->label);
}

// Since there are two passes to the compiler the cases might already be
// recorded.

int caselit(struct scase *cases, int64_t num)
{
	struct clit *lp;

	lp = casetab;
	while (lp) {
		if (memcmp(lp->cases, cases, num * sizeof(struct scase)) == 0)
			return (lp->label);
		lp = lp->next;
	}
	lp = (struct clit *)allocx(sizeof(struct clit));
	lp->label = nextlabel++;
	lp->nmspace = my_strdup(GetNamespace());
	lp->cases = (struct scase *)allocx(sizeof(struct scase)*(int)num);
	lp->num = (int)num;
	lp->pass = pass;
	memcpy(lp->cases, cases, (int)num * sizeof(struct scase));
	lp->next = casetab;
	casetab = lp;
	return lp->label;
}

int quadlit(Float128 *f128)
{
	Float128 *lp;
	lp = quadtab;
	// First search for the same literal constant and it's label if found.
	while(lp) {
		if (Float128::IsEqual(f128,Float128::Zero())) {
			if (Float128::IsEqualNZ(lp,f128))
				return (lp->label);
		}
		else if (Float128::IsEqual(lp,f128))
			return (lp->label);
		lp = lp->next;
	}
	lp = (Float128 *)allocx(sizeof(Float128));
	lp->label = nextlabel++;
	Float128::Assign(lp,f128);
	lp->nmspace = my_strdup(GetNamespace());
	lp->next = quadtab;
	quadtab = lp;
	return (lp->label);
}


int NumericLiteral(ENODE* node)
{
	struct nlit* lp;
	lp = numeric_tab;
	// First search for the same literal constant and it's label if found.
	while (lp) {
		if (lp->typ == node->etype) {
			switch (node->etype) {
			case bt_float:
				if (lp->f == node->f)
					return (lp->label);
				break;
			case bt_double:
				if (lp->f == node->f)
					return (lp->label);
				break;
			case bt_quad:
				if (Float128::IsEqual(&node->f128, Float128::Zero())) {
					if (Float128::IsEqualNZ(&lp->f128, &node->f128))
						return (lp->label);
				}
				else if (Float128::IsEqual(&lp->f128, &node->f128))
					return (lp->label);
				break;
			case bt_posit:
				if (Posit64::IsEqual(lp->p, node->posit))
					return (lp->label);
				break;
			}
		}
		lp = lp->next;
	}
	lp = (struct nlit*)allocx(sizeof(struct nlit));
	lp->label = nextlabel++;
	Float128::Assign(&lp->f128, &node->f128);
	lp->p.val = node->posit.val;
	lp->f = node->f;
	lp->nmspace = my_strdup(GetNamespace());
	lp->next = numeric_tab;
	lp->typ = node->etype;
	if (node->tp)
		lp->precision = node->tp->precision;
	else
		lp->precision = 64;
	numeric_tab = lp;
	return (lp->label);
}


char *strip_crlf(char *p)
{
     static char buf[2000];
     int nn;

     for (nn = 0; *p && nn < 1998; p++) {
         if (*p != '\r' && *p!='\n') {
            buf[nn] = *p;
            nn++;
         }
     }
     buf[nn] = '\0';
	 return buf;
}

int64_t GetStrtabLen()
{
	struct slit *p;
	int64_t len;
	char *cp;

	len = 0;
	for (p = strtab; p; p = p->next) {
		if (p->isString) {
			cp = p->str;
			while (*cp) {
				len++;
				cp++;
			}
			len++;	// for null char
		}
	}
	len += 7;
	len >>= 3;
	return (len);
}

int64_t GetQuadtabLen()
{
	Float128 *p;
	int64_t len;

	len = 0;
	for (p = quadtab; p; p = p->next) {
		len++;
	}
	return (len);
}

// Dump the literal pools.

void dumplits()
{
	char *cp;
	int64_t nn;
	slit *lit;
	union _tagFlt {
		double f;
		int64_t i;
	} Flt;
	union _tagFlt uf;
	int ln;
	int algn;

	dfs.printf("<Dumplits>\n");
	roseg();
	if (casetab) {
		nl();
		align(8);
		nl();
	}
	while (casetab != nullptr) {
		nl();
		if (casetab->pass == 2)
			put_label(casetab->label, "", casetab->nmspace, 'C', 0, 0);// 'D');
		for (nn = 0; nn < casetab->num; nn++) {
			if (casetab->cases[nn].pass==2)
				GenerateLabelReference(casetab->cases[nn].label, 0);
		}
		casetab = casetab->next;
	}
	if (numeric_tab) {
		nl();
		align(8);
		nl();
	}
	while (numeric_tab != nullptr) {
		nl();
		if (DataLabels[numeric_tab->label])
			switch (numeric_tab->typ) {
			case bt_float:
			case bt_double:
				put_label(numeric_tab->label, "", numeric_tab->nmspace, 'D', sizeOfFPD, 8);
				ofs.printf("\tdct\t");
				numeric_tab->f128.Pack(64);
				ofs.printf("%s", numeric_tab->f128.ToString(64));
				outcol += 35;
				break;
			case bt_quad:
				put_label(numeric_tab->label, "", numeric_tab->nmspace, 'D', sizeOfFPQ, 16);
				ofs.printf("\tdct\t");
				numeric_tab->f128.Pack(64);
				ofs.printf("%s", numeric_tab->f128.ToString(64));
				outcol += 35;
				break;
			case bt_posit:
				switch (numeric_tab->precision) {
				case 16:
					put_label(numeric_tab->label, "", numeric_tab->nmspace, 'D', 2, 2);
					ofs.printf("\t\dcw\t");
					ofs.printf("0x%04X\n", (int)(numeric_tab->p.val & 0xffffLL));
					outcol += 35;
					break;
				case 32:
					put_label(numeric_tab->label, "", numeric_tab->nmspace, 'D', 4, 4);
					ofs.printf("\t\dct\t");
					ofs.printf("0x%08X\n", (int)(numeric_tab->p.val & 0xffffffffLL));
					outcol += 35;
					break;
				default:
					put_label(numeric_tab->label, "", numeric_tab->nmspace, 'D', 8, 8);
					ofs.printf("\t\dco\t");
					ofs.printf("0x%016I64X\n", numeric_tab->p.val);
					outcol += 35;
					break;
				}
				break;
			case bt_void:
				put_label(numeric_tab->label, "", numeric_tab->nmspace, 'D', 0, 1);
				break;
			default:
				put_label(numeric_tab->label, "", numeric_tab->nmspace, 'D', 0, 2);
				;// printf("hi");
			}
		numeric_tab = numeric_tab->next;
	}

	if (quadtab) {
		nl();
		align(8);
		nl();
	}

	// Dumping to ro segment - no need for GC skip
	/*
	nn = GetQuadtabLen();
	if (nn) {
		sprintf_s(buf, sizeof(buf), "\tdw\t$%I64X ; GC_skip\n", nn | 0xFFF0200000000000LL);
		ofs.printf("%s", buf);
	}
	*/
	while(quadtab != nullptr) {
		nl();
		if (DataLabels[quadtab->label]) {
			put_label(quadtab->label, "", quadtab->nmspace, 'D', sizeOfFPQ, 16);
			ofs.printf("\tdh\t");
			quadtab->Pack(64);
			ofs.printf("%s", quadtab->ToString(64));
			outcol += 35;
		}
		quadtab = quadtab->next;
	}
	if (strtab) {
		nl();
		align(8);
		nl();
	}

	//nn = GetStrtabLen();
	//if (nn) {
	//	sprintf_s(buf, sizeof(buf), "\tdw\t$%I64X ; GC_skip\n", nn | 0xFFF0200000000000LL);
	//	ofs.printf("%s", buf);
	//}
	for (lit = strtab; lit; lit = lit->next) {
		ENODE *ep;
		agr = ep = (ENODE *)lit->str;
		dfs.printf(".");
		nl();
		if (!lit->isString) {
			if (DataLabels[lit->label])
				put_label(lit->label, strip_crlf(&lit->str[1]), lit->nmspace, 'D', ep->esize, ep->GetAggregateAlignment());
		}
		else {
			cp = lit->str;
			ln = 0;
			switch (*cp) {
			case 'B':
				algn = 1;
				cp++;
				while (*cp++)
					ln++;
				ln++;
				break;
			case 'W':
				algn = 2;
				cp++;
				while (*cp++)
					ln+=2;
				ln+=2;
				break;
			case 'T':
				algn = 4;
				cp++;
				while (*cp++)
					ln += 4;
				ln += 4;
				break;
			case 'O':
				algn = 8;
				cp++;
				while (*cp++)
					ln += 8;
				ln += 8;
				break;
			}
			put_label(lit->label, strip_crlf(&lit->str[1]), lit->nmspace, 'D', ln, algn);
		}
		if (lit->isString) {
			cp = lit->str;
			switch (*cp) {
			case 'B':
				cp++;
				while (*cp)
					GenerateByte(*cp++);
				GenerateByte(0);
				break;
			case 'W':
				cp++;
				while (*cp)
					GenerateChar(*cp++);
				GenerateChar(0);
				break;
			case 'T':
				cp++;
				while (*cp)
					GenerateHalf(*cp++);
				GenerateHalf(0);
				break;
			case 'O':
				cp++;
				while (*cp)
					GenerateWord(*cp++);
				GenerateWord(0);
				break;
			}
		}
		else {
			if (DataLabels[lit->label]) {
				ep->PutStructConst(ofs);
			}
		}
	}
	strtab = nullptr;
	nl();
	dfs.printf("</Dumplits>\n");
}

void nl()
{       
//	if(outcol > 0) {
		ofs.printf("\n");
		outcol = 0;
		gentype = nogen;
//	}
}

void align(int n)
{
	ofs.printf("\t.align\t%d\n",n);
}

void cseg()
{
	{
		if (curseg != codeseg) {
			nl();
			ofs.printf("\t.text\n");
			ofs.printf("\t.align\t4\n");
			curseg = codeseg;
		}
	}
}

void dseg()
{    
	nl();
	if( curseg != dataseg) {
		ofs.printf("\t.data\n");
		curseg = dataseg;
    }
	ofs.printf("\t.align\t8\n");
}

void tseg()
{    
	if( curseg != tlsseg) {
		nl();
		ofs.printf("\t.section .tls\n");
		ofs.printf("\t.align\t8\n");
		curseg = tlsseg;
    }
}

void roseg()
{
	if( curseg != rodataseg) {
		nl();
		ofs.printf("\t.section .rodata\n");
		ofs.printf("\t.align\t16\n");
		curseg = rodataseg;
    }
}

void seg(int sg, int algn)
{    
	nl();
	if( curseg != sg) {
		switch(sg) {
		case bssseg:
			ofs.printf("\t.section .bss\n");
			break;
		case dataseg:
			ofs.printf("\t.data\n");
			break;
		case tlsseg:
			ofs.printf("\t.section .tls\n");
			break;
		case idataseg:
			ofs.printf("\t.idata\n");
			break;
		case codeseg:
			ofs.printf("\t.text\n");
			break;
		case rodataseg:
			ofs.printf("\t.section .rodata\n");
			break;
		}
		curseg = sg;
    }
 	ofs.printf("\t.align\t%d\n", algn);
}
