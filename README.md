# HondaFitHDMI
Various efforts into hacking the 2015 Honda Fit head unit to allow HDMI input while driving

Known CAN ID's so far:

**0x12F85150**
Shifter Position (note, all 8 bytes are included in this payload - only the upper two represent the shifter state)

|Gear|Byte[0]|Byte[1]|
|:--:|:-----:|:-----:|
|P   |0x40   |0x03   |
|R   |0x10   |0x03   |
|N   |0x04   |0x03   |
|D   |0x01   |0x03   |
|S (Sport)   |0x00   |0x43   |
|X (Indeterminate)   |0x00   |0x03   |

**0x12F83010**
Driver's side doors

|Driver|Driver Rear|Byte[0]|
|:-:|:-:|:-:|
|Closed|Closed|0x00|
|Open|Closed|0x80|
|Closed|Open|0x20??|
|Open|Open|0xA0|

**0x12F84310**
Trunk Door

|Trunk State|Byte[0]|
|:-:|:-:|
|Closed|0x00|
|Open|0x80|
