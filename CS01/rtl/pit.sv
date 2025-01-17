`timescale 1ns / 1ps
// ============================================================================
//        __
//   \\__/ o\    (C) 2017-2020  Robert Finch, Waterloo
//    \  __ /    All rights reserved.
//     \/_//     robfinch<remove>@finitron.ca
//       ||
//
//	- programmable interval timer
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
//
//	Reg	Description
//	00	current count   (read only)
//	04	max count	    (read-write)
//  08  on time			(read-write)
//	0C	control
//		byte 0 for counter 0, byte 1 for counter 1, byte 2 for counter 2
//		bit in byte
//		0 = 1 = load, automatically clears
//	  1 = 1 = enable counting, 0 = disable counting
//		2 = 1 = auto-reload on terminal count, 0 = no reload
//		3 = 1 = use external clock, 0 = internal clk_i
//    4 = 1 = use gate to enable count, 0 = ignore gate
//		5 = 1 = bcd counting, 0 = binary
//		6 = 1 = count up, 0 = count down
//	10	current count 1
//	14  max count 1
//	18  on time 1
//	20	current count 2
//	24	max count 2
//	28	on time 2
//
//	- all three counter controls can be written at the same time with a
//    single instruction allowing synchronization of the counters.
// ============================================================================
//
module pit(rst_i, clk_i, cs_i, cyc_i, stb_i, ack_o, sel_i, we_i, adr_i, dat_i, dat_o,
	clk0, gate0, out0, clk1, gate1, out1, clk2, gate2, out2
	);
input rst_i;
input clk_i;
input cs_i;
input cyc_i;
input stb_i;
output ack_o;
input [3:0] sel_i;
input we_i;
input [5:0] adr_i;
input [31:0] dat_i;
output reg [31:0] dat_o;
input clk0;
input gate0;
output out0;
input clk1;
input gate1;
output out1;
input clk2;
input gate2;
output out2;

integer n;
reg [31:0] maxcount [0:2];
wire [31:0] count [0:2];
reg [31:0] ont [0:2];
wire [2:0] gate;
wire [2:0] pulse;
reg ld [0:2];
reg ce [0:2];
reg ar [0:2];
reg ge [0:2];
reg xc [0:2];
reg bc [0:2];
reg ud [0:2];
reg out [0:2];
wire cce [0:2];
wire lld [0:2];

wire cs = cyc_i & stb_i & cs_i;
reg rdy;
always @(posedge clk_i)
	rdy <= cs;
assign ack_o = cs ? (we_i ? 1'b1 : rdy) : 1'b0;

assign out0 = out[0];
assign out1 = out[1];
assign out2 = out[2];
assign gate[0] = gate0;
assign gate[1] = gate1;
assign gate[2] = gate2;

edge_det ued0 (.rst(rst_i), .clk(clk_i), .ce(1'b1), .i(clk0), .pe(pulse[0]), .ne(), .ee());
edge_det ued1 (.rst(rst_i), .clk(clk_i), .ce(1'b1), .i(clk1), .pe(pulse[1]), .ne(), .ee());
edge_det ued2 (.rst(rst_i), .clk(clk_i), .ce(1'b1), .i(clk2), .pe(pulse[2]), .ne(), .ee());

pit_counter32 u1
(
	.clk(clk),
	.ce(cce[0]),
	.ld(lld[0]),
	.ud(ud[0]),
	.bcd(bc[0]),
	.d(ud ? 32'h0 : maxcount[0]),
	.q(count[0]),
	.c()
);

pit_counter32 u2
(
	.clk(clk),
	.ce(cce[1]),
	.ld(lld[1]),
	.ud(ud[1]),
	.bcd(bc[1]),
	.d(ud ? 32'h0 : maxcount[1]),
	.q(count[1]),
	.c()
);

pit_counter32 u3
(
	.clk(clk),
	.ce(cce[2]),
	.ld(lld[2]),
	.ud(ud[2]),
	.bcd(bc[2]),
	.d(ud ? 32'h0 : maxcount[2]),
	.q(count[2]),
	.c()
);

initial begin
	for (n = 0; n < 3; n = n + 1) begin
		maxcount[n] <= 32'd0;
		count[n] <= 32'd0;
		ont[n] <= 32'd0;
		ld[n] <= 1'b0;
		ce[n] <= 1'b0;
		ar[n] <= 1'b0;
		ge[n] <= 1'b0;
		xc[n] <= 1'b0;
		bc[n] <= 1'b0;
		ud[n] <= 1'b0;
		out[n] <= 1'b0;
	end
end

assign cce[0] = (xc[0] ? pulse[0] & ce[0] : ce[0]) & (ge[0] ? gate[0] : 1'b1);
assign cce[1] = (xc[1] ? pulse[1] & ce[1] : ce[1]) & (ge[1] ? gate[1] : 1'b1);
assign cce[2] = (xc[2] ? pulse[2] & ce[2] : ce[2]) & (ge[2] ? gate[2] : 1'b1);
assign lld[0] = (cce[0] && ar[0] && (ud[0] ? count[0]==maxcount[0] : count[0]==32'd0)) || ld[0];
assign lld[1] = (cce[1] && ar[1] && (ud[1] ? count[1]==maxcount[1] : count[1]==32'd0)) || ld[1];
assign lld[2] = (cce[2] && ar[2] && (ud[2] ? count[2]==maxcount[2] : count[2]==32'd0)) || ld[2];

always @(posedge clk_i)
if (rst_i) begin
	for (n = 0; n < 3; n = n + 1) begin
		ld[n] <= 1'b0;
		ce[n] <= 1'b0;
		ar[n] <= 1'b1;
		ge[n] <= 1'b0;
		bc[n] <= 1'b0;
		ud[n] <= 1'b0;
		out[n] <= 1'b0;
	end	
end
else begin
	for (n = 0; n < 3; n = n + 1) begin
		ld[n] <= 1'b0;
		if (cs && we_i && adr_i[5:4]==n)
		case(adr_i[3:2])
		2'd1:	maxcount[n] <= dat_i;
		2'd2:	ont[n] <= dat_i;
		2'd3:	begin
					if (sel_i[0]) begin
						ld[0] <= dat_i[0];
						ce[0] <= dat_i[1];
						ar[0] <= dat_i[2];
						xc[0] <= dat_i[3];
						ge[0] <= dat_i[4];
						bc[0] <= dat_i[5];
						ud[0] <= dat_i[6];
					end
					if (sel_i[1]) begin
						ld[1] <= dat_i[8];
						ce[1] <= dat_i[9];
						ar[1] <= dat_i[10];
						xc[1] <= dat_i[11];
						ge[1] <= dat_i[12];
						bc[1] <= dat_i[13];
						ud[1] <= dat_i[14];
					end
					if (sel_i[2]) begin
						ld[2] <= dat_i[16];
						ce[2] <= dat_i[17];
						ar[2] <= dat_i[18];
						xc[2] <= dat_i[19];
						ge[2] <= dat_i[20];
						bc[2] <= dat_i[21];
						ud[2] <= dat_i[22];
					end
				end
		default:	;
		endcase
		if (adr_i[5:4]==n)
			case(adr_i[3:2])
			2'd0:	dat_o <= count[n];
			2'd1:	dat_o <= maxcount[n];
			2'd2:	dat_o <= ont[n];
			2'd3:	dat_o <= {ud[2],bc[2],ge[2],xc[2],ar[2],ce[2],
							2'b0,ud[1],bc[1],ge[1],xc[1],ar[1],ce[1],
							2'b0,ud[0],bc[0],ge[0],xc[0],ar[0],ce[0],1'b0};
			endcase
		
		if (ld[n])
			;
		else if (cce[n]) begin
			if (count[n]==ont[n])
				out[n] <= ~ud[n];
			if (ud[n] ? count[n]==maxcount[n] : count[n]==32'd0) begin
				out[n] <= ud[n];
				if (!ar[n])
					ce[n] <= 1'b0;
			end
		end
	end
end

endmodule

module pit_counter4(clk, ce, ld, ud, bcd, d, q, c);
input clk;
input ce;
input ld;
input bcd;
input ud;
input [3:0] d;
output reg [3:0] q;
output reg c;

always @*
begin
	if (bcd) begin
		if (ud)
			ce <= q==4'h9;
		else
			ce <= q==4'h0;
	end
	else begin
		if (ud)
			ce <= q==4'hF;
		else
			ce <= q==4'h0;
	end
end

always @(posedge clk)
begin
	if (ld)
		q <= d;
	else if (ce) begin
		if (bcd) begin
			if (q==4'd9 && ud)
				q <= 4'd0;
			else if (q==4'd0 && !ud)
				q <= 4'd9;
			else
				q <= ud ? q + 2'd1 : q - 2'd1;
		end
		else begin
			q <= ud ? q + 2'd1 : q - 2'd1;
		end
	end
end

endmodule

module pit_counter32(clk, ce, ld, ud, bcd, d, q, c);
input clk;
input ce;
input ld;
input ud;
input bcd;
input [31:0] d;
output [31:0] q;
output c;

reg [8:0] cc;
assign c = cc[8];
always @*
	cc[0] <= 1'b1;

genvar g;
generate begin : gctrs
for (g = 0; g < 8; g = g + 1)
pit_counter4 u1
(
	.clk(clk),
	.ce(ce & cc[n]),
	.ld(ld),
	.ud(ud),
	.bcd(bcd),
	.d(d[g*4-1:g*4]),
	.q(q[g*4-1:g*4]),
	.c(cc[g+1])
);
end
endgenerate

endmodule
