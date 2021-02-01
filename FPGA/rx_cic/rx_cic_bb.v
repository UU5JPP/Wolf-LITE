
module rx_cic (
	in_error,
	in_valid,
	in_ready,
	in_data,
	out_data,
	out_error,
	out_valid,
	out_ready,
	clken,
	clk,
	reset_n);	

	input	[1:0]	in_error;
	input		in_valid;
	output		in_ready;
	input	[22:0]	in_data;
	output	[85:0]	out_data;
	output	[1:0]	out_error;
	output		out_valid;
	input		out_ready;
	input		clken;
	input		clk;
	input		reset_n;
endmodule
