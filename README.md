# nrf_node
A nRF24L01-based firmware to handle data collection, transmission, and basic self-diagnostics. 

Communicates with a series of other nodes and one nrf_bridge in a star configuration. Node collects and transmits data from the field. Bridge acts as a relay between remote nodes and an IP-enabled processing device. Based on a STM32030F4 microcontroller. 

[Hardware schematic](https://github.com/cms-/nrf_node/blob/master/node_revb.pdf)

## Project notes

[MCU pin assignment and basic configuration](https://github.com/cms-/nrf_node/blob/master/004.jpg)

[Battery management circuit and component selection; mic frontend](https://github.com/cms-/nrf_node/blob/master/005.jpg)

[Buck/Boost regulator (RT6150) inductor selection](https://github.com/cms-/nrf_node/blob/master/006.jpg)

[Mic frontend output testing](https://github.com/cms-/nrf_node/blob/master/IMG_20161031_221144.jpg)

## Selected problem

First revision boards had reversed the Wheatstone bridge's connection to the amplifier (A-5 on the schematic), rendering the light sensing interface inoperable. Subsequent troubleshooting and board hacking with a razor and winding wire fixed the issue. The circuit and PCB layout were corrected in a subsequent revision.

[Circuit troubleshooting](https://github.com/cms-/nrf_node/blob/master/IMG_20161121_201819.jpg)

[PCB correction](https://github.com/cms-/nrf_node/blob/master/IMG_20161121_201519.jpg)
