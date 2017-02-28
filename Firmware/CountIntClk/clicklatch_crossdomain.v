// Fast Multisource Pulse Registration System
// Module:
// clicklatch_crossdomain
// Pulse Edge Detection
// (c) Jakob van Santen 2017-forever
module clicklatch_crossdomain (
   input level,
	input clock,
	input clock_ext,
	output data );


// reg clicked;
wire clicked;
reg prev_level;
reg toggle;
reg [2:0] sync;

initial begin
	toggle = 1'b0;
end

// detect rising edge in input
assign clicked = level == 1'b1 && prev_level == 1'b0;
 
always @ (posedge clock)
begin
	prev_level <= level;
	// record state change
	toggle <= clicked ^ toggle;
end
 
// shift the strobe into the slower clock domain
always @ (posedge clock_ext) sync <= {sync[1:0], toggle};

// reproduce the `clicked` strobe in the slower clock domain
// `data` is high if there were one (or more!) edge crossings
// in the last external clock cycle
assign data = (sync[2] ^ sync[1]);

endmodule