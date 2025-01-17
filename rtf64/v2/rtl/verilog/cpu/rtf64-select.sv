// ============================================================================
//        __
//   \\__/ o\    (C) 2019-2020  Robert Finch, Waterloo
//    \  __ /    All rights reserved.
//     \/_//     robfinch<remove>@finitron.ca
//       ||
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
//
import rtf64pkg::*;

module modSelect(opcode, sel);
input [7:0] opcode;
output reg [7:0] sel;

always @*
case(opcode)
`LDB,`LDBU,`LDBS,`LDBUS,`STB,`STBS:
  sel = 8'h01;
`LDW,`LDWU,`LDWS,`LDWUS,`STW,`STWS:
  sel = 8'h03;
`LDT,`LDTU,`LDTS,`LDTUS,`STT,`STTS:
  sel = 8'h0F;
`LDO,`LDOS,`LDOR,`LDORS,`STO,`STOS,`STOC,`STOCS,`STPTR,`STPTRS,`STOIS:
  sel = 8'hFF;
`FLDO,`PLDO,`FLDOS,`PLDOS:  
  sel = 8'hFF;
`FSTO,`PSTO,`FSTOS,`PSTOS:  
  sel = 8'hFF;
`JSR,`JSR18,`RTS:
  sel = 8'hFF;
default:	sel = 8'h00;
endcase

endmodule
