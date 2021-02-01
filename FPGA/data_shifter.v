module data_shifter(
	input wire [(in_width-1):0] data_in_I,
	input wire data_valid_I,
	input wire [(in_width-1):0] data_in_Q,
	input wire data_valid_Q,
	input unsigned [7:0] distance,
	input wire enabled,

	output wire [(out_width-1):0] data_out_I,
	output wire	data_valid_out_I,
	output wire [(out_width-1):0] data_out_Q,
	output wire data_valid_out_Q
);

parameter in_width = 88;
parameter out_width = 32;

assign data_valid_out_I = enabled ? data_valid_I : 0;
assign data_valid_out_Q = enabled ? data_valid_Q : 0;
assign data_out_I[(out_width-1):0] = enabled ? data_in_I[(distance-1) -: out_width] : 0;
assign data_out_Q[(out_width-1):0] = enabled ? data_in_Q[(distance-1) -: out_width] : 0;

endmodule
