// ============================================================================
//        __
//   \\__/ o\    (C) 2020  Robert Finch, Waterloo
//    \  __ /    All rights reserved.
//     \/_//     robfinch<remove>@finitron.ca
//       ||
//
//	PAM.sv
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

module PAM(rst, clk, alloc_i, free_i, freeall_i, pageno_i, pageno_o, done);
input rst;
input clk;
input alloc_i;
input free_i;
input freeall_i;
input [8:0] pageno_i;
output reg [8:0] pageno_o;
output reg done;

integer n;
reg [31:0] pam [0:15];
reg [3:0] state;
reg [3:0] wordno;
reg [4:0] bitno;
reg [31:0] map;

parameter IDLE = 4'd0;
parameter ALLOC1 = 4'd1;
parameter ALLOC2 = 4'd2;
parameter ALLOC3 = 4'd3;
parameter ALLOC4 = 4'd4;
parameter ALLOC5 = 4'd5;
parameter FREE1 = 4'd6;
parameter FREE2 = 4'd7;
parameter FREE3 = 4'd8;
parameter RESET = 4'd9;

always @(posedge clk)
if (rst) begin
	done <= 1'b0;
	wordno <= 4'd0;
	state <= RESET;
end
else begin
case (state)
IDLE:
	begin
		if (freeall_i) begin
			wordno <= 4'd0;
			done <= 1'b0;
			goto (RESET);
		end
		else if (free_i) begin
			wordno <= pageno_i[8:5];
			bitno <= pageno_i[4:0];
			done <= 1'b0;
			goto (FREE1);
		end
		else if (alloc_i) begin
			wordno <= 4'd0;
			done <= 1'b0;
			goto (ALLOC1);
		end
	end
RESET:
	begin
		pam[wordno] <= 32'h0;
		wordno <= wordno + 3'd1;
		if (wordno==4'd15) begin
			done <= 1'b1;
			goto (IDLE);
		end
	end

ALLOC1:
	begin
		map <= pam[wordno];
		// Force pages to always be allocated already
		// First 32 pages allocated for the OS
		if (wordno==4'd0)
			map <= 32'hFFFFFFFF;
		// Force last page allocated for system stack
		else if (wordno==4'd15)
			map[31] <= 1'b1;
		goto (ALLOC2);
	end
ALLOC2:
	begin
		goto (ALLOC3);
		if (map==32'hFFFFFFFF) begin
			wordno <= wordno + 2'd1;
			if (wordno==4'd15)
				goto (ALLOC5);
			else
				goto (ALLOC1);
		end
		for (n = 0; n < 32; n = n + 1)
			if (map[n]==1'b0)
				bitno <= n;
	end
ALLOC3:
	begin
		map[bitno] <= 1'b1;
		goto (ALLOC4);
	end
ALLOC4:
	begin
		pam[wordno] <= map;
		pageno_o <= {wordno,bitno};
		done <= 1'b1;
		goto (IDLE);
	end
ALLOC5:
	begin
		pageno_o <= 9'h00;
		done <= 1'b1;
		goto (IDLE);
	end

FREE1:
	begin
		map <= pam[wordno];
		goto(FREE2);
	end
FREE2:
	begin
		map[bitno] <= 1'b0;
		goto(FREE3);
	end
FREE3:
	begin
		pam[wordno] <= map;
		done <= 1'b1;
		goto (IDLE);
	end
endcase
	
end

task goto;
input [3:0] nst;
begin
	state <= nst;
end
endtask

endmodule