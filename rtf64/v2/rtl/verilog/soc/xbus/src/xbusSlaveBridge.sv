// ============================================================================
//        __
//   \\__/ o\    (C) 2020  Robert Finch, Waterloo
//    \  __ /    All rights reserved.
//     \/_//     robfinch<remove>@finitron.ca
//       ||
//
//	xbusSlaveBridge.sv
//
// BSD 3-Clause License
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// ============================================================================

module xbusSlaveBridge(dev_num_i, rst_i, clk_i, rclk_i, locked_i,
  cyc_o, stb_o, ack_i, we_o, sel_o, adr_o, dat_o, dat_i,
  xb_dat_i, xb_dat_o, xb_sync_o, xb_de_o, xb_sync_i, xb_de_i, xb_en_o);
input [3:0] dev_num_i; 
input rst_i;
input clk_i;
input rclk_i;
input locked_i;
output reg cyc_o;
output reg stb_o;
input ack_i;
output reg we_o;
output reg [15:0] sel_o;
output reg [31:0] adr_o;
output reg [127:0] dat_o;
input [127:0] dat_i;
input [35:0] xb_dat_i;
output reg [35:0] xb_dat_o;
output reg xb_sync_o;
output reg xb_de_o;
input xb_sync_i;
input xb_de_i;
output reg xb_en_o;

reg [3:0] state;
reg [3:0] ostate;
parameter IDLE = 4'd0;
parameter XD0_31 = 4'd3;
parameter XD32_63 = 4'd4;
parameter XD64_95 = 4'd5;
parameter XD96_127 = 4'd6;
parameter WAIT_ACK = 4'd7;
parameter WAIT_NACK = 4'd8;
parameter WAIT_LOCK = 4'd9;

reg ackw, ackr = 1'b0;
assign ack_o = ackw|ackr;


reg [31:0] adr;
reg [127:0] dath, dat;
reg [15:0] selh, sel;
reg [8:0] synccnt2;
reg [5:0] ctr;
reg [5:0] synccnt;
reg start_cycle, start_cycle1;
reg we;
reg data_cap;
reg was_write;
reg sync_locked;
reg sync_req;
reg [16:0] sync_req_cnt;
reg reset_sync_req_timeout;
reg sync_req_timeout;
reg [5:0] master_num;
reg [63:0] valid_master;

always @(posedge rclk_i)
if (rst_i)
  synccnt <= 6'd0;
else begin
  if (xb_sync_i)
    synccnt <= synccnt + 2'd1;
  else
    synccnt <= 6'd0;
end

always @(posedge rclk_i)
if (rst_i)
  sync_locked <= 1'b0;
else begin
  if (synccnt > 6'd10)
    sync_locked <= 1'b1;
end

always @(posedge clk_i)
if (rst_i)
  sync_req_cnt <= 17'd0;
else begin
  if (sync_req)
    sync_req_cnt <= sync_req_cnt + 2'd1;
  else
    sync_req_cnt <= 17'd0;
end

always @(posedge clk_i)
if (rst_i)
  sync_req_timeout <= 1'b0;
else begin
  if (reset_sync_req_timeout)
    sync_req_timeout <= 1'b0;
  else if (sync_req_cnt[16])
    sync_req_timeout <= 1'b1;
end

always @(posedge rclk_i)
if (rst_i) begin
  we <= 1'b0;
  sel <= 16'h0;
  adr <= 32'h0;
  dat <= 128'h0;
  start_cycle <= 1'b0;
  start_cycle1 <= 1'b0;
  xb_en_o <= 1'b0;
end
else begin
  if (sync_locked) begin
    start_cycle <= 1'b0;
    start_cycle1 <= start_cycle;
    case(xb_dat_i[35:32])
    4'h0:
      begin
        master_num <= xb_dat_i[29:24];
        if (valid_master[xb_dat_i[29:24]]) begin
          if (xb_dat_i[5:0]==dev_num_i) begin
            xb_en_o <= 1'b1;
            sync_req <= xb_dat_i[7];
          end
          else begin
            xb_en_o <= 1'b0;
          end
        end
        else
          sync_req <= 1'b0;
      end
    4'h1: adr[31:0] <= xb_dat_i[31:0];
    4'h3:
      begin
        if (valid_master[xb_dat_i[29:24]]) begin
          start_cycle <= xb_dat_i[23];
          we <= xb_dat_i[31];
          sel <= xb_dat_i[15:0];
        end
      end
    4'h4: dat[31:0] <= xb_dat_i[31:0];
    4'h5: dat[63:32] <= xb_dat_i[31:0];
    4'h6: dat[95:64] <= xb_dat_i[31:0];
    4'h7: dat[127:96] <= xb_dat_i[31:0];
    endcase
  end
end

always @(posedge clk_i)
if (rst_i) begin
  ostate <= IDLE;
  data_cap <= 1'b0;
  was_write <= 1'b0;
  dath <= 128'd0;
  selh <= 16'h0;
  xb_dat_o[35:32] <= 4'h0;  // send a NOP
  xb_dat_o[31:0] <= 32'h0;
  xb_sync_o <= 1'b0;
  xb_de_o <= 1'b1;
  valid_master <= 64'hFFFFFFFFFFFFFFFF;
  reset_sync_req_timeout <= 1'b0;
  ctr <= 6'd0;
end
else begin
if (start_cycle|start_cycle1) begin
  cyc_o <= 1'b1;
  stb_o <= 1'b1;
  we_o <= we;
  sel_o <= sel;
  adr_o <= adr;
  dat_o <= dat;
end
if (cyc_o & stb_o & ack_i) begin
  // capture data response
  dath <= dat_i;
  selh <= sel_o;
  // Terminate slave bus cycle
  cyc_o <= 1'b0;
  stb_o <= 1'b0;
  we_o <= 1'b0;
  sel_o <= 16'h0;
  data_cap <= 1'b1;
  was_write <= we_o;
end
reset_sync_req_timeout <= 1'b0;

case(ostate)

IDLE:
  begin
    if (sync_req) begin
      ctr <= ctr + 2'd1;
      //xb_sync_o <= ctr < 6'd16;
      xb_de_o <= ctr >= 6'd16 && ctr < 6'd60;
      xb_sync_o <= ctr >= 6'd32 && ctr < 6'd48;
      xb_dat_o[35:32] <= 4'h0;
      xb_dat_o[31:6] <= 28'h0;
      xb_dat_o[7] <= 1'b1;
      xb_dat_o[29:24] <= dev_num_i;
      xb_dat_o[5:0] <= master_num;
      valid_master[master_num] <= 1'b1;
      if (sync_req_timeout) begin
        reset_sync_req_timeout <= 1'b1;
        valid_master[master_num] <= 1'b0;
      end
    end
    else begin
      xb_dat_o[35:32] <= 4'h0;
      xb_dat_o[31:0] <= 32'h0;
      if (data_cap) begin
        data_cap <= 1'b0;
        ostate <= WAIT_ACK;
      end
    end
  end

WAIT_ACK:
  begin
    // If it was a read cycle send the data back
    if (!was_write) begin
      if (selh[3:0]) begin
        xb_dat_o[35:32] <= 4'h4;
        xb_dat_o[31:0] <= dath[31:0];
        ostate <= XD32_63;
      end
      else if (selh[7:4]) begin
        xb_dat_o[35:32] <= 4'h5;
        xb_dat_o[31:0] <= dath[63:32];
        ostate <= XD64_95;
      end
      else if (selh[11:8]) begin
        xb_dat_o[35:32] <= 4'h6;
        xb_dat_o[31:0] <= dath[95:64];
        ostate <= XD96_127;
      end
      else if (selh[15:12]) begin
        xb_dat_o[35:32] <= 4'h7;
        xb_dat_o[31:0] <= dath[127:96];
        ostate <= WAIT_NACK;
      end
      else
        ostate <= WAIT_NACK;
    end
    else begin
      xb_dat_o[35:32] <= 4'h3;
      xb_dat_o[31:0] <= 32'h0;
      xb_dat_o[30] <= 1'b1; // Send read ack / tran complete
      xb_dat_o[29:24] <= dev_num_i;
      xb_dat_o[5:0] <= master_num;
      ostate <= IDLE;
    end
  end
XD32_63:
  begin
    if (selh[7:4]) begin
      xb_dat_o[35:32] <= 4'h5;
      xb_dat_o[31:0] <= dath[63:32];
      ostate <= XD64_95;
    end
    else if (selh[11:8]) begin
      xb_dat_o[35:32] <= 4'h6;
      xb_dat_o[31:0] <= dath[95:64];
      ostate <= XD96_127;
    end
    else if (selh[15:12]) begin
      xb_dat_o[35:32] <= 4'h7;
      xb_dat_o[31:0] <= dath[127:96];
      ostate <= WAIT_NACK;
    end
    else
      ostate <= WAIT_NACK;
  end
XD64_95:
  begin
    if (selh[11:8]) begin
      xb_dat_o[35:32] <= 4'h6;
      xb_dat_o[31:0] <= dath[95:64];
      ostate <= XD96_127;
    end
    else if (selh[15:12]) begin
      xb_dat_o[35:32] <= 4'h7;
      xb_dat_o[31:0] <= dath[127:96];
      ostate <= WAIT_NACK;
    end
    else
      ostate <= WAIT_NACK;
  end
XD96_127:
  begin
    ostate <= WAIT_NACK;
    if (selh[15:12]) begin
      xb_dat_o[35:32] <= 4'h7;
      xb_dat_o[31:0] <= dath[127:96];
    end
  end
WAIT_NACK:
  begin
    ostate <= IDLE;
    xb_dat_o[35:32] <= 4'h3;
    xb_dat_o[31:0] <= 32'h0;
    xb_dat_o[30] <= 1'b1; // Send read ack / tran complete
    xb_dat_o[29:24] <= dev_num_i;
    xb_dat_o[5:0] <= master_num;
  end
endcase

end

endmodule
