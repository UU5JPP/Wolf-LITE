set_time_format -unit ns -decimal_places 3

create_clock -name "clk_sys" -period 64.32MHz [get_ports {clk_sys}]
create_clock -name "clock_stm32" -period 25MHz [get_ports {STM32_CLK}]
create_clock -name "iq_valid" -period 48KHz {rx_ciccomp:RX1_CICOMP_Q|rx_ciccomp_0002:rx_ciccomp_inst|rx_ciccomp_0002_ast:rx_ciccomp_0002_ast_inst|auk_dspip_avalon_streaming_source_hpfir:source|data_valid}

set_clock_groups -asynchronous -group { clock_sys clock_stm32 iq_valid }

derive_clock_uncertainty

derive_pll_clocks -create_base_clocks

set_output_delay -clock clock_crystal -max 36ps [get_ports {DAC_OUTPUT[*]}]
set_output_delay -clock clock_crystal -min 0ps [get_ports {DAC_OUTPUT[*]}]
set_output_delay -clock clock_stm32 -max 36ps [get_ports {STM32_DATA_BUS[*]}]
set_output_delay -clock clock_stm32 -min 0ps [get_ports {STM32_DATA_BUS[*]}]

set_input_delay -clock clock_adc -max 36ps [get_ports ADC_INPUT[*]]
set_input_delay -clock clock_adc -min 0ps [get_ports ADC_INPUT[*]]
set_input_delay -clock clock_adc -max 36ps [get_ports ADC_OTR]
set_input_delay -clock clock_adc -min 0ps [get_ports ADC_OTR]
set_input_delay -clock clock_stm32 -max 36ps [get_ports STM32_DATA_BUS[*]]
set_input_delay -clock clock_stm32 -min 0ps [get_ports STM32_DATA_BUS[*]]
set_input_delay -clock clock_stm32 -max 36ps [get_ports STM32_SYNC]
set_input_delay -clock clock_stm32 -min 0ps [get_ports STM32_SYNC]

set_multicycle_path -from [get_clocks {clock_stm32}] -to [get_clocks {iq_valid}] -setup -end 2
set_multicycle_path -from [get_clocks {clock_stm32}] -to [get_clocks {iq_valid}] -hold -end 2
set_multicycle_path -from [get_clocks {iq_valid}] -to [get_clocks {clock_stm32}] -setup -end 2
set_multicycle_path -from [get_clocks {iq_valid}] -to [get_clocks {clock_stm32}] -hold -end 2
set_multicycle_path -from [get_clocks {clock_stm32}] -to [get_clocks {clock_sys}] -setup -end 2
set_multicycle_path -from [get_clocks {clock_stm32}] -to [get_clocks {clock_sys}] -hold -end 2
