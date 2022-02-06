module stm32_interface(
clk_in,
RX_I,
RX_Q,
DATA_SYNC,
ADC_OTR,
DAC_OTR,
ADC_IN,
adcclk_in,
FLASH_data_in,
FLASH_busy,
IQ_valid,

DATA_BUS,
NCO_freq,
preamp_enable,
rx,
tx,
TX_I,
TX_Q,
reset_n,
stage_debug,
FLASH_data_out,
FLASH_enable,
FLASH_continue_read,
CIC_GAIN,
CICFIR_GAIN,
TX_CICFIR_GAIN,
DAC_GAIN,
tx_iq_valid,
TX_NCO_freq,
ATT_05,
ATT_1,
ATT_2,
ATT_4,
ATT_8,
ATT_16,
BPF_A,
BPF_B,
BPF_OE1,
BPF_OE2,
LPF_1,
LPF_2,
LPF_3,
VCXO_correction
);

input clk_in;
input signed [15:0] RX_I;
input signed [15:0] RX_Q;
input DATA_SYNC;
input ADC_OTR;
input DAC_OTR;
input signed [11:0] ADC_IN;
input adcclk_in;
input unsigned [7:0] FLASH_data_in;
input FLASH_busy;
input IQ_valid;

output reg unsigned [21:0] NCO_freq = 242347;
output reg unsigned [21:0] TX_NCO_freq = 242347;
output reg preamp_enable = 0;
output reg rx = 1;
output reg tx = 0;
output reg reset_n = 1;
output reg signed [15:0] TX_I = 'd0;
output reg signed [15:0] TX_Q = 'd0;
output reg [15:0] stage_debug = 0;
output reg unsigned [7:0] FLASH_data_out = 0;
output reg FLASH_enable = 0;
output reg FLASH_continue_read = 0;
output reg unsigned [7:0] CIC_GAIN = 32;
output reg unsigned [7:0] CICFIR_GAIN = 32;
output reg unsigned [7:0] TX_CICFIR_GAIN = 32;
output reg unsigned [7:0] DAC_GAIN = 32;
output reg tx_iq_valid = 0;
output reg ATT_05 = 0;
output reg ATT_1 = 0;
output reg ATT_2 = 0;
output reg ATT_4 = 0;
output reg ATT_8 = 0;
output reg ATT_16 = 0;
output reg BPF_A = 0;
output reg BPF_B = 0;
output reg BPF_OE1 = 0;
output reg BPF_OE2 = 0;
output reg LPF_1 = 0;
output reg LPF_2 = 0;
output reg LPF_3 = 0;
output reg unsigned [15:0] VCXO_correction = 32767;

inout [7:0] DATA_BUS;
reg   [7:0] DATA_BUS_OUT;
reg         DATA_BUS_OE; // 1 - out 0 - in
assign DATA_BUS = DATA_BUS_OE ? DATA_BUS_OUT : 8'bZ ;

parameter rx_buffer_length = (8 - 1);
reg signed [15:0] BUFFER_RX_I [0:rx_buffer_length];
reg signed [15:0] BUFFER_RX_Q [0:rx_buffer_length];
reg signed [15:0] BUFFER_RX_head = 'd0;
reg signed [15:0] BUFFER_RX_tail = 'd0;
reg signed [15:0] k = 'd1;
reg signed [15:0] REG_RX_I;
reg signed [15:0] REG_RX_Q;
reg signed [15:0] I_HOLD;
reg signed [15:0] Q_HOLD;
reg signed [11:0] ADC_MIN;
reg signed [11:0] ADC_MAX;
reg ADC_MINMAX_RESET;
reg sync_reset_n = 1;

always @ (posedge IQ_valid)
begin
	BUFFER_RX_I[BUFFER_RX_head][15:0] = RX_I[15:0];
	BUFFER_RX_Q[BUFFER_RX_head][15:0] = RX_Q[15:0];
	if(BUFFER_RX_head >= rx_buffer_length)
		BUFFER_RX_head = 0;
	else
		BUFFER_RX_head = BUFFER_RX_head + 16'd1;
end

always @ (posedge clk_in)
begin
	//начало передачи
	if (DATA_SYNC == 1)
	begin
		DATA_BUS_OE = 0;
		ADC_MINMAX_RESET = 0;
		FLASH_continue_read = 0;
		
		if(DATA_BUS[7:0] == 'd0) //BUS TEST
		begin
			k = 500;
		end
		else if(DATA_BUS[7:0] == 'd1) //GET PARAMS
		begin
			k = 100;
		end
		else if(DATA_BUS[7:0] == 'd2) //SEND PARAMS
		begin
			DATA_BUS_OE = 1;
			k = 200;
		end
		else if(DATA_BUS[7:0] == 'd3) //TX IQ
		begin
			tx_iq_valid = 0;
			k = 300;
		end
		else if(DATA_BUS[7:0] == 'd4) //RX IQ
		begin
			DATA_BUS_OE = 1;
			k = 400;
		end
		else if(DATA_BUS[7:0] == 'd5) //RESET ON
		begin
			sync_reset_n = 0;
			k = 999;
		end
		else if(DATA_BUS[7:0] == 'd6) //RESET OFF
		begin
			sync_reset_n = 1;
			k = 999;
		end
		else if(DATA_BUS[7:0] == 'd7) //FPGA FLASH READ
		begin
			FLASH_enable = 0;
			k = 700;
		end
	end
	else if (k == 100) //GET PARAMS
	begin
		rx = DATA_BUS[0:0];
		tx = !rx;
		preamp_enable = DATA_BUS[1:1];
		ATT_05 = DATA_BUS[2:2];
		ATT_1 = DATA_BUS[3:3];
		ATT_2 = DATA_BUS[4:4];
		ATT_4 = DATA_BUS[5:5];
		ATT_8 = DATA_BUS[6:6];
		ATT_16 = DATA_BUS[7:7];	
		k = 101;
	end
	else if (k == 101)
	begin	
		NCO_freq[21:16] = DATA_BUS[5:0];
		k = 102;
	end
	else if (k == 102)
	begin
		NCO_freq[15:8] = DATA_BUS[7:0];
		k = 103;
	end
	else if (k == 103)
	begin
		NCO_freq[7:0] = DATA_BUS[7:0];
		k = 104;
	end
	else if (k == 104)
	begin
		CIC_GAIN[7:0] = DATA_BUS[7:0];
		k = 105;
	end
	else if (k == 105)
	begin
		CICFIR_GAIN[7:0] = DATA_BUS[7:0];
		k = 106;
	end
	else if (k == 106)
	begin
		TX_CICFIR_GAIN[7:0] = DATA_BUS[7:0];
		k = 107;
	end
	else if (k == 107)
	begin
		DAC_GAIN[7:0] = DATA_BUS[7:0];
		k = 108;
	end
	else if (k == 108)
	begin
		TX_NCO_freq[21:16] = DATA_BUS[5:0];
		k = 109;
	end
	else if (k == 109)
	begin
		TX_NCO_freq[15:8] = DATA_BUS[7:0];
		k = 110;
	end
	else if (k == 110)
	begin
		TX_NCO_freq[7:0] = DATA_BUS[7:0];
		k = 111;
	end
	else if (k == 111)
	begin
		BPF_A = DATA_BUS[0:0];
		BPF_B = DATA_BUS[1:1];
		BPF_OE1 = DATA_BUS[2:2];
		BPF_OE2 = DATA_BUS[3:3];
		LPF_1 = DATA_BUS[4:4];
		LPF_2 = DATA_BUS[5:5];
		LPF_3 = DATA_BUS[6:6];
		k = 112;
	end		
	else if (k == 112)
	begin
		VCXO_correction[15:8] = DATA_BUS[7:0];
		k = 113;
	end	
	else if (k == 113)
	begin
		VCXO_correction[7:0] = DATA_BUS[7:0];
		k = 999;
	end	
	else if (k == 200) //SEND PARAMS
	begin
		DATA_BUS_OUT[0:0] = ADC_OTR;
		DATA_BUS_OUT[1:1] = DAC_OTR;
		k = 201;
	end
	else if (k == 201)
	begin
		DATA_BUS_OUT[3:0] = ADC_MIN[11:8];
		k = 202;
	end
	else if (k == 202)
	begin
		DATA_BUS_OUT[7:0] = ADC_MIN[7:0];
		k = 203;
	end
	else if (k == 203)
	begin
		DATA_BUS_OUT[3:0] = ADC_MAX[11:8];
		k = 204;
	end
	else if (k == 204)
	begin
		DATA_BUS_OUT[7:0] = ADC_MAX[7:0];
		ADC_MINMAX_RESET=1;
		k = 999;
	end
	else if (k == 300) //TX IQ
	begin
		Q_HOLD[15:8] = DATA_BUS[7:0];
		k = 301;
	end
	else if (k == 301)
	begin
		Q_HOLD[7:0] = DATA_BUS[7:0];
		k = 302;
	end
	else if (k == 302)
	begin
		I_HOLD[15:8] = DATA_BUS[7:0];
		k = 303;
	end
	else if (k == 303)
	begin
		I_HOLD[7:0] = DATA_BUS[7:0];
		TX_I[15:0] = I_HOLD[15:0];
		TX_Q[15:0] = Q_HOLD[15:0];
		tx_iq_valid = 1;
		k = 999;
	end
	else if (k == 400) //RX IQ
	begin
		if(BUFFER_RX_tail == BUFFER_RX_head) //догнал буффер
		begin	
			REG_RX_I[15:0] = 'd0;
			REG_RX_Q[15:0] = 'd0;
		end
		else
		begin
			REG_RX_I[15:0] = BUFFER_RX_I[BUFFER_RX_tail][15:0];
			REG_RX_Q[15:0] = BUFFER_RX_Q[BUFFER_RX_tail][15:0];
			
			if(BUFFER_RX_tail >= rx_buffer_length)
				BUFFER_RX_tail = 0;
			else
				BUFFER_RX_tail = BUFFER_RX_tail + 16'd1;
		end
		
		I_HOLD = REG_RX_I;
		Q_HOLD = REG_RX_Q;
		
		DATA_BUS_OUT[7:0] = Q_HOLD[15:8];
		k = 401;
	end
	else if (k == 401)
	begin
		DATA_BUS_OUT[7:0] = Q_HOLD[7:0];
		k = 402;
	end
	else if (k == 402)
	begin
		DATA_BUS_OUT[7:0] = I_HOLD[15:8];
		k = 403;
	end
	else if (k == 403)
	begin
		DATA_BUS_OUT[7:0] = I_HOLD[7:0];
		k = 999;
	end
	else if (k == 500) //BUS TEST
	begin
		Q_HOLD[7:0] = DATA_BUS[7:0];
		DATA_BUS_OE = 1;
		DATA_BUS_OUT[7:0] = Q_HOLD[7:0];
		k = 999;
	end
	else if (k == 700) //FPGA FLASH READ - SEND COMMAND
	begin
		DATA_BUS_OE = 0;
		FLASH_data_out[7:0] = DATA_BUS[7:0];
		if(FLASH_enable == 0)
			FLASH_enable = 1;
		else
			FLASH_continue_read = 1;
		k = 701;
	end
	else if (k == 701) //FPGA FLASH READ - READ ANSWER
	begin
		FLASH_continue_read = 0;
		DATA_BUS_OE = 1;
		if(FLASH_busy)
			DATA_BUS_OUT[7:0] = 'd255;
		else
			DATA_BUS_OUT[7:0] = FLASH_data_in[7:0];
		k = 700;
	end
	stage_debug=k;
end

always @ (posedge adcclk_in)
begin
	//ADC MIN-MAX
	if(ADC_MINMAX_RESET == 1)
	begin
		ADC_MIN = 'd2000;
		ADC_MAX = -12'd2000;
	end
	if(ADC_MAX<ADC_IN)
	begin
		ADC_MAX=ADC_IN;
	end
	if(ADC_MIN>ADC_IN)
	begin
		ADC_MIN=ADC_IN;
	end
end

always @ (negedge adcclk_in)
begin
	//RESET SYNC
	reset_n = sync_reset_n;
end

endmodule
