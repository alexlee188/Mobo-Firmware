#    Purpose: Interact with USB device firmware
#    Created: 2008-08-15 by Opendous Inc.
#    Released under the MIT License
#
#    Last Edit: 2009-09-09 by Loftur Jonasson, TF3LJ
#
#    The below is a simple/crude example of how to
#    control a DG8SAQ/PE0FKO device or or the MOBO
#    firmware through Python
#

import usb

# all the critical information regarding the device and the interface and endpoints you plan to use.
vendorid    = 0x16c0
productid   = 0x05dc
confignum   = 1
interfacenum= 0
timeout     = 1500


# Control Message bmRequestType Masks
REQDIR_HOSTTODEVICE = (0 << 7)
REQDIR_DEVICETOHOST = (1 << 7)
REQTYPE_STANDARD    = (0 << 5)
REQTYPE_CLASS       = (1 << 5)
REQTYPE_VENDOR      = (2 << 5)
REQREC_DEVICE       = (0 << 0)
REQREC_INTERFACE    = (1 << 0)
REQREC_ENDPOINT     = (2 << 0)
REQREC_OTHER        = (3 << 0)



# loop over all busses and all devices and find the one with the correct vendor and product IDs
# Note that if you have several of the same devices connected, it will select the last
print "STARTED DeviceAccessPy"
busses = usb.busses()
for bus in busses:
    devices = bus.devices
    for dev in devices:
        if (dev.idVendor == vendorid) & (dev.idProduct == productid):
            founddev = dev
            foundbus = bus

# the device has been found, otherwise exit with error
bus = foundbus
dev = founddev

# open the device for communication
handle = dev.open()

# choose which of the device's configurations to use
handle.setConfiguration(confignum)

# choose which interface to interact with
handle.claimInterface(interfacenum)


#TODO: this while() loop is where you should place your device interaction code
buffer = () # initialize the read/write buffer
##########


maxlen = 8 # size of buffer


# Read state of CW key inputs
bmRTmask = (REQDIR_DEVICETOHOST | REQTYPE_VENDOR | REQTYPE_STANDARD)
buffer = handle.controlMsg(requestType = bmRTmask, request = 0x51, value = 00,
    index = 00, buffer = maxlen, timeout = timeout)
print "CWkey state: 0x%x" % buffer


# Read all registers from Si570 device
bmRTmask = (REQDIR_DEVICETOHOST | REQTYPE_VENDOR | REQTYPE_STANDARD)
buffer = handle.controlMsg(requestType = bmRTmask, request = 0x3a, value = 00,
    index = 00, buffer = maxlen, timeout = timeout)
print "Si570 frequency by value reads:", buffer


#buffer=(0x66,0x66,0x86,0x03)
#buffer=(0xc3,0xf5,0xe8,0x00)

buffer = (0x5c,0x8f,0x82,0x03)
bmRTmask = (REQDIR_HOSTTODEVICE | REQTYPE_VENDOR | REQTYPE_STANDARD)
handle.controlMsg(requestType = bmRTmask, request = 0x32, value = 0,
      index = 0, buffer = buffer, timeout = timeout)
print "Si570 registers set to:", buffer



handle.releaseInterface()

