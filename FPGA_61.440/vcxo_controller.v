module vcxo_controller(
vcxo_clk_in,
tcxo_clk_in,
VCXO_correction,

freq_error,
pump,
PWM
);

parameter VCXO_freq_khz = 1228800; //x100hz
parameter TCXO_freq_khz = 122880; //x100hz

input vcxo_clk_in;
input tcxo_clk_in;
input signed [7:0] VCXO_correction;

output reg signed [23:0] freq_error = 0;
output reg pump = 0;
output reg signed [23:0] PWM = 500;

reg [23:0] PWM_max = 1000;
reg signed [23:0] freq_error_now = 0;
reg signed [23:0] freq_error_prev = 0;
reg signed [23:0] freq_error_diff = 0;
reg signed [31:0] VCXO_counter = 0;
reg signed [31:0] TCXO_counter = 0;
reg [31:0] PWM_counter = 0;
reg counter_reset = 0;
reg counter_resetted = 0;
reg [7:0] state = 0;

always @ (posedge vcxo_clk_in)
begin
	if(state != 2 && state != 3)
	begin
		if(counter_reset)
		begin
			VCXO_counter <= 0;
			counter_resetted <= 1;
		end
		else
		begin
			VCXO_counter <= VCXO_counter + 1;
			counter_resetted <= 0;
		end
	end
end

always @ (posedge tcxo_clk_in)
begin
	//do PWM
	PWM_counter = PWM_counter + 1;
	if(PWM_counter >= PWM_max)
		PWM_counter = 0;
	
	if(PWM > PWM_counter)
		pump = 1;
	else
		pump = 0;

	if(counter_reset && !counter_resetted)
	begin
		//wait VCXO reset
	end
	else
	begin
		if(state == 0)
		begin
			TCXO_counter = 0;
			counter_reset = 0;
			state = 1;
		end
		else if(state == 1)
		begin
			TCXO_counter = TCXO_counter + 1;
			
			if(TCXO_counter >= TCXO_freq_khz)
				state = 2;
		end
		else if(state == 2)
		begin
			freq_error_now = VCXO_counter - VCXO_freq_khz + VCXO_correction;	
			freq_error_diff = freq_error_prev - freq_error_now;
			
			if(freq_error_diff > -50 && freq_error_diff < 50) //measure errors
			if(freq_error_now > -1000 && freq_error_now < 1000) //measure errors
			begin
				//save
				freq_error = freq_error_now;
				
				//tune
				if(freq_error_now < 0)
					PWM = PWM + 1;
				else if(freq_error_now > 0)
					PWM = PWM - 1;
			end

			state = 3;
		end
		else if(state == 3)
		begin
			freq_error_prev = freq_error_now;
			if(PWM > PWM_max)
				PWM = PWM_max;
			if(PWM < 0)
				PWM = 0;
			counter_reset = 1;
			state = 0;
		end
	end
end

endmodule
