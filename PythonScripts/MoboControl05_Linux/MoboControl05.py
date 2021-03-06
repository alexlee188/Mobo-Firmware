#!/usr/bin/python
# -*- coding: cp1252 -*-

############################################################
#
# Adaptation of Control_Mobo_GUI to a PythonCard platform
#
# Version 0.5, 2010-04-18, Loftur J�nasson TF3LJ
#
# This version caters to a couple of new commands in version
# 1.05 of the Mobo 4.3 firmware.  Also, a number of new
# support features have been added, such as write/read to/from
# external PCF8574, improved Bias control, improved frequency
# control, etc...
#
# For guidance on use, see MoboControl05.txt
#
# The original Control_Mobo_GUI was written by
# Alex Lee, 9V1AL & Loftur Jonasson, TF3LJ
#
############################################################

from PythonCard import about, dialog, graphic, model, clipboard
import ConfigParser

import pprint

import os, sys
import wx

import usb
import array, time

import wx.lib.mixins.listctrl as listmix
#

#############################################################
# all the critical information regarding the USB device and
# the interface and endpoints you plan to use.
#############################################################
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

# Some globals
ON  = 1
OFF = 0
FeatureDisabled = 255

#################################################################
# Our one and only Class
#################################################################
class Launcher(model.Background):

    #############################################################
    # Initialise the works
    #############################################################
    def on_initialize(self, event):
        c = self.components
       
        # Init some Firmware identification variables
        self.firmw_I2C_PE0FKO = OFF
        self.firmw_I2C_Mobo = OFF
        self.firmw_I2C_feature = OFF

        # Self timer
        self.timer = wx.Timer(self, -1)
        self.timer.Start(200)       # 200ms intervals
        self.Bind(wx.EVT_TIMER, self.OnTimer)

        # Default values for a number of of items, some of which may not be supported,
        # depending on the firmware.
        self.Repeat_checked = 0     # initial setting of Repeat function is disabled
        self.frqMultiplier = 1      # initial Frequency Spinner multiplier in Hz
        self.rotary_value = 8       # initial value for rotary encoder resolution
        self.offs_frequency = 0     # initial value for PSDR-IQ RX frequency Offset
        #self.real_frequency = 0        
        self.diff_frequency = 0
        self.voltage = 0.0
        self.current = 0.0
        self.Vfwd = 0.0
        self.Vref = 0.0
        self.Pfwd = 0.0
        self.Pref = 0.0
        self.swr = 1.0
        self.temperature = 0.0
        self.tmpalarm = 0
        self.tmpFanOn = 0
        self.tmpFanOff = 0
        self.bias_low = 0           # Bias Low value (nominal is 20 mA)
        self.bias_high = 0          # Bias High value (nominal is 350 mA)
        self.bias_raw1 = 0
        self.bias_raw2 = 0
        self.pcal_value = 1000      # Power Meter Calibration
        self.PWR_Scale = 5          # Power Meter Scale
        self.PEP_value = 10         # PEP Sample window size
        self.SWRalarm_value = 30
        self.SWRlatency_value = 200

        self.PCF_ext_I2C = 0        # Initial default I2C address for External PCF8574 Widget
        self.PCF_output = 0         # Initial output for External PCF Widget
        self.PCF_input = 0          # Initial input for External PCF Widget

        self.frqMultiplier = 100    # Initial value of Frequency Spin is 100Hz 
        
        # Init the analog meters
        self.powerMeterinit(-1)
        self.swrMeterinit(-1)
        self.filtersUpdinit(-1)
        
        # Enumerate the USB
        self.OnUSB(-1)
        time.sleep(0.2)             # Give firmware some breathing space

        # Poll all values for the first time
        try:
            self.on_Refresh_command(-1)
            time.sleep(0.2)             # Give firmware some breathing space
        
            # Read the Crossover filter table for the first time
            self.on_btnFilters_command(-1)
            time.sleep(0.2)             # Give firmware some breathing space
        
            # Read the I2C addresses for the first time
            self.PollI2Caddr(-1)
            time.sleep(0.2)             # Give firmware some breathing space

        except:
            pass

    #############################################################
    # USB Device to Host communications (all comms except for 0x30 - 0x36)
    #############################################################
    def devicetohost(self, req=0, val=0, ind=0, maxlen = 8):
        bmRTmask = (REQDIR_DEVICETOHOST | REQTYPE_VENDOR | REQTYPE_STANDARD)
        buffer = self.handle.controlMsg(requestType = bmRTmask, request = req, value = val,
        index = ind, buffer = 256, timeout = timeout)
        return buffer

    #############################################################
    # USB Host to Device communications (used by Commands 0x30 - 0x36)
    #############################################################
    def hosttodevice(self, req=0, val=0, ind=0, buffer=[]):
        bmRTmask = (REQDIR_HOSTTODEVICE | REQTYPE_VENDOR | REQTYPE_STANDARD)
        try:
            self.handle.controlMsg(requestType = bmRTmask, request = req, value = val,
            index = ind, buffer = buffer, timeout = timeout)
        except:
            time.sleep(0.2) # Wait for a timeout... to fix issues with hosttodevice call
        return

    #############################################################
    # Repeat box or Refresh Button, Display all values
    # (most of the actual work is done from here)
    #############################################################
    # Capture and store state of Repeat tick box when changed       
    def on_Repeat_command(self, event):
            self.Repeat_checked = event.target.checked
            
    # Repeat action Checkbox has been ticked
    def OnTimer(self, event):
        if self.Repeat_checked:
            self.onRefreshLive(-1)

    ######################################
    # Read Frequency, Voltage, Current,
    # Temperature, Power Fwd/Ref and
    # calculate SWR
    # This function polls and displays
    # "live" changeable values
    #####################################
    def onRefreshLive(self, event):
        c = self.components

        # Get Mobo 4.3 Frequency
        self.handle.claimInterface(interfacenum)# Open the USB device for traffic
        output = self.devicetohost(0x3a)
        self.handle.releaseInterface()          # Release the USB device
        self.frequency = 0.0 + output[3]*2**24 + output[2]*2**16 + output[1]*2**8 + output[0]
        self.frequency = int(1000000.0 * self.frequency / (4*2**21) + 0.5)
        self.components.frequencySpinner.value = self.frequency # update spinbox with read value

        if self.firmw_I2C_PE0FKO == OFF :              # If this is not PE0FKO firmware
            try:
                # Get Mobo 4.3 Voltage
                self.handle.claimInterface(interfacenum)# Open the USB device for traffic
                output = self.devicetohost(0x61, 0, 3)
                self.handle.releaseInterface()          # Release the USB device
                self.v_feature = output[0]              # Is this feature supported by firmware?
                self.voltage = (output[1]*2**8 + output[0]) / 65536.0*5*(10+4.7)/4.7

                # Get Mobo 4.3 PA Current
                self.handle.claimInterface(interfacenum)# Open the USB device for traffic
                output = self.devicetohost(0x61, 0, 0)
                self.handle.releaseInterface()          # Release the USB device
                self.current = (output[1]*2**8 + output[0]) / 65536.0*2.5
            except:
                pass

            try:
                # Get Raw Voltage forward and Voltage reflected values from PO/SWR bridge
                self.handle.claimInterface(interfacenum)# Open the USB device for traffic
                output = self.devicetohost(0x61, 0, 1)
                self.handle.releaseInterface()          # Release the USB device
                self.p_feature = output[0]              # Is this feature supported by firmware?
                self.Vfwd = (output[1]*2**8 + output[0]) / 65536.0 * 5.0 # Volts forward
        
                self.handle.claimInterface(interfacenum)# Open the USB device for traffic
                output = self.devicetohost(0x61, 0, 2)
                self.handle.releaseInterface()          # Release the USB device
                self.Vref = (output[1]*2**8 + output[0]) / 65536.0 * 5.0 # Volts reflected
            except:
                pass

            try:
                # Get temperature
                self.handle.claimInterface(interfacenum)# Open the USB device for traffic
                output = self.devicetohost(0x61, 0, 4)
                self.handle.releaseInterface()          # Release the USB device
                self.temp_feature = output[0]           # Is this feature supported by firmware?
                self.temperature = (output[1]*2**8+output[0])*128.0/0x8000
            except:
                pass
        
        # Display Frequency
        c.frqRealSpinner.value = self.frequency
        c.diffDisplay.text = ('%9u' % self.diff_frequency) # initialise diff display value

        # Display Voltage and Current
        if self.firmw_I2C_Mobo != OFF :             # Function not supported by firmware
            c.Vdisplay.text = ('%2.03f V' % self.voltage)
            c.Idisplay.text = ('%1.03f A' % self.current)

        # Calculate Power forward and Power reflected
        if self.p_feature != FeatureDisabled :    # Function not supported by firmware
            self.Pfwd = self.rfpowercalc(self.Vfwd)
            self.Pref = self.rfpowercalc(self.Vref)

            # Calculate SWR
            self.swr = self.SWRcalc(self.Vfwd, self.Vref)

            # Display Power Meter as a function of (Forwarded - Reflected) power
            self.pwr = self.Pfwd - self.Pref
            self.powerMeter(-1)

            # Display SWR Meter
            self.swrMeter(-1)

        # Display Power Forward, Power Reverse and SWR
        c.PFdisplay.text = ('%02.02f W' % self.Pfwd)
        c.PRdisplay.text = ('%02.02f W' % self.Pref)
        c.SWRdisplay.text = ('%02.02f : 1' % self.swr)

        # Display Temperature
        c.TMPdisplay.text = ('%02.01f C' % self.temperature)

    ######################################
    # Read and Display All values
    # This function walks through display widgets
    # and enables/disables as appropriate
    # Reads and displays all values
    #####################################
    def on_Refresh_command(self, event):
        c = self.components
        
        # Set these features as disabled initially
        self.p_feature = FeatureDisabled
        self.v_feature = FeatureDisabled
        self.temp_feature = FeatureDisabled
        self.pcal_feature = FeatureDisabled
        self.pscale_feature = FeatureDisabled
        self.pepwin_feature = FeatureDisabled
        self.bias_feature = FeatureDisabled
        self.SWRalarm_feature = FeatureDisabled
        self.rotary_feature = FeatureDisabled
        self.offs_feature = FeatureDisabled

        # Read and Display Frequency, Voltage, Current,
        # Temperature, Power Fwd/Ref and calculate SWR
        self.onRefreshLive(-1)

        if self.firmw_I2C_PE0FKO == OFF :              # If this is not PE0FKO firmware
            try:
                # Get Power Meter Scale
                self.handle.claimInterface(interfacenum)# Open the USB device for traffic
                output = self.devicetohost(0x66, 0, 4)
                self.handle.releaseInterface()          # Release the USB device
                self.pscale_feature = output[0]         # Is this feature supported by firmware?
                if self.pscale_feature != FeatureDisabled :
                    self.PWR_Scale = output[0]
            except:
                pass

            try:
            # Get Power Calibrate Value
                self.handle.claimInterface(interfacenum)# Open the USB device for traffic
                output = self.devicetohost(0x66, 0, 3)
                self.handle.releaseInterface()          # Release the USB device
                self.pcal_feature = output[0]           # Is this feature supported by firmware?
                if self.pcal_feature != FeatureDisabled :
                    self.pcal_value = output[1]*2**8 + output[0]

                    # Fetch Valid min PWR trigger value
                    self.handle.claimInterface(interfacenum)# Open the USB device for traffic
                    output = self.devicetohost(0x66, 0, 0)
                    self.handle.releaseInterface()          # Release the USB device
                    self.validminPWRtrigger_value = output[1]*2**8 + output[0]
            except:
                pass

            try:
                # Get PEP Sample Window Size
                self.handle.claimInterface(interfacenum)# Open the USB device for traffic
                output = self.devicetohost(0x66, 0, 6)
                self.handle.releaseInterface()          # Release the USB device
                self.pepwin_feature = output[0]         # Is this feature supported by firmware?
                if self.pepwin_feature != FeatureDisabled :
                    self.PEP_value = output[0]
            except:
                pass

            try:
                # Get Rotary Encoder Resolution
                self.handle.claimInterface(interfacenum)# Open the USB device for traffic
                output = self.devicetohost(0x67, 0, 0)
                self.handle.releaseInterface()          # Release the USB device
                self.rotary_feature = output[0]         # Is this feature supported by firmware?
                if self.rotary_feature != FeatureDisabled :
                    self.rotary_value = output[1]*2**8 + output[0]

                    # Get LCD frq offset for PSDR-IQ
                    self.handle.claimInterface(interfacenum)# Open the USB device for traffic
                    output =  self.devicetohost(0x68, 0, 0)
                    self.handle.releaseInterface()          # Release the USB device
                    self.offs_value = output[0]
            except:
                pass

            #try:
            #    # Get LCD frq offset for PSDR-IQ
            #    self.handle.claimInterface(interfacenum)# Open the USB device for traffic
            #    output =  self.devicetohost(0x68, 0, 0)
            #    self.handle.releaseInterface()          # Release the USB device
            #    self.offs_feature = output[0]           # Is this feature supported by firmware?
            #    if self.offs_feature != FeatureDisabled :
            #        self.offs_value = output[0]
            #except:
            #    pass
            
            try:          
                # Get SWR Alarm Value
                self.handle.claimInterface(interfacenum)# Open the USB device for traffic
                output = self.devicetohost(0x66, 0, 2)
                self.handle.releaseInterface()          # Release the USB device
                self.SWRalarm_feature = output[0]       # Is this feature supported by firmware?
                if self.SWRalarm_feature != FeatureDisabled :
                    self.SWRalarm_value = output[1]*256 + output[0]

                # Get SWR Alarm Latency Timer
                self.handle.claimInterface(interfacenum)# Open the USB device for traffic
                output = self.devicetohost(0x66, 0, 1)
                self.handle.releaseInterface()          # Release the USB device
                if self.SWRalarm_feature != FeatureDisabled :
                    self.SWRlatency_value = output[0]
            except:
                pass

            try:
                # Get Bias settings
                self.handle.claimInterface(interfacenum)# Open the USB device for traffic
                output  = self.devicetohost(0x65,0,1)[0]
                self.bias_low = output * 10
                self.bias_feature = output              # is this feature supported by firmware 
                self.bias_high = self.devicetohost(0x65,0,2)[0] * 10
                self.bias_raw1 = self.devicetohost(0x65,0,3)[0]
                self.bias_raw2 = self.devicetohost(0x65,0,4)[0]
                self.bias_status = self.devicetohost(0x65,0,0)[0] 
                self.handle.releaseInterface()          # Release the USB device
            except:
                pass

            try:
                # Get Temperature Alarm and Fan Settings
                self.handle.claimInterface(interfacenum)# Open the USB device for traffic
                self.tmpalarm = self.devicetohost(0x64,0,0)[0]
                self.tmpFanOn = self.devicetohost(0x64,0,1)[0]
                self.tmpFanOff = self.devicetohost(0x64,0,2)[0]
                self.handle.releaseInterface()          # Release the USB device
            except:
                pass

        #        
        # Display Voltage and Current
        #
        #if self.v_feature == FeatureDisabled :    # Function not supported by firmware
        if self.firmw_I2C_Mobo == OFF :             # Function not supported by firmware
            c.stVoltage.visible = OFF
            c.Vdisplay.visible = OFF
            c.stCurrent.visible = OFF
            c.Idisplay.visible = OFF
        else:
            c.stVoltage.visible = ON
            c.Vdisplay.visible = ON
            c.stCurrent.visible = ON
            c.Idisplay.visible = ON

        #
        # Display Power and SWR
        #
        if self.p_feature == FeatureDisabled :    # Function not supported by firmware
            c.stPowerFWD.visible = OFF
            c.stPowerREF.visible = OFF
            c.stSWR.visible = OFF
            c.PFdisplay.visible = OFF
            c.PRdisplay.visible = OFF
            c.SWRdisplay.visible = OFF
            c.stPowergauge.visible = OFF
            c.BitmapCanvasPWR.visible = OFF
            c.stSWRgauge.visible = OFF
            c.BitmapCanvasSWR.visible = OFF
            c.StBoxMeasuredValues.visible = OFF
        else:
            c.stPowerFWD.visible = ON
            c.stPowerREF.visible = ON
            c.stSWR.visible = ON
            c.PFdisplay.visible = ON
            c.PRdisplay.visible = ON
            c.SWRdisplay.visible = ON
            c.stPowergauge.visible = ON
            c.BitmapCanvasPWR.visible = ON
            c.stSWRgauge.visible = ON
            c.BitmapCanvasSWR.visible = ON
            c.StBoxMeasuredValues.visible = ON

        #
        # Display Temperature
        #
        #if self.temp_feature == FeatureDisabled : # Function not supported by firmware
        if self.firmw_I2C_Mobo == OFF :             # Function not supported by firmware
            c.stTemperature.visible = OFF
            c.TMPdisplay.visible = OFF
            # Temperature settings box
            c.stTemp.visible = OFF
            c.stTemp1.visible = OFF
            c.stTemp2.visible = OFF
            c.stTemp3.visible = OFF             
            c.tmpalarmSpinner.visible = OFF
            c.tmpFanOnSpinner.visible = OFF
            c.tmpFanOffSpinner.visible = OFF
        else:
            c.stTemperature.visible = ON
            c.TMPdisplay.visible = ON
            # Temperature settings box
            c.stTemp.visible = ON
            c.stTemp1.visible = ON
            c.stTemp2.visible = ON
            c.stTemp3.visible = ON 
            #c.stTemp1.text = 'Tmp Alarm (�C):'  # Ugly hack, as Pythoncard doesn't display the 'deg' sign
            #c.stTemp2.text = 'Fan On....(�C):'
            #c.stTemp3.text = 'Fan Off....(�C):'           
            c.tmpalarmSpinner.visible = ON
            c.tmpFanOnSpinner.visible = ON
            c.tmpFanOffSpinner.visible = ON
            c.tmpalarmSpinner.value = self.tmpalarm
            c.tmpFanOnSpinner.value = self.tmpFanOn
            c.tmpFanOffSpinner.value = self.tmpFanOff
  
        #
        # Display Power Meter Calibration & SWR Alarm Box
        # 
       
        # Is there anything to display in the Power Meter and SWR Alarm Box?
        c.stbPowerCalText1.visible = OFF
        c.btnPwrButton.visible = OFF

        if self.SWRalarm_feature == FeatureDisabled: # Is this feature supported by firmware?
            c.stSWRAlarm.visible = OFF
            c.stAlarmLatency.visible = OFF
            c.wxtextSWRAlarm.visible = OFF
            c.wxtextSWRlatency.visible = OFF
        else:
            c.stbPowerCalText1.visible = ON
            c.btnPwrButton.visible = ON
            c.stSWRAlarm.visible = ON
            c.stAlarmLatency.visible = ON
            c.wxtextSWRAlarm.visible = ON
            c.wxtextSWRlatency.visible = ON            
            c.wxtextSWRAlarm.text = ('%03.1f' % float(self.SWRalarm_value/10.0))
            c.wxtextSWRlatency.text = ('%u' % self.SWRlatency_value)

        #
        # Display Power Meter Scale
        #
        if self.pscale_feature == FeatureDisabled : # Function not supported by firmware
            c.stPscale.visible = OFF
            c.wxtextCtrlPScale.visible = OFF
        else:
            c.stbPowerCalText1.visible = ON
            c.btnPwrButton.visible = ON
            c.stPscale.visible = ON
            c.wxtextCtrlPScale.visible = ON
            c.wxtextCtrlPScale.text = ('%3u' % self.PWR_Scale)
            self.OnPScale(-1)                       # Set Power meter scale
            
        #
        # Display Power Calibrate Value
        #
        if self.p_feature == FeatureDisabled :      # Function not supported by firmware
        #if self.pcal_feature == FeatureDisabled :  # Function not supported by firmware
            c.stCal.visible = OFF
            c.wxtextCtrlPCal.visible = OFF
            c.stvalidminpwrtrigger.visible = OFF
            c.wxtextvalidminPWRtrigger.visible = OFF
        else:
            c.stbPowerCalText1.visible = ON
            c.btnPwrButton.visible = ON
            c.stCal.visible = ON
            c.wxtextCtrlPCal.visible = ON
            c.stvalidminpwrtrigger.visible = ON
            c.wxtextvalidminPWRtrigger.visible = ON
            c.wxtextCtrlPCal.text = ('%5u' % self.pcal_value)
            c.wxtextvalidminPWRtrigger.text = ('%5u' % self.validminPWRtrigger_value)

        #
        # Display PEP Sample Window Size
        #
        if self.p_feature == FeatureDisabled :      # Function not supported by firmware
        #if self.pepwin_feature == FeatureDisabled :
            c.stPEP.visible = OFF
            c.wxtextCtrlPEPwin.visible = OFF
        else:
            c.stPEP.visible = ON
            c.wxtextCtrlPEPwin.visible = ON
            c.wxtextCtrlPEPwin.text = ('%2u' % self.PEP_value)
         

        # Common to Rotary and Offset
        c.stbFrqOpt.visible = OFF                   # Bells and whistles frame
        
        #
        # Display Rotary Encoder
        #
        if self.rotary_feature == FeatureDisabled : # Function not supported by firmware
            c.stRotary.visible = OFF
            c.wxTextCtrlRotEnc.visible = OFF
            c.btnEncButton.visible = OFF
        else:
            c.stbFrqOpt.visible = ON                # Bells and whistles frame
            c.stRotary.visible = ON
            c.wxTextCtrlRotEnc.visible = ON
            c.btnEncButton.visible = ON
            c.wxTextCtrlRotEnc.text = ('%4d' % self.rotary_value)
            
        #
        # Display LCD frq offset for PSDR-IQ
        #
        #if self.offs_feature == FeatureDisabled :   # Function not supported by firmware
        if self.rotary_feature == FeatureDisabled : # Function not supported by firmware

            c.stLCDoffset.visible = OFF
            c.wxTextCtrlLCDoffs.visible = OFF
            c.btnOffsButton.visible = OFF
        else:
            c.stbFrqOpt.visible = ON                # Bells and whistles frame
            c.stLCDoffset.visible = ON
            c.wxTextCtrlLCDoffs.visible = ON
            c.btnOffsButton.visible = ON
            c.wxTextCtrlLCDoffs.text = ('%3d' % self.offs_frequency)

        #
        # Display Bias Calibration Box
        #
        if self.bias_feature == FeatureDisabled :   # Function not supported by firmware
            c.stbBiasCalText.visible = OFF
            c.stBiasL.visible = OFF
            c.stBiasH.visible = OFF
            c.stBiasRawTextL.visible = OFF
            c.stBiasRawTextH.visible = OFF
            c.btnBiasButton.visible = OFF
            c.btnBiasLButton.visible = OFF
            c.btnBiasHButton.visible = OFF
            c.biasLowSpinner.visible = OFF
            c.biasHighSpinner.visible = OFF
            c.biasRawLSpinner.visible = OFF
            c.biasRawHSpinner.visible = OFF
            c.stBiasStatus.visible = OFF

        else:
            c.stbBiasCalText.visible = ON
            c.stBiasL.visible = ON
            c.stBiasH.visible = ON
            c.stBiasRawTextL.visible = ON
            c.stBiasRawTextH.visible = ON
            c.btnBiasButton.visible = ON
            c.btnBiasLButton.visible = ON
            c.btnBiasHButton.visible = ON
            c.biasLowSpinner.visible = ON
            c.biasHighSpinner.visible = ON
            c.biasRawLSpinner.visible = ON
            c.biasRawHSpinner.visible = ON
            c.stBiasStatus.visible = ON
            c.biasLowSpinner.value = self.bias_low
            c.biasHighSpinner.value = self.bias_high
            c.biasRawLSpinner.value = self.bias_raw1
            c.biasRawHSpinner.value = self.bias_raw2
            bstatusdisplay = ('R','L','H')
            c.stBiasStatus.text = ('%c' % bstatusdisplay[self.bias_status])

        #
        # Display I2C Address Control widgets
        #
        if self.firmw_I2C_Mobo == OFF :     # Function not supported by firmware
            c.stPCF8574A.visible = OFF
            c.stPCF8574B.visible = OFF
            c.stPCF8574C.visible = OFF
            c.stTMP100.visible = OFF
            c.stAD5301.visible = OFF
            c.stAD7991.visible = OFF
            c.stPCF8574F.visible = OFF
            c.wxtext8574m.visible = OFF
            c.wxtext8574l1.visible = OFF
            c.wxtext8574l2.visible = OFF
            c.wxtextTMP100.visible = OFF
            c.wxtextad5301.visible = OFF
            c.wxtextad7991.visible = OFF
            c.wxtext8574f.visible = OFF
        else:
            c.stPCF8574A.visible = ON
            c.stPCF8574B.visible = ON
            c.stPCF8574C.visible = ON
            c.stTMP100.visible = ON
            c.stAD5301.visible = ON
            c.stAD7991.visible = ON
            c.stPCF8574F.visible = ON
            c.wxtext8574m.visible = ON
            c.wxtext8574l1.visible = ON
            c.wxtext8574l2.visible = ON
            c.wxtextTMP100.visible = ON
            c.wxtextad5301.visible = ON
            c.wxtextad7991.visible = ON
            c.wxtext8574f.visible = ON

        #
        # Display External PCF8574 control widgets
        #
        # Multiple I2C if this is Mobo firmware
        if self.firmw_I2C_Mobo == OFF :     # Function not supported by firmware
            c.externalPcf8574.visible = OFF
            c.StaticTextPCFF.visible = OFF
            c.StaticTextPCFF1.visible = OFF
            c.StaticTextPCFF11.visible = OFF
            c.PCFButtonWrite.visible = OFF
            c.PCFButtonRead.visible = OFF
            c.wxTextPCFaddr.visible = OFF
            c.wxTextPCFwrite.visible = OFF
            c.StaticTextPCFRead.visible = OFF
            c.PCFCheckBox0.visible = OFF
            c.PCFCheckBox1.visible = OFF
            c.PCFCheckBox2.visible = OFF
            c.PCFCheckBox3.visible = OFF
            c.PCFCheckBox4.visible = OFF
            c.PCFCheckBox5.visible = OFF
            c.PCFCheckBox6.visible = OFF
            c.PCFCheckBox7.visible = OFF
        else:            
            c.externalPcf8574.visible = ON
            c.StaticTextPCFF.visible = ON
            c.StaticTextPCFF1.visible = ON
            c.StaticTextPCFF11.visible = ON
            c.PCFButtonWrite.visible = ON
            c.PCFButtonRead.visible = ON
            c.wxTextPCFaddr.visible = ON
            c.wxTextPCFwrite.visible = ON
            c.StaticTextPCFRead.visible = ON
            c.PCFCheckBox0.visible = ON
            c.PCFCheckBox1.visible = ON
            c.PCFCheckBox2.visible = ON
            c.PCFCheckBox3.visible = ON
            c.PCFCheckBox4.visible = ON
            c.PCFCheckBox5.visible = ON
            c.PCFCheckBox6.visible = ON
            c.PCFCheckBox7.visible = ON 

            c.wxTextPCFaddr.text = ('%3u' % self.PCF_ext_I2C)
            c.wxTextPCFwrite.text = ('%2X' % self.PCF_output)
            c.StaticTextPCFRead.text = ('%2X' % self.PCF_input)
            c.PCFCheckBox0.checked = self.PCF_input & 0x01
            c.PCFCheckBox1.checked = self.PCF_input & 0x02
            c.PCFCheckBox2.checked = self.PCF_input & 0x04
            c.PCFCheckBox3.checked = self.PCF_input & 0x08
            c.PCFCheckBox4.checked = self.PCF_input & 0x10
            c.PCFCheckBox5.checked = self.PCF_input & 0x20
            c.PCFCheckBox6.checked = self.PCF_input & 0x40
            c.PCFCheckBox7.checked = self.PCF_input & 0x80

        # Set Focus on the Firmware Read Button
        c.btnRefresh.setFocus()

    #############################################################
    # Calculate RF power, based on input voltage from PO/SWR head
    #############################################################
    def rfpowercalc(self, voltage):
        #PWR_Calibrate = 1000 # Power calibrate value
        PWR_Calibrate = self.pcal_value

        # The below formula is lifted directly from the Mobo 4.3 firmware and adapted
        # to floating point arithmetic on actual voltage rather than integer arithmetic
        # on a 16bit value
    
        # If no input voltage, then we do not add offset voltage as this would otherwise
        # result in a bogus minimum power reading.  
        if (voltage > 0.0): voltage = voltage + 0.1 # equals 100mV Schottky diode loss
    
        # Formula roughly adjusted for the ratio in the PO/SWR bridge
        measured_P = voltage * PWR_Calibrate/102.5
        measured_P =(measured_P*measured_P)/50.0
        return measured_P

    #############################################################
    # Calculate Voltage Standing Wave Ratio (SWR)
    #############################################################
    def SWRcalc(self, vfwd, vref):
        V_MIN_TRIGGER = 0.001 # Minimum voltage trigger for a valid measurement
        if vfwd < V_MIN_TRIGGER: measured_SWR = 1.0
        elif vref >= vfwd: measured_SWR = 99.9
        else: measured_SWR = (vfwd + vref)/(vfwd - vref)
        if measured_SWR > 99.9: measured_SWR = 99.9
        return measured_SWR
    
    #############################################################
    # StartUSB Button (is also run automatically on startup)
    # Find and open USB port for access
    #############################################################
    def on_USB_command(self, event):
        self.on_initialize(-1)      # Open USB and poll all values
        
    def OnUSB(self, event):
        # loop over all busses and all devices and find the one with the correct vendor and product IDs
        # Note that if you have several of the same devices connected, it will select the last
        #print "STARTED DeviceAccessPy
        busses = usb.busses()
        for bus in busses:
            devices = bus.devices
            for dev in devices:
                if (dev.idVendor == vendorid) & (dev.idProduct == productid):
                    founddev = dev
                    foundbus = bus
        try:
        # the device has been found, otherwise exit with error
          bus = foundbus
          dev = founddev
        except:
          dlg = wx.MessageDialog(self, "DG8SAQ/MOBO device not found", "USB", wx.OK | wx.ICON_INFORMATION)
          dlg.ShowModal()
          #dlg.Destroy()
          #self.OnExit(-1)
        else:
          # open the device for communication
          self.handle = dev.open()
          # choose which of the device's configurations to use
          self.handle.setConfiguration(confignum)
 
          # Get Firmware serial number
          firmw_serial = self.handle.getString( dev.iSerialNumber, 30)

          # Poll firmware for version information
          self.handle.claimInterface(interfacenum)    # Open the USB device for traffic
          output = self.devicetohost(0x00, 0xa55a, 0) # Poll for version (echo value to capture DG8SAQ firmware)
          self.handle.releaseInterface()
          
          if output == (0x5a, 0xa5):
              self.firmw_I2C_PE0FKO = ON              # Certain features cannot not be supported
              firmw_version = 'DG8SAQ 1.4 or 2.0'
          else:
              firmw_version = str(output[1])+'.'+str(output[0])
              if output[1] == 15:
                  self.firmw_I2C_PE0FKO = ON          # Certain features cannot be supported
                  firmw_version = 'PE0FKO '+firmw_version
                  if output[0] > 11:
                      self.firmw_I2C_feature = ON     # I2C poll feature supported in newer PE0FKO firmware
                  else:
                      self.firmw_I2C_feature = OFF
              elif (output[1] == 16) and (output[0]>99):
                  firmw_version = 'SDRWidget '+firmw_version
                  self.firmw_I2C_PE0FKO = OFF
                  self.firmw_I2C_Mobo = ON
                  self.firmw_I2C_feature = ON
              elif (output[1] == 16) and (output[0]>9):
                  self.firmw_I2C_PE0FKO = OFF
                  firmw_version = 'Mobo '+firmw_version
                  self.firmw_I2C_Mobo = ON
                  self.firmw_I2C_feature = ON
              else:
                  self.firmw_I2C_PE0FKO = OFF
                  self.firmw_I2C_feature = ON
                  firmw_version = 'UBW/ATMega '+firmw_version
                  
              self.components.FirmwareSdisplay.text = (firmw_serial)
              self.components.FirmwareVdisplay.text = (firmw_version)

    ################################################################
    # I2C Address maintenance
    ################################################################
    def PollI2Caddr(self, event):
        c = self.components
        self.I2Caddr = range(8)
        
        if self.firmw_I2C_feature:
          self.handle.claimInterface(interfacenum)  # Open the USB device for traffic
          self.I2Caddr[0] = self.devicetohost(0x41, 0, 0)[0]    # Poll Si570 I2C address
          c.wxtextSi570.text = str(self.I2Caddr[0])
          self.handle.releaseInterface()
          if self.firmw_I2C_Mobo:                   # Poll for further I2C if this is Mobo firmware
              x = 1
              while x < 8:
                self.handle.claimInterface(interfacenum)  # Open the USB device for traffic
                self.I2Caddr[x] = self.devicetohost(0x41, 0, x)[0]  # Poll for I2C addresses
                self.handle.releaseInterface()
                x= x+1
              c.wxtext8574m.text = str(self.I2Caddr[1])
              c.wxtext8574l1.text = str(self.I2Caddr[2])
              c.wxtext8574l2.text = str(self.I2Caddr[3])
              c.wxtextTMP100.text = str(self.I2Caddr[4])
              c.wxtextad5301.text = str(self.I2Caddr[5])
              c.wxtextad7991.text = str(self.I2Caddr[6])
              c.wxtext8574f.text = str(self.I2Caddr[7])
        else:
            c.wxtextSi570.text = str(0x55)          # Old firmware, cannot read

    def on_btnI2CButton_command(self,event):
        c = self.components
        
        self.I2Caddr[0] = int(c.wxtextSi570.text)
        if self.firmw_I2C_Mobo:                     # Further I2C if this is Mobo firmware
            self.I2Caddr[1] = int(c.wxtext8574m.text)
            self.I2Caddr[2] = int(c.wxtext8574l1.text)
            self.I2Caddr[3] = int(c.wxtext8574l2.text)
            self.I2Caddr[4] = int(c.wxtextTMP100.text)
            self.I2Caddr[5] = int(c.wxtextad5301.text)
            self.I2Caddr[6] = int(c.wxtextad7991.text)
            self.I2Caddr[7] = int(c.wxtext8574f.text)

        x = 0
        if self.firmw_I2C_Mobo:                     # Further I2C if this is Mobo firmware
            y = 8
        else:
            y = 1
        while x < y:
            try:
                if self.I2Caddr[x] < 255:
                    self.handle.claimInterface(interfacenum)# Open the USB device for traffic
                    self.devicetohost(0x41,self.I2Caddr[x], x) # Update I2C addresses
                    self.handle.releaseInterface()
            except:
                pass
            x= x+1
            
        # Firmware Reset
        try:
            self.handle.claimInterface(interfacenum) # Open the USB device for traffic
            self.devicetohost(0x0f, 0, 0)            # Reset
            self.handle.releaseInterface()
        except:
            time.sleep(4)   # Wait for Mobo uC to reset itself

        self.OnUSB(-1)                 # Connect the USB bus again
        self.PollI2Caddr(-1)                    # Somewhat redundant, read everything back again
        
    ################################################################
    # Frequency Spin Control
    ################################################################
    def on_ComboBoxFrqSpin_textUpdate(self, event):
        if event.target.stringSelection == 'Hz':
            self.frqMultiplier = 1
        elif event.target.stringSelection == '10Hz':
            self.frqMultiplier = 10
        elif event.target.stringSelection == '100Hz':
            self.frqMultiplier = 100            
        elif event.target.stringSelection == 'kHz':
            self.frqMultiplier = 1000
        elif event.target.stringSelection == '10kHz':
            self.frqMultiplier = 10000
        elif event.target.stringSelection == '100kHz':
            self.frqMultiplier = 100000            
        else: self.frqMultiplier = 1000000
        
    def on_frequencySpinner_textUpdate(self, event):
        if self.components.frequencySpinner.value ==  self.frequency + 1:   # Up
            self.frequency = self.components.frequencySpinner.value = self.frequency + self.frqMultiplier
        elif self.components.frequencySpinner.value ==  self.frequency - 1:   # Down
            self.frequency = self.components.frequencySpinner.value = self.frequency - self.frqMultiplier
        else: self.frequency = self.components.frequencySpinner.value       # Frequency entered 
        self.EvtFrequency(-1)

    def EvtFrequency(self, event):
        freq = self.frequency * 4 * 2**21 / 1000000
        frq3 = freq / 2**24
        frq2 = (freq - frq3 * 2**24) / 2 **16
        frq1 = (freq - frq3 * 2**24 - frq2 * 2**16)/ 2**8
        frq0 = (freq - frq3 * 2**24 - frq2 * 2**16) - frq1*2**16
        buffer = [frq0, frq1, frq2, frq3]
        self.handle.claimInterface(interfacenum) # Open the USB device for traffic
        self.hosttodevice(0x32,buffer=buffer)
        self.handle.releaseInterface() # Release the USB device
        self.components.frequencySpinner.value = self.frequency

    #############################################################
    # Temperature and Cooling Fan settings
    #############################################################
    def on_tmpalarmSpinner_textUpdate(self, event):
        self.tmpalarm = self.components.tmpalarmSpinner.value
        self.handle.claimInterface(interfacenum)# Open the USB device for traffic
        self.devicetohost(0x64,self.tmpalarm,0)
        self.handle.releaseInterface()          # Release the USB device

    def on_tmpFanOnSpinner_textUpdate(self, event):
        self.tmpFanOn = self.components.tmpFanOnSpinner.value
        self.handle.claimInterface(interfacenum)# Open the USB device for traffic
        self.devicetohost(0x64,self.tmpFanOn,1)
        self.handle.releaseInterface()          # Release the USB device

    def on_tmpFanOffSpinner_textUpdate(self, event):
        self.tmpFanOff = self.components.tmpFanOffSpinner.value
        self.handle.claimInterface(interfacenum)# Open the USB device for traffic
        self.devicetohost(0x64,self.tmpFanOff,2)
        self.handle.releaseInterface()          # Release the USB device

    #############################################################
    # Bias Calibration, spincontrol changes
    #############################################################
    def on_biasLowSpinner_textUpdate(self, event):
        self.bias_low = self.components.biasLowSpinner.value / 10
        self.handle.claimInterface(interfacenum)# Open the USB device for traffic
        self.bias_raw1 = self.devicetohost(0x65,self.bias_low,1)[0]
        self.handle.releaseInterface()          # Release the USB device

    def on_biasHighSpinner_textUpdate(self, event):
        self.bias_high = self.components.biasHighSpinner.value / 10
        self.handle.claimInterface(interfacenum)# Open the USB device for traffic
        self.bias_high = self.devicetohost(0x65,self.bias_high,2)[0]
        self.handle.releaseInterface()          # Release the USB device

    def on_biasRawLSpinner_textUpdate(self, event):
        self.bias_raw1 = self.components.biasRawLSpinner.value
        self.handle.claimInterface(interfacenum)# Open the USB device for traffic
        self.bias_raw1 = self.devicetohost(0x65,self.bias_raw1,3)[0]
        self.handle.releaseInterface()          # Release the USB device
        self.components.biasRawLSpinner.value = self.bias_raw1

    def on_biasRawHSpinner_textUpdate(self, event):
        self.bias_raw2 = self.components.biasRawHSpinner.value
        self.handle.claimInterface(interfacenum)# Open the USB device for traffic
        self.bias_raw2 = self.devicetohost(0x65,self.bias_raw2,4)[0]
        self.handle.releaseInterface()          # Release the USB device
        self.components.biasRawHSpinner.value = self.bias_raw2        

    #############################################################
    # Bias Calibration, selection of High or Low bias setting
    #############################################################
    def on_btnBiasLButton_command(self, event):
        self.handle.claimInterface(interfacenum)# Open the USB device for traffic
        self.devicetohost(0x65, 1, 0)           # Select Low Bias
        self.handle.releaseInterface()          # Release the USB device
        self.on_Refresh_command(-1)                 # Poll all values

    def on_btnBiasHButton_command(self, event):
        self.handle.claimInterface(interfacenum)# Open the USB device for traffic
        self.devicetohost(0x65, 2, 0)           # Select Low Bias
        self.handle.releaseInterface()          # Release the USB device
        self.on_Refresh_command(-1)                 # Poll all values

    #############################################################
    # Bias Calibration Button
    #############################################################
    def on_btnBiasButton_command(self, event):
        self.handle.claimInterface(interfacenum)    # Open the USB device for traffic
        self.devicetohost(0x65, 0xfe, 0)
        self.handle.releaseInterface()
        time.sleep(3) # Wait for operation to finish
        self.on_Refresh_command(-1)                 # Poll all values

        
    ################################################################
    # Calibrate Si570 fcryst frequency based on tuning to known freq
    ################################################################
    def on_frqRealSpinner_textUpdate(self, event):
        self.real_frequency = self.components.frqRealSpinner.value
        self.diff_frequency = self.real_frequency - self.frequency
        self.components.diffDisplay.text = ('%9u' % self.diff_frequency)

    def on_btnSi570Calibrate_command(self, event):
        self.handle.claimInterface(interfacenum)    # Open the USB device for traffic
        output = self.devicetohost(0x3d)            # Get fcryst
        self.handle.releaseInterface()
        self.fcryst = 0.0 + output[3]*2**24 + output[2]*2**16 + output[1]*2**8 + output[0]
        self.fcryst = self.fcryst / (2**24) * 1000000.0
        new_fcryst = self.fcryst * self.real_frequency / self.frequency

        freq = int(new_fcryst * (2**24) / 1000000.0)
        frq3 = freq / 2**24
        frq2 = (freq - frq3 * 2**24) / 2 **16
        frq1 = (freq - frq3 * 2**24 - frq2 * 2**16)/ 2**8
        frq0 = (freq - frq3 * 2**24 - frq2 * 2**16) - frq1*2**8
        buf = [frq0, frq1, frq2, frq3]

        self.handle.claimInterface(interfacenum)    # Open the USB device for traffic
        self.hosttodevice(0x33, buffer=buf)         # Set fcryst
        self.handle.releaseInterface()

    #################################################################
    # Set Rotary Encoder Resolution
    #################################################################
    def on_CMD67_command(self, event):
        self.rotary_value = int(self.components.wxTextCtrlRotEnc.text)
 
        self.handle.claimInterface(interfacenum)    # Open the USB device for traffic
        self.devicetohost(0x67, self.rotary_value, 0)  # Set Rotary Enc Resolution
        self.handle.releaseInterface()

    #################################################################
    # Set LCD Display Offset Frequency for PowerSDR-IQ during Receive
    #################################################################
    def on_CMD68_command(self, event):
        self.offs_frequency = int(self.components.wxTextCtrlLCDoffs.text)
        
        freq = self.offs_frequency
        if freq < 0:
            freq = 256 + freq

        self.handle.claimInterface(interfacenum)    # Open the USB device for traffic
        self.devicetohost(0x68, freq, 1) # Set PSDR-IQ-Offset
        self.handle.releaseInterface()

    ################################################################
    # Set Power Meter Bargraph Scale
    ################################################################
    def OnPScale(self, event):
        self.PWR_Scale = int(self.components.wxtextCtrlPScale.text)
        self.powerMeter(-1)                         # Refresh to resize scale
        self.handle.claimInterface(interfacenum)    # Open the USB device for traffic
        self.devicetohost(0x66, self.PWR_Scale, 4)  # Set LCD PowerMeter Bargraph Scale
        self.handle.releaseInterface()

        # A bit sloppy, but hey, so what
        if self.PWR_Scale == FeatureDisabled:
            self.PWR_Scale = int(self.components.wxtextCtrlPScale.text)

    ################################################################
    # Set Valid min PWR trigger value
    ################################################################
    def OnPTrigger(self, event):
        self.validminPWRtrigger_value = int(self.components.wxtextvalidminPWRtrigger.text)
        self.handle.claimInterface(interfacenum)    # Open the USB device for traffic
        self.devicetohost(0x66, self.validminPWRtrigger_value, 0) # Set LCD Power Cal Value
        self.handle.releaseInterface()

    ################################################################
    # Calibrate Power Meter
    ################################################################
    def OnPCal(self, event):
        self.pcal_value = int(self.components.wxtextCtrlPCal.text)
        self.handle.claimInterface(interfacenum)    # Open the USB device for traffic
        self.devicetohost(0x66, self.pcal_value, 3) # Set LCD Power Cal Value
        self.handle.releaseInterface()

    ################################################################
    # Set PEP Sample Size
    ################################################################
    def OnPEP(self, event):
        self.PEP_value = int(self.components.wxtextCtrlPEPwin.text)
        self.handle.claimInterface(interfacenum)    # Open the USB device for traffic
        self.devicetohost(0x66, self.PEP_value, 6)  # Set LCD Power Cal Value
        self.handle.releaseInterface()

        # A bit sloppy, but hey, so what
        if self.PEP_value == FeatureDisabled:
            self.PEP_value = int(self.components.wxtextCtrlPEPwin.text)
            
    ################################################################
    # Set SWR Alarm
    ################################################################
    def OnSWRAlarm(self, event):
        self.SWRalarm_value = int(float(self.components.wxtextSWRAlarm.text)*10.0)
        self.handle.claimInterface(interfacenum)    # Open the USB device for traffic
        self.devicetohost(0x66, self.SWRalarm_value, 2)  # Set SWR Alarm Value
        self.handle.releaseInterface()

    ################################################################
    # Set SWR Alarm Latency Timer
    ################################################################
    def OnSWRLatency(self, event):
        self.SWRlatency_value = int(self.components.wxtextSWRlatency.text)
        self.handle.claimInterface(interfacenum)    # Open the USB device for traffic
        self.devicetohost(0x66, self.SWRlatency_value, 1)  # Set SWR Latency timer Value
        self.handle.releaseInterface()

    ################################################################
    # Power Meter Functions (Scale/Calibrate/PEP)
    ################################################################
    def on_btnPwrButton_command(self, event):
        self.OnPTrigger(-1)
        self.OnPScale(-1)
        self.OnPCal(-1)
        self.OnPEP(-1)
        self.OnSWRAlarm(-1)
        self.OnSWRLatency(-1)
        self.OnUSB(-1) # Read settings back again
        
    ################################################################
    # Reset the Mobo
    ################################################################
    def on_btnReset_command(self, event):
        # Firmware Reset
        try:
            self.handle.claimInterface(interfacenum) # Open the USB device for traffic
            self.devicetohost(0x0f, 0, 0)            # Reset
            self.handle.releaseInterface()
        except:
            time.sleep(4)   # Wait for Mobo uC to reset itself
            self.OnUSB(-1) # Read settings back again
            self.on_Refresh_command(-1)
            self.PollI2Caddr(-1)
            self.on_btnFilters_command(-1)
   
    ################################################################
    # Factory Reset the Mobo
    ################################################################
    def on_btnFactoryReset_command(self, event):
        try:
            self.handle.claimInterface(interfacenum) # Open the USB device for traffic
            self.devicetohost(0x41, 0xff, 0) # Reset to Default
            self.handle.releaseInterface()
        except:
            time.sleep(4)   # Wait for Mobo uC to reset itself
            self.OnUSB(-1) # Read settings back again
            self.on_Refresh_command(-1)
            self.PollI2Caddr(-1)
            self.on_btnFilters_command(-1)
       

    #############################################################
    # External PCF8574 functions
    #############################################################
    #
    # Whenever a PCF Checkbox is ticked or unticked, then
    # translate the PCF Checkbox States into the PCF Output Value
    def on_PCFCheckBox0_command(self, event):
        val = event.target.checked * 0x01
        self.PCF_output = (self.PCF_output & 0xFE) + val
        self.components.wxTextPCFwrite.text = ('%2X' % self.PCF_output)        
    def on_PCFCheckBox1_command(self, event):
        val = event.target.checked * 0x02
        self.PCF_output = (self.PCF_output & 0xFD) + val
        self.components.wxTextPCFwrite.text = ('%2X' % self.PCF_output)
    def on_PCFCheckBox2_command(self, event):
        val = event.target.checked * 0x04
        self.PCF_output = (self.PCF_output & 0xFB) + val
        self.components.wxTextPCFwrite.text = ('%2X' % self.PCF_output)
    def on_PCFCheckBox3_command(self, event):
        val = event.target.checked * 0x08
        self.PCF_output = (self.PCF_output & 0xF7) + val
        self.components.wxTextPCFwrite.text = ('%2X' % self.PCF_output)
    def on_PCFCheckBox4_command(self, event):
        val = event.target.checked * 0x10
        self.PCF_output = (self.PCF_output & 0xEF) + val
        self.components.wxTextPCFwrite.text = ('%2X' % self.PCF_output)
    def on_PCFCheckBox5_command(self, event):
        val = event.target.checked * 0x20
        self.PCF_output = (self.PCF_output & 0xDF) + val
        self.components.wxTextPCFwrite.text = ('%2X' % self.PCF_output)
    def on_PCFCheckBox6_command(self, event):
        val = event.target.checked * 0x40
        self.PCF_output = (self.PCF_output & 0xBF) + val
        self.components.wxTextPCFwrite.text = ('%2X' % self.PCF_output)
    def on_PCFCheckBox7_command(self, event):
        val = event.target.checked * 0x80
        self.PCF_output = (self.PCF_output & 0x7F) + val
        self.components.wxTextPCFwrite.text = ('%2X' % self.PCF_output)

    # Whenever the PCF Output Value is modified,
    # translate the Output Value into the PCF tick boxes
    def on_wxTextPCFwrite_textUpdate(self, event):
        c = self.components
        self.PCF_output = int(c.wxTextPCFwrite.text, 16)
        c.PCFCheckBox0.checked = self.PCF_output & 0x01
        c.PCFCheckBox1.checked = self.PCF_output & 0x02
        c.PCFCheckBox2.checked = self.PCF_output & 0x04
        c.PCFCheckBox3.checked = self.PCF_output & 0x08
        c.PCFCheckBox4.checked = self.PCF_output & 0x10
        c.PCFCheckBox5.checked = self.PCF_output & 0x20
        c.PCFCheckBox6.checked = self.PCF_output & 0x40
        c.PCFCheckBox7.checked = self.PCF_output & 0x80

    # PCF Write was pressed
    def on_PCFButtonWrite_command(self, event):
        c = self.components
        self.PCF_ext_I2C = int(c.wxTextPCFaddr.text)

        # Write to Mobo
        self.handle.claimInterface(interfacenum)
        self.devicetohost(0x6e, self.PCF_output, self.PCF_ext_I2C)
        self.handle.releaseInterface()    
    
    # PCF Read was pressed
    def on_PCFButtonRead_command(self, event):
        c = self.components
        self.PCF_ext_I2C = int(c.wxTextPCFaddr.text)
 
        # Read from Mobo
        self.handle.claimInterface(interfacenum)
        self.PCF_input = self.devicetohost(0x6f, 0, self.PCF_ext_I2C)[0]
        self.handle.releaseInterface()    
 
        # Update the displayed PCF "In" Value 
        c.StaticTextPCFRead.text = ('%2X' % self.PCF_input)
        
        # And copy it into the PCF "Out" Value
        c.wxTextPCFwrite.text = ('%2X' % self.PCF_input)

        # And finally, translate it into the PCF check boxes
        c.PCFCheckBox0.checked = self.PCF_input & 0x01
        c.PCFCheckBox1.checked = self.PCF_input & 0x02
        c.PCFCheckBox2.checked = self.PCF_input & 0x04
        c.PCFCheckBox3.checked = self.PCF_input & 0x08
        c.PCFCheckBox4.checked = self.PCF_input & 0x10
        c.PCFCheckBox5.checked = self.PCF_input & 0x20
        c.PCFCheckBox6.checked = self.PCF_input & 0x40
        c.PCFCheckBox7.checked = self.PCF_input & 0x80


    #############################################################
    # PTT Check Box
    #############################################################
    def on_PTTon_command(self, event):
        self.handle.claimInterface(interfacenum) # Open the USB device for traffic
        ptt_value = event.target.checked
        self.devicetohost(0x50, ptt_value, 0)
        self.handle.releaseInterface()       
        
    #############################################################
    # Exit Button
    #############################################################
    def on_Exit_command(self, event):
        self.timer.Stop()
        self.Close()


    ################################################################
    # Filters Maintenance
    ################################################################
    # Retrieve Filter Switchpoints
    def retrieveswitchpoints(self, raw_switch):
        length = len(raw_switch)/2 - 1
        x = 0
        switchpoint = range(length)
        while x < length:
            # retrieve switchpoints and convert them to kHz
            switchpoint[x]= raw_switch[x*2+1]*2**8 + raw_switch[x*2]
            switchpoint[x] = switchpoint[x] * 1000 / 2**7
            x=x+1
        return switchpoint

    # Read Complete Crossover Table
    def on_btnFilters_command(self, event):
        self.handle.claimInterface(interfacenum)# Open the USB device for traffic
        output = self.devicetohost(0x17,0xffff, 0x00ff,32)
        self.handle.releaseInterface()          # Release the USB device
        swp_bpf = self.retrieveswitchpoints(output)

        self.handle.claimInterface(interfacenum)# Open the USB device for traffic
        output = self.devicetohost(0x17,0xffff, 0x01ff,32)
        self.handle.releaseInterface()          # Release the USB device
        swp_lpf = self.retrieveswitchpoints(output)
        #self.lpf_feature = output[0]

        if self.firmw_I2C_PE0FKO == ON :         # Function not supported by firmware
        #if self.lpf_feature == FeatureDisabled :# Function not supported by firmware
          self.components.LPFlist.visible = OFF
          self.components.stLPFupdate.visible = OFF
          self.components.LPFnr.visible = OFF
          self.components.stLPFcpupdate.visible = OFF
          self.components.LPFnr1.visible = OFF
          self.components.btnLPFupdate.visible = OFF
        else:
          self.components.LPFlist.visible = ON
          self.components.stLPFupdate.visible = ON
          self.components.LPFnr.visible = ON
          self.components.stLPFcpupdate.visible = ON
          self.components.LPFnr1.visible = ON
          self.components.btnLPFupdate.visible = ON

        self.components.BPFlist.columnHeadings=('Filter','Crossover')
        self.length_bpf = len(swp_bpf)
        self.BPFitems = range(self.length_bpf)
        x = 0
        while x < self.length_bpf:
                self.BPFitems[x] = str(swp_bpf[x])
                index = ('BPF# %02d ' % (x), self.BPFitems[x]+' kHz'),
                self.components.BPFlist.Append(index)
                x = x + 1
        
        self.components.LPFlist.columnHeadings=('Filter','Crossover')
        self.length_lpf = len(swp_lpf)
        self.LPFitems = range(self.length_lpf)
        x = 0
        while x < self.length_lpf:   
                self.LPFitems[x] = str(swp_lpf[x])
                index = ('LPF# %02d ' % (x), self.LPFitems[x]+' kHz'),
                self.components.LPFlist.Append(index)
                x = x + 1

    # First time initialize of Filters value boxes
    def filtersUpdinit(self, event):
        self.components.BPFnr.text = str('0')
        self.components.BPFnr1.text = str('0')
        self.components.LPFnr.text = str('0')
        self.components.LPFnr1.text = str('0')

    # Band Pass Filter Update
    def on_btnBPFupdate_command(self, event):
        bpfidx = int(self.components.BPFnr.text)
        bpfCP = int(self.components.BPFnr1.text)
        if bpfidx < self.length_bpf:                # valid entry
            index = bpfidx
            switchpoint = bpfCP * 2**7 / 1000       # Convert value from kHz
            self.handle.claimInterface(interfacenum)
            self.devicetohost(0x17, switchpoint, index) 
            self.handle.releaseInterface()
            self.on_btnFilters_command(-1)          # Read table back

    # Low Pass Filter Update
    def on_btnLPFupdate_command(self, event):
        lpfidx = int(self.components.LPFnr.text)
        lpfCP = int(self.components.LPFnr1.text)
        if lpfidx < self.length_lpf:                # valid entry
            index = lpfidx
            switchpoint = lpfCP * 2**7 / 1000       # Convert value from kHz
            self.handle.claimInterface(interfacenum)
            self.devicetohost(0x17, switchpoint, index+0x100) 
            self.handle.releaseInterface()
            self.on_btnFilters_command(-1)          # Read table back


    ################################################################
    # Configuration Save/Restore
    ################################################################
    #
    # Configuration Save
    #
    def on_btnFileSave_command(self,event):
        c = self.components
        wildcard = "INI files (*.ini)|*.ini|All Files (*.*)|*.*"
        result = dialog.fileDialog(self, 'Save Configuration', '', '', wildcard)
        ### todo... result = wx.FileDialog(self, 'Save Configuration', '', '', wildcard)
        if not result.accepted:
            pprint.pprint(result)
            return

        parser = ConfigParser.ConfigParser()

        parser.add_section('MoboConfig')
        parser.set('MoboConfig','Frequency',self.frequency)

        if self.bias_feature != FeatureDisabled :        # Function supported by firmware
            parser.set('MoboConfig','Bias_Low',self.bias_low)
            parser.set('MoboConfig','Bias_High',self.bias_high)
            parser.set('MoboConfig','Bias_Raw_Low',self.bias_raw1)
            parser.set('MoboConfig','Bias_Raw_High',self.bias_raw2)
            parser.set('MoboConfig','Bias_Status',self.bias_status)

        if self.pscale_feature != FeatureDisabled:
            parser.set('MoboConfig','Power_Scale',self.PWR_Scale)

        if self.pcal_feature != FeatureDisabled:  
            parser.set('MoboConfig','Power_Cal',self.pcal_value)
            parser.set('MoboConfig','Power_min_Trigger',self.validminPWRtrigger_value)

        if self.pepwin_feature != FeatureDisabled:
            parser.set('MoboConfig','PEP_Window',self.PEP_value)

        if self.SWRalarm_feature != FeatureDisabled:
            parser.set('MoboConfig','SWR_alarm',c.wxtextSWRAlarm.text)
            parser.set('MoboConfig','SWR_latency',c.wxtextSWRlatency.text)            

        if self.temp_feature != FeatureDisabled:
            parser.set('MoboConfig','tmp_Alarm',self.tmpalarm)
            parser.set('MoboConfig','tmp_Fan_On',self.tmpFanOn)
            parser.set('MoboConfig','tmp_Fan_Off',self.tmpFanOff)

        if self.rotary_feature != FeatureDisabled : # Function supported by firmware
            parser.set('MoboConfig','Rotary_enc',c.wxTextCtrlRotEnc.text)
            parser.set('MoboConfig','PSDR-IQ_LCD_offs',c.wxTextCtrlLCDoffs.text)
            
        # Process the Frequency Crossover Data
        x = 0
        BPFstring = ''
        while x < len(self.BPFitems):
            BPFstring = BPFstring+str(self.BPFitems[x])+','
            x = x+1
        parser.set('MoboConfig','BPF_Crossover',BPFstring)

        if self.firmw_I2C_PE0FKO != ON :        # Check if Function supported by firmware
        #if self.lpf_feature != FeatureDisabled :# Function not supported by firmware
            x = 0
            LPFstring = ''
            while x < len(self.LPFitems):
                LPFstring = LPFstring+str(self.LPFitems[x])+','
                x = x+1
            parser.set('MoboConfig','LPF_Crossover',LPFstring)
        
        # I2C addresses
        parser.set('MoboConfig','Si570_I2C',c.wxtextSi570.text)
        if self.firmw_I2C_Mobo == ON :          # Function supported by firmware
            parser.set('MoboConfig','PCF8574_M_I2C',c.wxtext8574m.text)
            parser.set('MoboConfig','PCF8574_LPF1_I2C',c.wxtext8574l1.text)
            parser.set('MoboConfig','PCF8574_LPF2_I2C',c.wxtext8574l2.text)
            parser.set('MoboConfig','TMP100_I2C',c.wxtextTMP100.text)
            parser.set('MoboConfig','AD5301_I2C',c.wxtextad5301.text)
            parser.set('MoboConfig','AD7991_I2C',c.wxtextad7991.text)
            parser.set('MoboConfig','PCF8574_F_I2C',c.wxtext8574f.text) # PCF Fan Control Address

           # Strictly speaking, this one does not belong here, but it is related,
            # in fact, the External PCF may actually be the same as the PCF used for Fan Control
            parser.set('MoboConfig','PCF8574_Ext_I2C',c.wxTextPCFaddr.text)
             
        file = open(result.paths[0],'w')
        parser.write(file)
        file.close()
        
    #
    # Configuration Restore
    #
    def on_btnFileRestore_command(self,event):
        #self._save_current_list()
        c = self.components
        wildcard = "INI files (*.ini)|*.ini|All Files (*.*)|*.*"
        result = dialog.fileDialog(self, 'Restore Configuration', '', '', wildcard )
        ### todo... result = wx.FileDialog(self, 'Restore Configuration', '', '', wildcard )
        if not result.accepted:
            pprint.pprint(result)
            return
        parser = ConfigParser.ConfigParser()
        parser.read(result.paths)
        
        # Fetch various Parameters
        self.frequency = int(parser.get('MoboConfig', 'Frequency'))
        if self.bias_feature != FeatureDisabled :           # Function supported by firmware
            c.biasLowSpinner.value = int(parser.get('MoboConfig', 'Bias_Low'))
            c.biasHighSpinner.value = int(parser.get('MoboConfig', 'Bias_High'))
            c.biasRawLSpinner.value = int(parser.get('MoboConfig', 'Bias_Raw_Low'))
            c.biasRawHSpinner.value = int(parser.get('MoboConfig', 'Bias_Raw_High'))
            self.bias_status = int(parser.get('MoboConfig', 'Bias_Status'))
            #self.on_biasLowSpinner_textUpdate(-1)           # Update Bias params.  Ends by a firmware read
            #self.on_biasHighSpinner_textUpdate(-1)         #unnecessary, automatic
            #self.on_biasRawLSpinner_textUpdate(-1)
            #self.on_biasRawHSpinner_textUpdate(-1)

            self.handle.claimInterface(interfacenum)        # Open the USB device for traffic
            self.devicetohost(0x65, self.bias_status, 0)    # Select Bias
            self.handle.releaseInterface()                  # Release the USB device
            self.on_Refresh_command(-1)                     # Poll all values

        if self.pscale_feature != FeatureDisabled:
            c.wxtextCtrlPScale.text = parser.get('MoboConfig', 'Power_Scale')

        if self.pcal_feature != FeatureDisabled:  
            c.wxtextCtrlPCal.text = parser.get('MoboConfig', 'Power_Cal')
            c.wxtextvalidminPWRtrigger.text = parser.get('MoboConfig', 'Power_min_Trigger')

        if self.pepwin_feature != FeatureDisabled:
            c.wxtextCtrlPEPwin.text = parser.get('MoboConfig', 'PEP_Window')

        if self.SWRalarm_feature != FeatureDisabled:
            c.wxtextSWRAlarm.text = parser.get('MoboConfig', 'SWR_alarm')
            c.wxtextSWRlatency.text = parser.get('MoboConfig', 'SWR_latency')

        if self.temp_feature != FeatureDisabled:
            c.tmpalarmSpinner.value = int(parser.get('MoboConfig', 'tmp_alarm'))
            c.tmpFanOnSpinner.value = int(parser.get('MoboConfig', 'tmp_Fan_On'))
            c.tmpFanOffSpinner.value = int(parser.get('MoboConfig', 'tmp_Fan_Off'))

        if self.rotary_feature != FeatureDisabled : # Function supported by firmware
            c.wxTextCtrlRotEnc.text = parser.get('MoboConfig', 'Rotary_enc')
            c.wxTextCtrlLCDoffs.text = parser.get('MoboConfig', 'PSDR-IQ_LCD_offs')
            self.on_CMD67_command(-1)                       # Update Bells and Whistles
            self.on_CMD68_command(-1)                       # Update Bells and Whistles

        # Update BPF table
        BPFstring = parser.get('MoboConfig', 'BPF_Crossover')
        BPFstritems = BPFstring.split(',')
        # Process the Frequency Crossover Data
        x = 0
        while x < len(self.BPFitems):
            self.BPFitems[x] = int(BPFstritems[x])
            x = x+1

        x = 0
        while x < len(self.BPFitems):
            # write one table entry
            switchpoint = self.BPFitems[x] * 2**7 / 1000    # Convert value from kHz
            self.handle.claimInterface(interfacenum)
            self.devicetohost(0x17, switchpoint, x) 
            self.handle.releaseInterface()
            x = x+1

        if self.firmw_I2C_PE0FKO != ON :        # Check if function is supported by firmware
        #if self.lpf_feature != FeatureDisabled :            # Function supported by firmware
           # Update LPF table
            LPFstring = parser.get('MoboConfig', 'LPF_Crossover')
            LPFstritems = LPFstring.split(',')
            # Process the Frequency Crossover Data
            x = 0
            while x < len(self.LPFitems):
                self.LPFitems[x] = int(LPFstritems[x])
                x = x+1
            x = 0
            while x < len(self.LPFitems):
                # write one table entry
                switchpoint = self.LPFitems[x] * 2**7 / 1000 # Convert value from kHz
                self.handle.claimInterface(interfacenum)
                self.devicetohost(0x17, switchpoint, x+0x100) 
                self.handle.releaseInterface()
                x = x+1
        
        self.EvtFrequency(-1)                               # Update frequency
        if (self.pscale_feature != FeatureDisabled) and \
           (self.pcal_feature != FeatureDisabled) and \
           (self.pepwin_feature != FeatureDisabled):
            self.on_btnPwrButton_command(-1)                # Update Power meter parameters

        #if self.temp_feature != FeatureDisabled:
        #    self.on_tmpalarmSpinner_textUpdate(-1)          # unnecessary, automatic
        #    self.on_tmpFanOnSpinner_textUpdate(-1)          # Update Temperature settings
        #    self.on_tmpFanOffSpinner_textUpdate(-1)         # Update Temperature settings

 
        #
        self.on_Refresh_command(-1)
        self.on_btnFilters_command(-1)                      # Read Filters Crossover table back

        # I2C addresses
        c.wxtextSi570.text = parser.get('MoboConfig','Si570_I2C')
        if self.firmw_I2C_Mobo == ON :          # Function supported by firmware
            c.wxtext8574m.text  = parser.get('MoboConfig','PCF8574_M_I2C')
            c.wxtext8574l1.text = parser.get('MoboConfig','PCF8574_LPF1_I2C')
            c.wxtext8574l2.text = parser.get('MoboConfig','PCF8574_LPF2_I2C')
            c.wxtextTMP100.text = parser.get('MoboConfig','TMP100_I2C')
            c.wxtextad5301.text = parser.get('MoboConfig','AD5301_I2C')
            c.wxtextad7991.text = parser.get('MoboConfig','AD7991_I2C')
            c.wxtext8574f.text = parser.get('MoboConfig','PCF8574_F_I2C')

            # Strictly speaking, this one does not belong here, but it is related,
            # in fact, the External PCF may actually be the same as the PCF used for Fan Control
            c.wxTextPCFaddr.text = parser.get('MoboConfig','PCF8574_Ext_I2C')


        self.on_btnI2CButton_command(-1)
        #file = open(result.paths[0],'w')
        #parser.write(file)
        #file.close()        


    #############################################################
    # Power Meter Widget
    #############################################################
    def powerMeterinit(self, event):
        self.filename = None
        comp = self.components
        comp.BitmapCanvasPWR.backgroundColor = 'black'
        comp.BitmapCanvasPWR.clear()

        #A couple of variables for PEP measurement
        global rbuf
        global pow_avg
        rbuf = 0
        pow_avg = array.array('f',[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0])
        
    def powerMeter(self, event):
        canvas = self.components.BitmapCanvasPWR

        num = range(1, self.PWR_Scale, 1)
        #dc = wx.PaintDC(self)
        #canvas.font = self.font
        w, h = canvas.size

        step = float(w / self.PWR_Scale)

        # Calculate PEP power
        # Store value in ringbuffer
        global rbuf
        global pow_avg
        pow_avg[rbuf] = self.pwr
        rbuf +=1
        if rbuf > (self.PEP_value -1):
            rbuf = 0

        # Retrieve the largest value out of the measured PEP window
        pow_pep = 0.0
        for k in range(0,(self.PEP_value - 1),1):
            if pow_avg[k] > pow_pep:
                pow_pep = pow_avg[k]

        till = w * self.pwr  / self.PWR_Scale
        pep  = w * pow_pep   / self.PWR_Scale
        full = w * self.PWR_Scale / self.PWR_Scale

        canvas.clear()
        # Draw all white with a black frame
        canvas.foregroundColor = 'white'
        canvas.fillColor = 'white'
        canvas.backgroundColor = 'black'
        canvas.drawRectangle((1, 1),(full-3, h-3))
        
         # Draw Power gauge
        canvas.foregroundColor = 'green' 
        canvas.fillColor = 'green'
        canvas.drawRectangle((1, 1),(till, h-3))

        # Draw PEP indicator needle
        if pow_pep > 0.0:
            canvas.foregroundColor = 'red' 
            canvas.drawRectangle((pep-2, 1),(2, h-3))
        
        canvas.foregroundColor = 'blue'
        # Draw scale and scale bars in quadruple resolution
        j = 0
        for i in range(1,self.PWR_Scale*4,1):
            if i%4 == 0:  # identify full units
                canvas.drawLine((i*step/4, 1),(i*step/4, 6))
                width, height = canvas.getTextExtent(str(num[j]))
                canvas.drawText(str(num[j]),(i*step/4-width/2, 6))
                j = j + 1
            else:
                canvas.drawLine((i*step/4, 1),(i*step/4, 4))

    #############################################################
    # SWR Meter Widget
    #############################################################
    def swrMeterinit(self, event):
        self.filename = None
        comp = self.components
        comp.BitmapCanvasSWR.backgroundColor = 'black'
        comp.BitmapCanvasSWR.clear()

    def swrMeter(self, event):
        canvas = self.components.BitmapCanvasSWR

        num = range(1, 16, 1)
        scale = ("", "1.03", "1.13", "1.3", "1.5", "1.8" ,"2.1", "2.5",
                 "3.0", "3.5", "4.0", "4.8", "5.5", "6.3", "7.0", "8.0", "")

        w, h = canvas.size

        step = int(round(w / 16.0))

        if self.swr > 9.0: self.swr = 9.0               # Set upper bound
            
        warn = w * 0.5                                  # Set Warn limit at SWR 3.0:1
        full = w * 1.0                                  # Set Full scale
        till = w * (self.swr - 1.0)**.5 / 8.0**.5       # Set measured value

        canvas.clear()
        # Draw all white with a black frame
        canvas.foregroundColor = 'white'
        canvas.fillColor = 'white'
        canvas.backgroundColor = 'black'
        canvas.drawRectangle((1, 1),(full-3, h-3))
  
        #draw SWR
        if self.swr >= 3.0:
            canvas.foregroundColor = 'green' 
            canvas.fillColor = 'green'
            canvas.drawRectangle((1, 1),(warn, h-3))
            canvas.foregroundColor = 'red' 
            canvas.fillColor = 'red'
            canvas.drawRectangle((warn, 1),(till - warn, h-3))
        else: 
            canvas.foregroundColor = 'green' 
            canvas.fillColor = 'green'
            canvas.drawRectangle((1, 1),(till, h-3))

        canvas.foregroundColor = 'blue'

        # Draw scale and scale bars in quadruple resolution
        j = 0
        for i in range(1, 16, 1):
                canvas.drawLine((i*step, 1),(i*step, 6))
                width, height = canvas.getTextExtent(str(num[j]))
                canvas.drawText(scale[j+1],(i*step-width/2, 6))
                j = j + 1

#############################################################
# Here be Main
#############################################################
if __name__ == '__main__':
    app = model.Application(Launcher)
    app.MainLoop()
