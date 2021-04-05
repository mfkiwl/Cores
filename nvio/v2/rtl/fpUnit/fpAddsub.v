// ============================================================================
//        __
//   \\__/ o\    (C) 2006-2019  Robert Finch, Waterloo
//    \  __ /    All rights reserved.
//     \/_//     robfinch<remove>@finitron.ca
//       ||
//
//	fpAddsub.v
//    - floating point adder/subtracter
//    - ten cycle latency
//    - can issue every clock cycle
//    - parameterized width
//    - IEEE 754 representation
//
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

`include "fpConfig.sv"

module fpAddsub(clk, ce, rm, op, a, b, o);
parameter FPWID = 128;
`include "fpSize.sv"

input clk;		// system clock
input ce;		// core clock enable
input [2:0] rm;	// rounding mode
input op;		// operation 0 = add, 1 = subtract
input [MSB:0] a;	// operand a
input [MSB:0] b;	// operand b
output [EX:0] o;	// output

wire so;			// sign output
wire [EMSB:0] xo;	// de normalized exponent output
reg [FX:0] mo;	// mantissa output

assign o = {so,xo,mo};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// Clock edge #1
// - Decompose inputs into more digestible values.
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
wire [MSB:0] a1;
wire [MSB:0] b1;
wire sa1, sb1;
wire [EMSB:0] xa1, xb1;
wire [FMSB:0] ma1, mb1;
wire [FMSB+1:0] fracta1, fractb1;
wire adn1, bdn1;		// a,b denormalized ?
wire xaInf1, xbInf1;
wire aInf1, bInf1;
wire aNan1, bNan1;
wire az1, bz1;	// operand a,b is zero
wire op1;

fpDecompReg #(FPWID) u1a (.clk(clk), .ce(ce), .i(a), .o(a1), .sgn(sa1), .exp(xa1), .man(ma1), .fract(fracta1), .xz(adn1), .vz(az1), .xinf(xaInf1), .inf(aInf1), .nan(aNan1) );
fpDecompReg #(FPWID) u1b (.clk(clk), .ce(ce), .i(b), .o(b1), .sgn(sb1), .exp(xb1), .man(mb1), .fract(fractb1), .xz(bdn1), .vz(bz1), .xinf(xbInf1), .inf(bInf1), .nan(bNan1) );
delay1 #(1)  dop1(.clk(clk), .ce(ce), .i(op), .o(op1) );

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// Clock edge #2
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
reg xabeq2;
reg mabeq2;
reg anbz2;
reg xabInf2;
reg anbInf2;
wire [EMSB:0] xa2, xb2;
wire [FMSB:0] ma2, mb2;
// operands sign,exponent,mantissa
wire [FMSB+1:0] fracta2, fractb2;
wire az2, bz2;	// operand a,b is zero
reg xa_gt_xb2;
reg var2;
reg [EMSB:0] xad2;
reg [EMSB:0] xbd2;
reg realOp2;

delay1 #(EMSB+1)  dxa2(.clk(clk), .ce(ce), .i(xa1), .o(xa2) );
delay1 #(EMSB+1)  dxb2(.clk(clk), .ce(ce), .i(xb1), .o(xb2) );
delay1 #(FMSB+1)  dma2(.clk(clk), .ce(ce), .i(ma1), .o(ma2) );
delay1 #(FMSB+1)  dmb2(.clk(clk), .ce(ce), .i(mb1), .o(mb2) );
delay1 #(1)  daz2(.clk(clk), .ce(ce), .i(az1), .o(az2) );
delay1 #(1)  dbz2(.clk(clk), .ce(ce), .i(bz1), .o(bz2) );
delay1 #(FMSB+2)  dfracta2(.clk(clk), .ce(ce), .i(fracta1), .o(fracta2) );
delay1 #(FMSB+2)  dfractb2(.clk(clk), .ce(ce), .i(fractb1), .o(fractb2) );

always @(posedge clk)
	if (ce) xa_gt_xb2 <= xa1 > xb1;
always @(posedge clk)
	if (ce) var2 <= (xa1==xb1 && ma1 > mb1);
always @(posedge clk)
	if (ce) xad2 <= xa1|adn1;	// operand a exponent, compensated for denormalized numbers
always @(posedge clk)
	if (ce) xbd2 <= xb1|bdn1;	// operand b exponent, compensated for denormalized numbers
always @(posedge clk)
	if (ce) xabeq2 <= xa1==xb1;
always @(posedge clk)
	if (ce) mabeq2 <= ma1==mb1;
always @(posedge clk)
	if (ce) anbz2 <= az1 & bz1;
always @(posedge clk)
	if (ce) xabInf2 <= xaInf1 & xbInf1;
always @(posedge clk)
	if (ce) anbInf2 <= aInf1 & bInf1;

// Figure out which operation is really needed an add or
// subtract ?
// If the signs are the same, use the orignal op,
// otherwise flip the operation
//  a +  b = add,+
//  a + -b = sub, so of larger
// -a +  b = sub, so of larger
// -a + -b = add,-
//  a -  b = sub, so of larger
//  a - -b = add,+
// -a -  b = add,-
// -a - -b = sub, so of larger
always @(posedge clk)
	if (ce) realOp2 <= op1 ^ sa1 ^ sb1;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// Clock edge #3
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
wire [EMSB:0] xa3, xb3;
wire xa_gt_xb3;
reg x_gt_b3;
wire xabInf3;
wire sa3,sb3;
wire op3;
wire [2:0] rm3;
reg [EMSB:0] xdiff3;
// which has greater magnitude ? Used for sign calc
reg a_gt_b3;
reg resZero3;
reg [FMSB+1:0] mfs3;
wire aNan3, bNan3;

delay1 #(EMSB+1)  dxa3(.clk(clk), .ce(ce), .i(xa2), .o(xa3));
delay1 #(EMSB+1)  dxb3(.clk(clk), .ce(ce), .i(xb2), .o(xb3));
delay1 #(1) dxabInf2(.clk(clk), .ce(ce), .i(xabInf2), .o(xabInf3));
delay1 #(1) dxagtxb2(.clk(clk), .ce(ce), .i(xa_gt_xb2), .o(xa_gt_xb3));
delay2 #(1) dsa2(.clk(clk), .ce(ce), .i(sa1), .o(sa3));
delay2 #(1) dsb2(.clk(clk), .ce(ce), .i(sb1), .o(sb3));
delay2 #(1) dop2(.clk(clk), .ce(ce), .i(op1), .o(op3));
delay3 #(3) drm2(.clk(clk), .ce(ce), .i(rm), .o(rm3));
delay2 #(1) dan2(.clk(clk), .ce(ce), .i(aNan1), .o(aNan3));
delay2 #(1) dbn2(.clk(clk), .ce(ce), .i(bNan1), .o(bNan3));

always @(posedge clk)
	if (ce) a_gt_b3 <= xa_gt_xb2 || var2;
// Find out if the result will be zero.
always @(posedge clk)
	if (ce) resZero3 <= (realOp2 & xabeq2 & mabeq2) |	anbz2;	// subtract, same magnitude, 	both a,b zero

// Compute the difference in exponents, provides shift amount
always @(posedge clk)
	if (ce) xdiff3 <= xa_gt_xb2 ? xad2 - xbd2 : xbd2 - xad2;
// determine which fraction to denormalize
always @(posedge clk)
	if (ce) mfs3 <= xa_gt_xb2 ? fractb2 : fracta2;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// Clock edge #4
// Compute output exponent
//
// The output exponent is the larger of the two exponents, unless a subtract
// operation is in progress and the two numbers are equal, in which case the
// exponent should be zero.
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
reg [EMSB:0] xdif4;
wire [FMSB+1:0] mfs4;
reg [EMSB:0] xo4;	// de normalized exponent output
reg so4;

always @(posedge clk)
	if (ce) xo4 <= xabInf3 ? xa3 : resZero3 ? {EMSB+1{1'b0}} : xa_gt_xb3 ? xa3 : xb3;

// Compute output sign
always @(posedge clk)
if (ce)
	casez ({aNan3,bNan3,resZero3,sa3,op3,sb3})	// synopsys full_case parallel_case
	6'b10????: so4 <= sa3;
	6'b01????: so4 <= sb3;
	6'b11????: so4 <= a_gt_b3 ? sa3 : sb3;
	6'b000000: so4 <= 0;			// + + + = +
	6'b000001: so4 <= a_gt_b3 ? 1'b0 : 1'b1;	// + + - = sign of larger
	6'b000010: so4 <= a_gt_b3 ? 1'b0 : 1'b1;	// + - + = sign of larger
	6'b000011: so4 <= 0;			// + - - = +
	6'b000100: so4 <= a_gt_b3 ? 1'b1 : 1'b0;		// - + + = sign of larger
	6'b000101: so4 <= 1;			// - + - = -
	6'b000110: so4 <= 1;			// - - + = -
	6'b000111: so4 <= a_gt_b3 ? 1'b1 : 1'b0;		// - - - = sign of larger

	6'b001000: so4 <= 0;			//  A +  B, sign = +
	6'b001001: so4 <= rm3==3'd3;		//  A + -B, sign = + unless rounding down
	6'b001010: so4 <= rm3==3'd3;		//  A -  B, sign = + unless rounding down
	6'b001011: so4 <= 0;			// +A - -B, sign = +
	6'b001100: so4 <= rm3==3'd3;		// -A +  B, sign = + unless rounding down
	6'b001101: so4 <= 1;			// -A + -B, sign = -
	6'b001110: so4 <= 1;			// -A - +B, sign = -
	6'b001111: so4 <= rm3==3'd3;		// -A - -B, sign = + unless rounding down
	endcase

always @(posedge clk)
if (ce) xdif4 <= xdiff3 > FMSB+3 ? FMSB+3 : xdiff3;
delay1 #(FMSB+2) dmsf3(.clk(clk), .ce(ce), .i(mfs3), .o(mfs4));

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// Clock edge #5
// Determine the sticky bit
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
wire [EMSB:0] xdif5;
wire [FMSB+1:0] mfs5;
wire sticky, sticky5;

// register inputs to shifter and shift
delay1 #(1)      dstky4(.clk(clk), .ce(ce), .i(sticky), .o(sticky5) );
delay1 #(EMSB+1) dxdif4(.clk(clk), .ce(ce), .i(xdif4), .o(xdif5) );
delay1 #(FMSB+2) dmsf4(.clk(clk), .ce(ce), .i(mfs4), .o(mfs5));

generate
begin
if (FPWID==128)
    redor128 u1 (.a(xdif4), .b({mfs4,2'b0}), .o(sticky) );
else if (FPWID==96)
    redor96 u1 (.a(xdif4), .b({mfs4,2'b0}), .o(sticky) );
else if (FPWID==84)
    redor84 u1 (.a(xdif4), .b({mfs4,2'b0}), .o(sticky) );
else if (FPWID==80)
    redor80 u1 (.a(xdif4), .b({mfs4,2'b0}), .o(sticky) );
else if (FPWID==64)
    redor64 u1 (.a(xdif4), .b({mfs4,2'b0}), .o(sticky) );
else if (FPWID==40)
    redor40 u1 (.a(xdif4), .b({mfs4,2'b0}), .o(sticky) );
else if (FPWID==32)
    redor32 u1 (.a(xdif4), .b({mfs4,2'b0}), .o(sticky) );
end
endgenerate

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// Clock edge #6
// Shift (denormalize)
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
reg [FMSB+3:0] md6;
wire xa_gt_xb6;
wire [FMSB+1:0] fracta6, fractb6;

delay3 #(1) dxagtxb5(.clk(clk), .ce(ce), .i(xa_gt_xb3), .o(xa_gt_xb6));
delay4 #(FMSB+2)  dfracta5(.clk(clk), .ce(ce), .i(fracta2), .o(fracta6) );
delay4 #(FMSB+2)  dfractb5(.clk(clk), .ce(ce), .i(fractb2), .o(fractb6) );

always @(posedge clk)
	if (ce) md6 <= ({mfs5,2'b0} >> xdif5)|sticky5;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// Clock edge #7
// Sort operands
// addition can generate an extra bit, subtract can't go negative
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
reg [FMSB+3:0] oa7;
reg [FMSB+3:0] ob7;
wire a_gt_b7;

delay4 #(1) dagtb5(.clk(clk), .ce(ce), .i(a_gt_b3), .o(a_gt_b7));

always @(posedge clk)
	if (ce) oa7 <= xa_gt_xb6 ? {fracta6,2'b0} : md6;
always @(posedge clk)
	if (ce) ob7 <= xa_gt_xb6 ? md6 : {fractb6,2'b0};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// Clock edge #8
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
reg [FMSB+3:0] oaa8;
reg [FMSB+3:0] obb8;
wire [EMSB:0] xo8;
wire realOp8;
vtdl #(.WID(1)) drealop7 (.clk(clk), .ce(ce), .a(4'd5), .d(realOp2), .q(realOp8));
vtdl #(.WID(EMSB+1)) dxo7(.clk(clk), .ce(ce), .a(4'd3), .d(xo4), .q(xo8));
always @(posedge clk)
	if (ce) oaa8 <= a_gt_b7 ? oa7 : ob7;
always @(posedge clk)
	if (ce) obb8 <= a_gt_b7 ? ob7 : oa7;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// Clock edge #9
// perform add/subtract
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
reg [FMSB+4:0] mab9;
wire anbInf9;
wire aNan9, bNan9;
wire op9;
wire [FMSB+1:0] fracta9, fractb9;
wire xo9;
reg xinf9;

vtdl #(1) danbInf7(.clk(clk), .ce(ce), .a(4'd6), .d(anbInf2), .q(anbInf9));
vtdl #(1) danan8(.clk(clk), .ce(ce), .a(4'd7), .d(aNan1), .q(aNan9));
vtdl #(1) dbnan8(.clk(clk), .ce(ce), .a(4'd7), .d(bNan1), .q(bNan9));
vtdl #(1) dop6(.clk(clk), .ce(ce), .a(4'd5), .d(op3), .q(op9));
delay3 #(FMSB+2)  dfracta8(.clk(clk), .ce(ce), .i(fracta6), .o(fracta9) );
delay3 #(FMSB+2)  dfractb8(.clk(clk), .ce(ce), .i(fractb6), .o(fractb9) );

always @(posedge clk)
	if (ce) mab9 <= realOp8 ? oaa8 - obb8 : oaa8 + obb8;
always @(posedge clk)
	if (ce) xinf9 <= xo8 == {EMSB+1{1'b1}};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
// Clock edge #10
// Final outputs
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
vtdl #(1) dso6(.clk(clk), .ce(ce), .a(4'd5), .d(so4), .q(so));
vtdl #(.WID(EMSB+1)) dxo6(.clk(clk), .ce(ce), .a(4'd1), .d(xo8), .q(xo));

always @(posedge clk)
if (ce)
	casez({anbInf9,aNan9,bNan9,xinf9})
	4'b1???:	mo <= {1'b0,op9,{FMSB-1{1'b0}},op9,{FMSB{1'b0}}};	// inf +/- inf - generate QNaN on subtract, inf on add
	4'b01??:	mo <= {1'b1,1'b1,fracta9[FMSB-1:0],{FMSB+1{1'b0}}};	// Set MSB of Nan to convert to quiet
	4'b001?: 	mo <= {1'b1,1'b1,fractb9[FMSB-1:0],{FMSB+1{1'b0}}};
	4'b0001:	mo <= 1'd0;		// exponent hit infinity -> force mantissa to zero
	default:	mo <= {mab9,{FMSB-1{1'b0}}};	// mab has an extra lead bit and two trailing bits
	endcase

endmodule

module fpAddsubnr(clk, ce, rm, op, a, b, o);
parameter FPWID = 128;
`include "fpSize.sv"

input clk;		// system clock
input ce;		// core clock enable
input [2:0] rm;	// rounding mode
input op;		// operation 0 = add, 1 = subtract
input [MSB:0] a;	// operand a
input [MSB:0] b;	// operand b
output [MSB:0] o;	// output

wire [EX:0] o1;
wire [MSB+3:0] fpn0;

fpAddsub  #(FPWID) u1 (clk, ce, rm, op, a, b, o1);
fpNormalize #(FPWID) u2(.clk(clk), .ce(ce), .under_i(1'b0), .i(o1), .o(fpn0) );
fpRound  	#(FPWID) u3(.clk(clk), .ce(ce), .rm(rm), .i(fpn0), .o(o) );

endmodule
