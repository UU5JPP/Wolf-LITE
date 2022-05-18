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
<li>Transverter funkction</li>
<li>AGC takes into account the characteristics of human hearing (K-Weighting)</li>
<li>Tangent support Yaesu MH-36 и MH-48</li>
<li>And other.. (see menu)</li>


# Трансивер "Волк-Лайт"
Телеграм канал сообщества: https://t.me/Wolf_lite

## Принцип работы
ВЧ сигнал оцифровывается высокоскоростной микросхемой АЦП, и подаётся на FPGA процессор.
В нём происходит DDC/DUC преобразование (цифровое смещение частоты вниз или вверх по спектру) - по аналогии с приёмником прямого преобразования.
I и Q квадратурные сигналы, полученные в ходе преобразований, поступают на микропроцессор STM32.
В нём происходит фильтрация, (де)модуляция и вывод звука на аудио-кодек/USB. Также он обрабатывает весь пользовательский интерфейс.
При передаче процесс происходит в обратном порядке, только в конце цепочки стоит ЦАП, преобразующий цифровой сигнал обратно в аналоговый ВЧ.

## Технические характеристики
<li>Частоты приёма: 0 MHz - 55 MHz</li>
<li>Частоты передачи: 0 MHz - 30 MHz</li>
<li>Мощность TX 20W</li>
<li>Виды модуляции (TX/RX): CW, LSB, USB, AM, FM, DIGI</li>
<li>Предусилитель</li>
<li>Регулируемый аттенюатор на 0-31дБ</li>
<li>Полосовые фильтры</li>
<li>Динамический диапазон АЦП (12 бит) ~74дБ</li>
<li>Напряжение питания: 13.8в (защита от перенапряжения и смены полярности нет)</li>
<li>Потребляемый ток при приёме: ~350-800mA (DC-DC или линей стабилизатор)</li>
<li>Потребляемый ток при передаче: ~5А</li>

## Функции трансивера
<li>Панорама (спектр+водопад) шириной до 48 кГц</li>
<li>Несколько видов оформления спектра</li>
<li>Регулируемая полоса пропускания: ФВЧ от 0гц до 600гц, ФНЧ от 100гц до 20кГц</li>
<li>Встроенный КСВ/Power метр (КВ)</li>
<li>Автоматический и ручной Notch фильтр</li>
<li>Отключаемое АРУ (AGC) с регулируемой скоростью атаки</li>
<li>Карта диапазонов, с возможностью автоматического переключения моды</li>
<li>CAT виртуальный COM-порт (эмуляция FT-450, RTS - PTT, DTR - CW)</li>
<li>Работа по USB (передача звука, CAT, KEY, PTT)</li>
<li>Построение графиков КСВ по диапазонам</li>
<li>Анализатор спектра</li>
<li>Эквалайзер TX/RX, ревербератор</li>
<li>AGC учитывает особенности человеческого слуха (K-Weighting)</li>
<li>Режим трансвертера</li>
<li>Поддержка тангент Yaesu MH-36 и MH-48</li>
<li>И другое (см. работу с меню)</li>


<br><a href="https://imgbb.com/"><img src="https://i.ibb.co/1Z7yGWk/2021-11-22-17-33-03.png" alt="2021-11-22-17-33-03" border="0"></a>
<a href="https://ibb.co/n6djp4J"><img src="https://i.ibb.co/BcWBkvM/photo-2021-04-25-09-53-57.jpg" alt="photo-2021-04-25-09-53-57" border="0"></a>
<a href="https://ibb.co/PWt7MX3"><img src="https://i.ibb.co/JpCfq60/photo-2021-11-22-17-40-02.jpg" alt="photo-2021-11-22-17-40-02" border="0"></a>
