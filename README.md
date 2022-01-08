# ESP32-ADAM6050 emulator
    ESP32-ADAM6050 emulator makes it possible to use ESP32 as a I/O module with Synology Surveilance Station
    Copyright (C) 2022  Jānis Ročāns

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.

	This code should be able for ESP32 to act like a ADAM-6050 I/O module with Synology Surveilance Station using WiFi for communications. I've tested it only with Az-Delivery ESP32-WROOM-32 board.
    Documentation used for reference: http://advdownload.advantech.com/productfile/Downloadfile4/1-1M99LTH/ADAM-6000_User_Manaul_Ed_9.pdf
    Change WiFi credentials before use.

    GPIO pins 36, 39, 34 and 35 do not have pull-up resistors on my ESP32 board, so external pull-up resistors should be used.
    I organised pins in order that was easier for me to solder jumper wires, so output pins might seem out of order.

    WARNING!!!
    This code does not implement password validation and session management and should not be used in external networks.
