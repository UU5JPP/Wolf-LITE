module spi_interface(
clk_in,
enabled,
data_in,
continue_read,
MISO_DQ1,

data_out,
MOSI_DQ0,
SCK_C,
CS_S,
busy,
spi_stage
);

input clk_in;
input enabled;
input unsigned [7:0] data_in;
input MISO_DQ1;
input continue_read;

output reg unsigned [7:0] data_out = 1;
output reg MOSI_DQ0 = 1;
output reg SCK_C = 1;
output reg CS_S = 1;
output reg busy = 0;
output reg unsigned [7:0] spi_stage = 0;

reg unsigned [7:0] spi_bit_position = 7;
reg continue_read_prev = 0;
reg enabled_prev = 0;

//CPOL = 0 CHPA = 0
always @ (posedge clk_in)
begin	
	if(enabled == 1)
	begin	
		if(spi_stage == 0) //начинаем передачу
		begin
			busy <= 1;
			CS_S <= 0;
			SCK_C <= 0; //1
			spi_bit_position <= 7;
			spi_stage <= 1;
		end
		else if(spi_stage == 1) //отсылаем бит
		begin
			busy <= 1;
			MOSI_DQ0 <= data_in[spi_bit_position];
			spi_stage <= 2;
			continue_read_prev <= 1;
		end
		else if(spi_stage == 2) //поднимаем фронт
		begin
			busy <= 1;
			spi_stage <= 3;
			SCK_C <= 1;
		end
		else if(spi_stage == 3) //принимаем ответ
		begin
			busy <= 1;
			spi_stage <= 4;
			data_out[spi_bit_position] <= MISO_DQ1;
		end
		else if(spi_stage == 4) //снижаем фронт и переключаем бит
		begin
			SCK_C <= 0;
			busy <= 1;
			
			if(spi_bit_position == 0)
			begin
				spi_stage <= 99;
				busy <= 0;
			end
			else
			begin
				spi_bit_position <= spi_bit_position - 8'd1;
				spi_stage <= 1;
			end
		end		
		enabled_prev <= 1;
	end
	else
	begin	
		SCK_C <= 0;
		CS_S <= 1;
		MOSI_DQ0 <= 1;
		spi_bit_position <= 7;
		spi_stage <= 0;
		busy <= 0;
		enabled_prev <= 0;
	end
	
	//continue read
	if(continue_read_prev == 0 && continue_read == 1)
	begin
		spi_stage <= 1;
		spi_bit_position <= 7;
	end
	if(continue_read == 0)
	begin
		continue_read_prev <= 0;
	end
	
	//reset command
	if(enabled_prev == 0 && enabled == 1)
	begin
		spi_stage <= 0;
	end
end

endmodule
