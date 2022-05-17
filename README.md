# Wolf-LITE
DUC-DDC Трансивер Wolf-Lite
Community telegram channel: https://t.me/Wolf_lite

## Principle of operation
The RF signal is digitized by a high-speed ADC chip and fed to an FPGA processor.
It performs DDC / DUC conversion (digital frequency shift down or up the spectrum) - by analogy with a direct conversion receiver.
The I and Q quadrature signals from the conversions are fed to the STM32 microprocessor.
It filters, (de) modulates and outputs audio to an audio codec / USB. It also handles the entire user interface.
When transmitting, the process occurs in the opposite order, only at the end of the chain there is a DAC, which converts the digital signal back to analog RF.

## Specifications
<li>Receiving frequencies: 0 MHz - 55 MHz</li>
<li>Transmission frequencies: 0 MHz - 30 MHz</li>
<li>TX power : 20W</li>
<li>Modulation types (TX / RX): CW, LSB, USB, AM, FM, DIGI</li>
<li>Preamplifier</li>
<li>Adjustable attenuator 0-31dB</li>
<li>Band pass filters</li>
<li>ADC dynamic range (12 bit) ~74dB</li>
<li>Supply voltage: 13.8V (overvoltage and polarity reversal NOT protection)</li>
<li>Consumption current when receiving: 350-800mA (DC-DC or linear) </li>
<li>Current consumption during transmission: ~5А</li>

## Transceiver Features
<li>Panorama (spectrum + waterfall) up to 48 kHz wide</li>
<li>Panorama tweaks and themes</li>
<li>Adjustable bandwidth: HPF from 0Hz to 600Hz, LPF from 100Hz to 20kHz</li>
<li>Integrated SWR/power meter (HF)</li>
<li>Automatic and manual Notch filter</li>
<li>Switchable AGC (AGC) with adjustable attack rate</li>
<li>Range map, with the ability to automatically switch modes</li>
<li>CAT virtual COM port (FT-450 emulation, RTS - PTT, DTR - CW)</li>
<li>USB operation (audio transmission, CAT, KEY, PTT)</li>
<li>SWR Graphs</li>
<li>Spectrum analyzer</li>
<li>Equalizer TX/RX, reverber</li>
<li>AGC takes into account the characteristics of human hearing (K-Weighting)</li>
<li>TCXO frequency stabilization (it is possible to use an external clock source, such as GPS)</li>
<li>Tangent support Yaesu MH-36 и MH-48</li>
<li>And other.. (see menu)</li>
<li>Sensitivity</li>


<br><a href="https://imgbb.com/"><img src="https://i.ibb.co/1Z7yGWk/2021-11-22-17-33-03.png" alt="2021-11-22-17-33-03" border="0"></a>
<a href="https://ibb.co/n6djp4J"><img src="https://i.ibb.co/BcWBkvM/photo-2021-04-25-09-53-57.jpg" alt="photo-2021-04-25-09-53-57" border="0"></a>
<a href="https://ibb.co/PWt7MX3"><img src="https://i.ibb.co/JpCfq60/photo-2021-11-22-17-40-02.jpg" alt="photo-2021-11-22-17-40-02" border="0"></a>
