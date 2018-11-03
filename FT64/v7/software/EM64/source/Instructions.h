#pragma once

#define IBRK    0x00
#define IRR		0x02
#define IR1			0x01
#define IMEMDB			0x10
#define IMEMSB			0x11
#define ISYNC			0x12
#define ISHIFT		0x0F
#define ISHIFTH		0x3F
#define ISHIFTC		0x2F
#define ISHIFTB		0x1F
#define ISHL			0x0
#define ISHR			0x1
#define IASL			0x2
#define IASR			0x3
#define IROL			0x4
#define IROR			0x5
#define ISHLI			0x8
#define ISHRI			0x9
#define IASLI			0xA
#define IASRI			0xB
#define IROLI			0xC
#define IRORI			0xD
#define IBTFLD	0x22
#define IBFSET			0x0
#define IBFCLR			0x1
#define IBFCHG			0x2
#define IBFINS			0x3
#define IBFINSI			0x4
#define IBFEXT			0x5
#define IBFEXTU			0x6
#define INAND		0x0C
#define INOR			0x0D
#define IXNOR		0x0E
#define ILHX			0x10
#define ILHUX		0x11
#define ILWX			0x12
#define ILBX			0x13
#define ISHX			0x14
#define ISBX			0x15
#define ISWX			0x16
#define ISWCX		0x17
#define ILWRX		0x1D
#define ILCX			0x20
#define ILCUX		0x21
#define IMOV		0x22
#define ILBUX		0x23
#define ISCX			0x24
#define ICASX		0x25
#define ICMOVEQ		0x28
#define ICMOVNE		0x29
#define IMUX			0x2A
#define IDEMUX		0x2B
#define IMIN			0x2C
#define IMAX			0x2D
#define IXCHG		0x2E
#define ISEI         0x30
#define IWAIT        0x31
#define IRTI         0x32
#define IMULU		0x38
#define IMULSU		0x39
#define IMUL		0x3A
#define ICHK		0x3B
#define IDIVMODU	0x3C
#define IDIVMODSU	0x3D
#define IDIVMOD		0x3E
#define IBccR	0x03
#define IADD    0x04
#define ISUB    0x05
#define ICMP    0x06
#define ICMPU	0x07
#define IAND    0x08
#define IOR     0x09
#define IXOR    0x0A
#define IFLOAT	0x0B
#define IFADD		0x04
#define IFSUB		0x05
#define IFCMP		0x06
#define IFMUL		0x08
#define IFDIV		0x09
#define IFLT2FIX	    0x12
#define IFIX2FLT	    0x13
#define IFMOV		0x10
#define IFNEG        0x14
#define IFABS		0x15
#define IFSIGN		0x16
#define IFMAN		0x17
#define IFNABS		0x18
#define IFTX			0x20
#define IFCX			0x21
#define IFEX			0x22
#define IFDX			0x23
#define IFRM			0x24
#define IFSYNC		0x36
#define IREX		0x0D
#define ICSR		0x0E
#define IEXEC	0x0F
#define IBBc		0x26
#define IBcc		0x30
#define IBEQ         0x0
#define IBNE         0x1
#define IBLT         0x2
#define IBGE         0x3
#define IBLTU        0x6
#define IBGEU        0x7
#define IRET    0x29
#define IJAL	0x18
#define INOP     0x1C
#define ILB			0x13
#define ILx			0x20
#define ILBU		0x23
#define ISx			0x24
#define ISB     0x15
#define ISWC	0x17
#define ICALL	0x19
#define IJMP	0x28
#define IDIVUI	0x3C
#define IDIVSUI	0x3D
#define IDIVI	0x3E
#define IMODUI	0x2C
#define IMODSUI	0x2D
#define IMODI	0x2E

#define ILWR	0x1D
