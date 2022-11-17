import sys

import machine

sys.path.insert(0, './config')

import config.base as base_config

led_fail = machine.Pin(16, machine.Pin.OUT)
led_fail.off()
led_success = machine.Pin(17, machine.Pin.OUT)
led_success.off()

while True:
    base_config.func_a()

#
# def do_connect():
#     import network
#     sta_if = network.WLAN(network.STA_IF)
#     if not sta_if.isconnected():
#         print('connecting to network...')
#         sta_if.active(True)
#         sta_if.connect(BaseConfig.WIFI_SSID, BaseConfig.WIFI_PASSWORD)
#         while not sta_if.isconnected():
#             led_success.off()
#             led_fail.on()
#
#     while True:
#         led_fail.off()
#         led_success.on()
#         print("Connected. Network config: ", sta_if.ifconfig())
#
#
# if __name__ == '__main__':
#     do_connect()
