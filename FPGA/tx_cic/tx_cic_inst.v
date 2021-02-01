	tx_cic u0 (
		.in_error  (<connected-to-in_error>),  //  av_st_in.error
		.in_valid  (<connected-to-in_valid>),  //          .valid
		.in_ready  (<connected-to-in_ready>),  //          .ready
		.in_data   (<connected-to-in_data>),   //          .in_data
		.out_data  (<connected-to-out_data>),  // av_st_out.out_data
		.out_error (<connected-to-out_error>), //          .error
		.out_valid (<connected-to-out_valid>), //          .valid
		.out_ready (<connected-to-out_ready>), //          .ready
		.clk       (<connected-to-clk>),       //     clock.clk
		.reset_n   (<connected-to-reset_n>)    //     reset.reset_n
	);

