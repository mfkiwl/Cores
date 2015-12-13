
module Thor_tb();
parameter DBW=32;
reg rst;
reg clk;
reg nmi;
wire [2:0] cti;
wire cpu_clk;
wire cyc;
wire stb;
wire we;
wire [7:0] sel;
wire br_ack;
wire [31:0] adr;
wire [DBW-1:0] br_dato;
wire scr_ack;
wire [63:0] scr_dato;

wire cpu_ack;
wire [DBW-1:0] cpu_dati;
wire [DBW-1:0] cpu_dato;
wire pic_ack,irq;
wire [31:0] pic_dato;
wire [7:0] vecno;

wire LEDS_ack;

initial begin
	#0 rst = 1'b0;
	#0 clk = 1'b0;
	#0 nmi = 1'b0;
	#10 rst = 1'b1;
	#50 rst = 1'b0;
	#19550 nmi = 1'b1;
	#20 nmi = 1'b0;
end

always #5 clk = ~clk;

assign LEDS_ack = cyc && stb && adr[31:8]==32'hFFDC06;
always @(posedge clk)
	if (LEDS_ack)
		$display("LEDS: %b", cpu_dato[7:0]);

wire tc1_ack, tc2_ack;
wire kbd_ack;
wire [31:0] tc1_dato, tc2_dato;
wire [7:0] kbd_dato;

//wire cs0 = cyc&& stb && adr[31:16]==16'h0000;

assign cpu_ack =
	LEDS_ack |
	scr_ack |
	br_ack |
	tc1_ack | tc2_ack |
	kbd_ack | pic_ack
	;
assign cpu_dati =
	scr_dato |
	br_dato |
	tc1_dato | tc2_dato |
	{4{kbd_dato}} |
	pic_dato
	;

Ps2Keyboard_sim ukbd
(
    .rst_i(rst),
    .clk_i(cpu_clk),
    .cyc_i(cyc),
    .stb_i(stb),
    .ack_o(kbd_ack),
    .we_i(we),
    .adr_i(adr),
    .dat_i(cpu_dato),
    .dat_o(kbd_dato),
    .kclk(),
    .kd(),
    .irq_o()
);

rtfTextController3 #(.num(1), .pTextAddress(32'hFFD00000))  tc1
(
	.rst_i(rst),
	.clk_i(cpu_clk),
	.cyc_i(cyc),
	.stb_i(stb),
	.ack_o(tc1_ack),
	.we_i(we),
	.adr_i(adr),
	.dat_i(cpu_dato),
	.dat_o(tc1_dato),
	.lp(),
	.curpos(),
	.vclk(),
	.hsync(),
	.vsync(),
	.blank(),
	.border(),
	.rgbIn(),
	.rgbOut()
);

rtfTextController3 #(.num(1), .pTextAddress(32'hFFD10000))  tc2
(
	.rst_i(rst),
	.clk_i(cpu_clk),
	.cyc_i(cyc),
	.stb_i(stb),
	.ack_o(tc2_ack),
	.we_i(we),
	.adr_i(adr),
	.dat_i(cpu_dato),
	.dat_o(tc2_dato),
	.lp(),
	.curpos(),
	.vclk(),
	.hsync(),
	.vsync(),
	.blank(),
	.border(),
	.rgbIn(),
	.rgbOut()
);

scratchmem32 #(DBW) uscrm1
(
	.rst_i(rst),
	.clk_i(cpu_clk),
	.cyc_i(cyc),
	.stb_i(stb),
	.ack_o(scr_ack),
	.we_i(we),
	.sel_i(sel),
	.adr_i({32'd0,adr}),
	.dat_i(cpu_dato),
	.dat_o(scr_dato)
);

bootrom #(DBW) ubr1
(
	.rst_i(rst),
	.clk_i(cpu_clk),
	.cti_i(cti),
	.cyc_i(cyc),
	.stb_i(stb),
	.ack_o(br_ack),
	.adr_i(adr),
	.dat_o(br_dato),
	.perr()
);

wire nmio;
Thor_pic upic1
(
	.rst_i(rst),		// reset
	.clk_i(cpu_clk),	// system clock
	.cyc_i(cyc),	// cycle valid
	.stb_i(stb),	// strobe
    .ack_o(pic_ack),	// transfer acknowledge
	.we_i(we),		// write
	.adr_i(adr),	// address
	.dat_i(cpu_dato),
	.dat_o(pic_dato),
	.vol_o(),		// volatile register selected
	.i1(),
	.i2(),
	.i3(),
	.i4(),
	.i5(),
	.i6(),
	.i7(),
	.i8(),
	.i9(),
	.i10(),
	.i11(),
	.i12(),
	.i13(),
	.i14(),
	.i15(),
	.irqo(irq),	// normally connected to the processor irq
	.nmii(nmi),		// nmi input connected to nmi requester
	.nmio(nmio),	// normally connected to the nmi of cpu
	.vecno(vecno)
);

Thor #(DBW) uthor1
(
	.rst_i(rst),
	.clk_i(clk),
	.clk_o(cpu_clk),
	.nmi_i(nmio),
	.irq_i(1'b0),
	.vec_i(8'h00),
	.bte_o(),
	.cti_o(cti),
	.bl_o(),
	.cyc_o(cyc),
	.stb_o(stb),
	.ack_i(cpu_ack),
	.err_i(1'b0),
	.we_o(we),
	.sel_o(sel),
	.adr_o(adr),
	.dat_i(cpu_dati),
	.dat_o(cpu_dato)
);

endmodule
