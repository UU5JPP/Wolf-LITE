module DAC_corrector(
clk_in,
DATA_IN,
distance,

DATA_OUT
);

parameter in_width = 32;
parameter out_width = 14;

input clk_in;
input signed [31:0] DATA_IN;
input unsigned [7:0] distance;

output reg unsigned [13:0] DATA_OUT;

reg signed [13:0] tmp=0;

always @ (posedge clk_in)
begin
	//получаем 14 бит
	if (distance<out_width)
	begin
		tmp[(out_width-1):0] = DATA_IN[(out_width-1):0];
	end
	if (distance>in_width)
	begin
		tmp[(out_width-1):0] = DATA_IN[(in_width-1) -: out_width];
	end
	else
	begin
		tmp[(out_width-1):0] = DATA_IN[(distance-1) -: out_width];
	end

	DATA_OUT[(out_width-1):0]={~tmp[(out_width-1)],tmp[(out_width-2):0]}; //инвертируем первый бит, получая unsigned из two's complement
end


endmodule
