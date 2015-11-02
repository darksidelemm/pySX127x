""" Arduino SPI Bridge BOARD backend. """

# Copyright 2015 Mark Jessop <vk5qi@rfhead.net>
#
# This file is part of pySX127x.
#
# pySX127x is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
# License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
# version.
#
# pySX127x is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
# warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
# details.
#
# You can be released from the requirements of the license by obtaining a commercial license. Such a license is
# mandatory as soon as you develop commercial activities involving pySX127x without disclosing the source code of your
# own applications, or shipping pySX127x with a closed source product.
#
# You should have received a copy of the GNU General Public License along with pySX127x.  If not, see
# <http://www.gnu.org/licenses/>.


import time
from spibridge import SPIBridge


class BOARD:
    """ Board initialisation/teardown and pin configuration is kept here.
        Only the DIO0 and DIO5 pins are wired up
    """
    # The spi object is kept here
    spi = None

    @staticmethod
    def setup(port="/dev/ttyUSB0",baud=57600):
        """ Configure the Raspberry GPIOs
        :rtype : None
        """
        BOARD.spi = SPIBridge(port,baud)

        # blink 2 times to signal the board is set up
        BOARD.blink(.1, 2)

    @staticmethod
    def teardown():
        """ Cleanup Serial Instance """
        BOARD.spi.close()

    @staticmethod
    def SpiDev():
        """ Init and return the SpiDev object
        :return: SpiDev object
        :rtype: SpiDev
        """
        return BOARD.spi

    @staticmethod
    def add_event_detect(dio_number, callback):
        """ Wraps around the GPIO.add_event_detect function
        :param dio_number: DIO pin 0...5
        :param callback: The function to call when the DIO triggers an IRQ.
        :return: None
        """
        pass

    @staticmethod
    def add_events(cb_dio0, cb_dio1, cb_dio2, cb_dio3, cb_dio4, cb_dio5, switch_cb=None):
        pass

    @staticmethod
    def led_on(value=1):
        """ Switch the proto shields LED
        :param value: 0/1 for off/on. Default is 1.
        :return: value
        :rtype : int
        """
        BOARD.spi.set_led(1)
        return value

    @staticmethod
    def led_off():
        """ Switch LED off
        :return: 0
        """
        BOARD.spi.set_led(0)
        return 0

    @staticmethod
    def blink(time_sec, n_blink):
        if n_blink == 0:
            return
        BOARD.led_on()
        for i in range(n_blink):
            time.sleep(time_sec)
            BOARD.led_off()
            time.sleep(time_sec)
            BOARD.led_on()
        BOARD.led_off()
