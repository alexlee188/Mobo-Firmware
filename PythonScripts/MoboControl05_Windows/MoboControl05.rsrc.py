{'application':{'type':'Application',
          'name':'Mobo Control',
    'backgrounds': [
    {'type':'Background',
          'name':'bgControlMobo',
          'title':'Mobo Control Widget',
          'size':(922, 607),

         'components': [

{'type':'StaticText', 
    'name':'StaticTextSpin', 
    'position':(313, 102), 
    'text':'Spin:', 
    },

{'type':'ComboBox', 
    'name':'ComboBoxFrqSpin', 
    'position':(343, 99), 
    'size':(64, -1), 
    'items':[u'Hz', u'10Hz', u'100Hz', u'kHz', u'10kHz', u'100kHz', u'MHz'], 
    'stringSelection':'100Hz', 
    'text':'100Hz', 
    },

{'type':'StaticText', 
    'name':'StaticTextPCFRead', 
    'position':(439, 258), 
    'size':(11, -1), 
    'text':'XX', 
    },

{'type':'TextField', 
    'name':'wxTextPCFaddr', 
    'position':(433, 174), 
    'size':(27, -1), 
    'text':'wxTextPCFaddr', 
    },

{'type':'StaticBox', 
    'name':'externalPcf8574', 
    'position':(383, 154), 
    'size':(111, 153), 
    'label':'External PCF8574', 
    },

{'type':'StaticText', 
    'name':'StaticTextPCFF11', 
    'position':(389, 258), 
    'text':'In   hex:', 
    },

{'type':'StaticText', 
    'name':'StaticTextPCFF1', 
    'position':(388, 207), 
    'text':'Out hex:', 
    },

{'type':'StaticText', 
    'name':'StaticTextPCFF', 
    'position':(387, 178), 
    'text':'I2C addr:', 
    },

{'type':'TextField', 
    'name':'wxTextPCFwrite', 
    'position':(433, 203), 
    'size':(27, -1), 
    'text':'wxTextPCFwrite', 
    },

{'type':'Button', 
    'name':'PCFButtonRead', 
    'position':(387, 278), 
    'command':'PCFButtonRead', 
    'label':'Read', 
    },

{'type':'Button', 
    'name':'PCFButtonWrite', 
    'position':(387, 227), 
    'command':'PCFButtonWrite', 
    'label':'Write', 
    },

{'type':'CheckBox', 
    'name':'PCFCheckBox7', 
    'position':(466, 279), 
    'command':'PCFCheckBox7', 
    'label':'7', 
    },

{'type':'CheckBox', 
    'name':'PCFCheckBox6', 
    'position':(466, 264), 
    'command':'PCFCheckBox6', 
    'label':'6', 
    },

{'type':'CheckBox', 
    'name':'PCFCheckBox5', 
    'position':(466, 249), 
    'command':'PCFCheckBox5', 
    'label':'5', 
    },

{'type':'CheckBox', 
    'name':'PCFCheckBox4', 
    'position':(466, 234), 
    'command':'PCFCheckBox4', 
    'label':'4', 
    },

{'type':'CheckBox', 
    'name':'PCFCheckBox3', 
    'position':(466, 219), 
    'command':'PCFCheckBox3', 
    'label':'3', 
    },

{'type':'CheckBox', 
    'name':'PCFCheckBox2', 
    'position':(466, 204), 
    'command':'PCFCheckBox2', 
    'label':'2', 
    },

{'type':'CheckBox', 
    'name':'PCFCheckBox1', 
    'position':(466, 189), 
    'command':'PCFCheckBox1', 
    'label':'1', 
    },

{'type':'CheckBox', 
    'name':'PCFCheckBox0', 
    'position':(466, 174), 
    'command':'PCFCheckBox0', 
    'label':'0', 
    },

{'type':'StaticText', 
    'name':'stBiasStatus', 
    'position':(353, 229), 
    'size':(9, -1), 
    'foregroundColor':(0, 0, 128, 255), 
    'text':'stBiasStatus', 
    },

{'type':'Button', 
    'name':'btnReset', 
    'position':(545, 538), 
    'command':'btnReset', 
    'label':'Reset', 
    },

{'type':'Button', 
    'name':'btnBiasLButton', 
    'position':(346, 172), 
    'size':(20, -1), 
    'command':'btnBiasLButton', 
    'label':'L', 
    },

{'type':'Button', 
    'name':'btnBiasHButton', 
    'position':(346, 197), 
    'size':(20, -1), 
    'command':'btnBiasHButton', 
    'label':'H', 
    },

{'type':'Spinner', 
    'name':'biasRawHSpinner', 
    'position':(282, 248), 
    'size':(60, -1), 
    'max':254, 
    'min':0, 
    'value':0, 
    },

{'type':'Spinner', 
    'name':'biasRawLSpinner', 
    'position':(282, 223), 
    'size':(60, -1), 
    'max':254, 
    'min':0, 
    'value':0, 
    },

{'type':'StaticText', 
    'name':'stBiasRawTextH', 
    'position':(209, 249), 
    'foregroundColor':(0, 0, 128, 255), 
    'text':'Bias Raw High:', 
    },

{'type':'StaticText', 
    'name':'stTemp3', 
    'position':(508, 241), 
    'foregroundColor':(0, 0, 128, 255), 
    'text':'Fan Off....(C):', 
    },

{'type':'StaticText', 
    'name':'stTemp2', 
    'position':(508, 217), 
    'foregroundColor':(0, 0, 128, 255), 
    'text':'Fan On....(C):', 
    },

{'type':'Spinner', 
    'name':'tmpFanOffSpinner', 
    'position':(585, 237), 
    'size':(50, -1), 
    'max':127, 
    'min':0, 
    'value':40, 
    },

{'type':'Spinner', 
    'name':'tmpFanOnSpinner', 
    'position':(585, 213), 
    'size':(50, -1), 
    'max':127, 
    'min':0, 
    'value':45, 
    },

{'type':'Spinner', 
    'name':'tmpalarmSpinner', 
    'position':(585, 189), 
    'size':(50, -1), 
    'max':127, 
    'min':0, 
    'value':55, 
    },

{'type':'StaticText', 
    'name':'stTemp1', 
    'position':(508, 193), 
    'foregroundColor':(0, 0, 128, 255), 
    'text':'Tmp Alarm (C):', 
    },

{'type':'StaticBox', 
    'name':'stTemp', 
    'position':(503, 171), 
    'size':(136, 94), 
    'label':'Temperature settings', 
    },

{'type':'Button', 
    'name':'btnEncButton', 
    'position':(848, 213), 
    'size':(50, -1), 
    'command':'CMD67', 
    'label':'Update', 
    },

{'type':'StaticText', 
    'name':'stPCF8574F', 
    'position':(512, 468), 
    'foregroundColor':(0, 0, 160, 255), 
    'text':'PCF8574 FAN..:', 
    },

{'type':'TextField', 
    'name':'wxtext8574f', 
    'position':(590, 465), 
    'size':(30, -1), 
    'alignment':'right', 
    'text':'wxtext8574f', 
    },

{'type':'TextField', 
    'name':'wxtextvalidminPWRtrigger', 
    'position':(856, 412), 
    'size':(40, -1), 
    'alignment':'right', 
    'text':'wxtextvalidminPWRtrigger', 
    },

{'type':'StaticText', 
    'name':'stvalidminpwrtrigger', 
    'position':(731, 415), 
    'foregroundColor':(0, 0, 128, 255), 
    'text':'Valid min PWR trig..(mW):', 
    },

{'type':'StaticText', 
    'name':'stAlarmLatency', 
    'position':(731, 394), 
    'foregroundColor':(0, 0, 128, 255), 
    'text':'Alarm Latency...(x10ms):', 
    },

{'type':'StaticText', 
    'name':'stSWRAlarm', 
    'position':(731, 371), 
    'foregroundColor':(0, 0, 128, 255), 
    'text':'SWR Alarm...(1.1 - 99.9):', 
    },

{'type':'TextField', 
    'name':'wxtextSWRlatency', 
    'position':(856, 389), 
    'size':(40, -1), 
    'alignment':'right', 
    'text':'wxtextSWRlatency', 
    },

{'type':'TextField', 
    'name':'wxtextSWRAlarm', 
    'position':(856, 366), 
    'size':(40, -1), 
    'alignment':'right', 
    'text':'wxtextSWRAlarm', 
    },

{'type':'StaticBox', 
    'name':'StaticBoxI2C', 
    'position':(503, 286), 
    'size':(122, 229), 
    'label':'I2C Address Control', 
    },

{'type':'Button', 
    'name':'btnI2CButton', 
    'position':(546, 489), 
    'command':'btnI2CButton', 
    'label':'Update', 
    },

{'type':'TextField', 
    'name':'wxtextad7991', 
    'position':(590, 441), 
    'size':(30, -1), 
    'alignment':'right', 
    'text':'wxtextad7991', 
    },

{'type':'TextField', 
    'name':'wxtextad5301', 
    'position':(590, 418), 
    'size':(30, -1), 
    'alignment':'right', 
    'text':'wxtextad5301', 
    },

{'type':'TextField', 
    'name':'wxtextTMP100', 
    'position':(590, 395), 
    'size':(30, -1), 
    'alignment':'right', 
    'text':'wxtextTMP100', 
    },

{'type':'TextField', 
    'name':'wxtext8574l2', 
    'position':(590, 372), 
    'size':(30, -1), 
    'alignment':'right', 
    'text':'wxtext8574l2', 
    },

{'type':'TextField', 
    'name':'wxtext8574l1', 
    'position':(590, 349), 
    'size':(30, -1), 
    'alignment':'right', 
    'text':'wxtext8574l1', 
    },

{'type':'TextField', 
    'name':'wxtext8574m', 
    'position':(590, 326), 
    'size':(30, -1), 
    'alignment':'right', 
    'text':'wxtext8574m', 
    },

{'type':'TextField', 
    'name':'wxtextSi570', 
    'position':(590, 303), 
    'size':(30, -1), 
    'alignment':'right', 
    'text':'wxtextSi570', 
    },

{'type':'StaticText', 
    'name':'stAD7991', 
    'position':(512, 444), 
    'foregroundColor':(0, 0, 160, 255), 
    'text':'AD7991 ADC...:', 
    },

{'type':'StaticText', 
    'name':'stAD5301', 
    'position':(512, 421), 
    'foregroundColor':(0, 0, 160, 255), 
    'text':'AD5301 DAC...:', 
    },

{'type':'StaticText', 
    'name':'stTMP100', 
    'position':(512, 398), 
    'foregroundColor':(0, 0, 160, 255), 
    'text':'TMP100 temp..:', 
    },

{'type':'StaticText', 
    'name':'stPCF8574C', 
    'position':(512, 375), 
    'foregroundColor':(0, 0, 160, 255), 
    'text':'PCF8574 LPF2.:', 
    },

{'type':'StaticText', 
    'name':'stPCF8574B', 
    'position':(512, 352), 
    'foregroundColor':(0, 0, 160, 255), 
    'text':'PCF8574 LPF1.:', 
    },

{'type':'StaticText', 
    'name':'stPCF8574A', 
    'position':(512, 329), 
    'foregroundColor':(0, 0, 160, 255), 
    'text':'PCF8574 Mobo:', 
    },

{'type':'StaticText', 
    'name':'stSi570', 
    'position':(512, 306), 
    'foregroundColor':(0, 0, 160, 255), 
    'text':'Si570 VCXO....:', 
    },

{'type':'StaticBox', 
    'name':'readFromFirmware', 
    'position':(14, 522), 
    'size':(175, 43), 
    'label':'Read From Firmware', 
    },

{'type':'StaticBox', 
    'name':'ConfigurationSaveRestore', 
    'position':(383, 522), 
    'size':(331, 43), 
    'label':'Configuration Maintenance', 
    },

{'type':'Button', 
    'name':'btnFileSave', 
    'position':(388, 538), 
    'command':'btnFileSave', 
    'default':1, 
    'label':'Save', 
    },

{'type':'Button', 
    'name':'btnFileRestore', 
    'position':(466, 538), 
    'command':'btnFileRestore', 
    'default':1, 
    'label':'Restore', 
    },

{'type':'StaticText', 
    'name':'StaticText1', 
    'position':(626, 344), 
    'size':(98, 90), 
    'alignment':'center', 
    'font':{'style': 'bold', 'faceName': u'Tahoma', 'family': 'sansSerif', 'size': 18}, 
    'foregroundColor':(128, 128, 128, 255), 
    'text':'Mobo Control Widget', 
    },

{'type':'StaticBox', 
    'name':'staticboxFirmware', 
    'position':(726, 479), 
    'size':(175, 86), 
    'label':'Firmware Control', 
    },

{'type':'StaticText', 
    'name':'FirmwareSdisplay', 
    'position':(805, 500), 
    'size':(87, -1), 
    'text':'FirmwareSdisplay', 
    },

{'type':'StaticText', 
    'name':'serialNumber', 
    'position':(733, 499), 
    'foregroundColor':(0, 0, 160, 255), 
    'text':'Serial Number:', 
    },

{'type':'StaticText', 
    'name':'FirmwareVdisplay', 
    'position':(805, 520), 
    'size':(87, -1), 
    'text':'FirmwareVdisplay', 
    },

{'type':'StaticText', 
    'name':'stFirmwareV', 
    'position':(733, 519), 
    'foregroundColor':(0, 0, 160, 255), 
    'text':'Version........:', 
    },

{'type':'StaticBox', 
    'name':'StBoxMeasuredValues', 
    'position':(23, 153), 
    'size':(163, 154), 
    },

{'type':'StaticLine', 
    'name':'StaticLine11', 
    'position':(387, 430), 
    'size':(107, -1), 
    'layout':'horizontal', 
    },

{'type':'StaticLine', 
    'name':'StaticLine1', 
    'position':(387, 344), 
    'size':(107, -1), 
    'layout':'horizontal', 
    },

{'type':'StaticBox', 
    'name':'bpfUpdBox', 
    'position':(383, 308), 
    'size':(111, 207), 
    },

{'type':'Button', 
    'name':'btnBPFupdate', 
    'position':(410, 404), 
    'command':'btnBPFupdate', 
    'label':'Update', 
    },

{'type':'Button', 
    'name':'btnLPFupdate', 
    'position':(410, 489), 
    'command':'btnLPFupdate', 
    'label':'Update', 
    },

{'type':'TextField', 
    'name':'LPFnr', 
    'position':(450, 436), 
    'size':(35, -1), 
    'alignment':'right', 
    'text':'LPFnr', 
    },

{'type':'StaticText', 
    'name':'stLPFupdate', 
    'position':(390, 440), 
    'text':'LPF......:', 
    },

{'type':'TextField', 
    'name':'LPFnr1', 
    'position':(435, 462), 
    'size':(50, -1), 
    'alignment':'right', 
    'text':'LPFnr1', 
    },

{'type':'StaticText', 
    'name':'stLPFcpupdate', 
    'position':(390, 466), 
    'text':'CP (kHz):', 
    },

{'type':'TextField', 
    'name':'BPFnr1', 
    'position':(435, 377), 
    'size':(50, -1), 
    'alignment':'right', 
    'text':'BPFnr1', 
    },

{'type':'StaticText', 
    'name':'stBPFcpupdate', 
    'position':(390, 380), 
    'text':'CP (kHz):', 
    },

{'type':'TextField', 
    'name':'BPFnr', 
    'position':(450, 351), 
    'size':(35, -1), 
    'alignment':'right', 
    'text':'BPFnr', 
    },

{'type':'StaticText', 
    'name':'stBPFupdate', 
    'position':(390, 356), 
    'text':'BPF......:', 
    },

{'type':'StaticBox', 
    'name':'StaticBox1', 
    'position':(512, 92), 
    'size':(125, 71), 
    },

{'type':'StaticBox', 
    'name':'stbPowerCalText1', 
    'position':(726, 280), 
    'size':(175, 188), 
    'label':'Power Meter and SWR Alarm', 
    },

{'type':'StaticBox', 
    'name':'stbBiasCalText', 
    'position':(203, 154), 
    'size':(168, 153), 
    'label':'Bias Calibration', 
    },

{'type':'StaticText', 
    'name':'stAuthor', 
    'position':(245, 543), 
    'size':(82, 13), 
    'alignment':'center', 
    'font':{'faceName': u'Arial', 'family': 'sansSerif', 'size': 6}, 
    'foregroundColor':(0, 0, 128, 255), 
    'text':'V 0.5 2010-05-18 TF3LJ', 
    },

{'type':'Spinner', 
    'name':'biasHighSpinner', 
    'position':(282, 198), 
    'size':(60, -1), 
    'max':2540, 
    'min':0, 
    'value':350, 
    },

{'type':'Spinner', 
    'name':'biasLowSpinner', 
    'position':(282, 173), 
    'size':(60, -1), 
    'max':1000, 
    'min':0, 
    'value':20, 
    },

{'type':'StaticText', 
    'name':'stBiasRawTextL', 
    'position':(208, 226), 
    'foregroundColor':(0, 0, 128, 255), 
    'text':'Bias Raw Low:', 
    },

{'type':'StaticText', 
    'name':'stBiasH', 
    'position':(208, 203), 
    'foregroundColor':(0, 0, 128, 255), 
    'text':'Bias High (mA):', 
    },

{'type':'StaticText', 
    'name':'stBiasL', 
    'position':(208, 178), 
    'foregroundColor':(0, 0, 128, 255), 
    'text':'Bias Low  (mA):', 
    },

{'type':'StaticText', 
    'name':'stFrequency', 
    'position':(10, 109), 
    'font':{'style': 'bold', 'faceName': u'Tahoma', 'family': 'sansSerif', 'size': 12}, 
    'foregroundColor':(0, 0, 255, 255), 
    'text':'Frequency', 
    },

{'type':'StaticText', 
    'name':'stSWRgauge', 
    'position':(10, 60), 
    'font':{'style': 'bold', 'faceName': u'Tahoma', 'family': 'sansSerif', 'size': 12}, 
    'foregroundColor':(0, 0, 255, 255), 
    'text':'SWR', 
    },

{'type':'StaticText', 
    'name':'stPowergauge', 
    'position':(10, 20), 
    'font':{'style': 'bold', 'faceName': u'Tahoma', 'family': 'sansSerif', 'size': 12}, 
    'foregroundColor':(0, 0, 255, 255), 
    'text':'Power', 
    },

{'type':'StaticText', 
    'name':'Vdisplay', 
    'position':(118, 235), 
    'size':(60, 13), 
    'alignment':'right', 
    'font':{'style': 'bold', 'faceName': u'Tahoma', 'family': 'sansSerif', 'size': 10}, 
    'text':'Vdisplay', 
    },

{'type':'TextField', 
    'name':'wxtextCtrlPEPwin', 
    'position':(871, 343), 
    'size':(25, -1), 
    'alignment':'right', 
    'text':'wxtextCtrlPEPwin', 
    },

{'type':'TextField', 
    'name':'wxtextCtrlPCal', 
    'position':(856, 320), 
    'size':(40, -1), 
    'alignment':'right', 
    'text':'wxtextCtrlPCal', 
    },

{'type':'TextField', 
    'name':'wxtextCtrlPScale', 
    'position':(866, 297), 
    'size':(30, -1), 
    'alignment':'right', 
    'text':'wxtextCtrlPScale', 
    },

{'type':'StaticText', 
    'name':'stPEP', 
    'position':(731, 348), 
    'foregroundColor':(0, 0, 128, 255), 
    'text':'PEP Sample window (1-20):', 
    },

{'type':'StaticBox', 
    'name':'stbFrqOpt', 
    'position':(647, 195), 
    'size':(256, 70), 
    'label':'Bells and Whistles', 
    },

{'type':'TextField', 
    'name':'wxTextCtrlLCDoffs', 
    'position':(814, 239), 
    'size':(30, -1), 
    'text':'wxTextCtrlLCDoffs', 
    },

{'type':'TextField', 
    'name':'wxTextCtrlRotEnc', 
    'position':(797, 214), 
    'size':(47, -1), 
    'text':'wxTextCtrlRotEnc', 
    },

{'type':'StaticText', 
    'name':'stLCDoffset', 
    'position':(651, 244), 
    'foregroundColor':(0, 0, 128, 255), 
    'text':'LCD frq offset for PSDR-IQ (kHz):', 
    },

{'type':'StaticText', 
    'name':'stRotary', 
    'position':(651, 219), 
    'foregroundColor':(0, 0, 128, 255), 
    'text':'Rotary Encoder Resolution..:', 
    },

{'type':'StaticText', 
    'name':'SWRdisplay', 
    'position':(118, 210), 
    'size':(60, 13), 
    'alignment':'right', 
    'font':{'style': 'bold', 'faceName': u'Tahoma', 'family': 'sansSerif', 'size': 10}, 
    'text':'SWRdisplay', 
    },

{'type':'StaticText', 
    'name':'stSWR', 
    'position':(28, 210), 
    'font':{'faceName': u'Tahoma', 'family': 'sansSerif', 'size': 10}, 
    'foregroundColor':(0, 0, 128, 255), 
    'text':'SWR              :', 
    },

{'type':'StaticText', 
    'name':'diffDisplay', 
    'position':(548, 122), 
    'alignment':'right', 
    'text':'diffDisplay', 
    },

{'type':'StaticText', 
    'name':'stReal', 
    'position':(515, 104), 
    'foregroundColor':(0, 0, 128, 255), 
    'text':'Real:', 
    },

{'type':'StaticText', 
    'name':'PRdisplay', 
    'position':(118, 185), 
    'size':(60, 13), 
    'alignment':'right', 
    'font':{'style': 'bold', 'faceName': u'Tahoma', 'family': 'sansSerif', 'size': 10}, 
    'text':'PRdisplay', 
    },

{'type':'StaticText', 
    'name':'PFdisplay', 
    'position':(118, 160), 
    'size':(60, 13), 
    'alignment':'right', 
    'font':{'style': 'bold', 'faceName': u'Tahoma', 'family': 'sansSerif', 'size': 10}, 
    'text':'PFdisplay', 
    },

{'type':'StaticText', 
    'name':'Idisplay', 
    'position':(118, 260), 
    'size':(60, 13), 
    'alignment':'right', 
    'font':{'style': 'bold', 'faceName': u'Tahoma', 'family': 'sansSerif', 'size': 10}, 
    'text':'Idisplay', 
    },

{'type':'StaticText', 
    'name':'stDiff', 
    'position':(516, 122), 
    'foregroundColor':(0, 0, 128, 255), 
    'text':'Diff:', 
    },

{'type':'Spinner', 
    'name':'frqRealSpinner', 
    'position':(541, 101), 
    'size':(90, -1), 
    'command':'frqRealSpinner', 
    'max':54000000, 
    'min':0, 
    'value':875000, 
    },

{'type':'StaticText', 
    'name':'stHz', 
    'position':(270, 108), 
    'font':{'faceName': u'Tahoma', 'family': 'sansSerif', 'size': 14}, 
    'text':'Hz', 
    },

{'type':'Spinner', 
    'name':'frequencySpinner', 
    'position':(100, 98), 
    'size':(164, 34), 
    'backgroundColor':(192, 192, 192, 255), 
    'font':{'style': 'bold', 'faceName': u'Calibri', 'family': 'sansSerif', 'size': 20}, 
    'foregroundColor':(128, 0, 0, 255), 
    'max':54000000, 
    'min':0, 
    'value':7020004, 
    },

{'type':'Button', 
    'name':'btnBiasButton', 
    'position':(282, 278), 
    'command':'btnBiasButton', 
    'label':'Bias Calibrate', 
    },

{'type':'CheckBox', 
    'name':'PTTon', 
    'position':(208, 283), 
    'command':'PTTon', 
    'label':'Key TX', 
    },

{'type':'StaticText', 
    'name':'stCal', 
    'position':(731, 325), 
    'foregroundColor':(0, 0, 128, 255), 
    'text':'Cal..............(1-65535):', 
    },

{'type':'StaticText', 
    'name':'stPscale', 
    'position':(731, 302), 
    'foregroundColor':(0, 0, 128, 255), 
    'text':'Scale.............(1-50W):', 
    },

{'type':'StaticText', 
    'name':'TMPdisplay', 
    'position':(118, 285), 
    'size':(60, 13), 
    'alignment':'right', 
    'font':{'style': 'bold', 'faceName': u'Tahoma', 'family': 'sansSerif', 'size': 10}, 
    'text':'TMPdisplay', 
    },

{'type':'StaticText', 
    'name':'stTemperature', 
    'position':(28, 285), 
    'font':{'faceName': u'Tahoma', 'family': 'sansSerif', 'size': 10}, 
    'foregroundColor':(0, 0, 128, 255), 
    'text':'Temperature  :', 
    },

{'type':'StaticText', 
    'name':'stPowerREF', 
    'position':(28, 185), 
    'font':{'faceName': u'Tahoma', 'family': 'sansSerif', 'size': 10}, 
    'foregroundColor':(0, 0, 128, 255), 
    'text':'Power Ref      :', 
    },

{'type':'StaticText', 
    'name':'stPowerFWD', 
    'position':(28, 160), 
    'font':{'faceName': u'Tahoma', 'family': 'sansSerif', 'size': 10}, 
    'foregroundColor':(0, 0, 160, 255), 
    'text':'Power Fwd     :', 
    },

{'type':'StaticText', 
    'name':'stCurrent', 
    'position':(28, 260), 
    'font':{'faceName': u'Tahoma', 'family': 'sansSerif', 'size': 10}, 
    'foregroundColor':(0, 0, 160, 255), 
    'text':'PA Current     :', 
    },

{'type':'StaticText', 
    'name':'stVoltage', 
    'position':(28, 235), 
    'font':{'faceName': u'Tahoma', 'family': 'sansSerif', 'size': 10}, 
    'foregroundColor':(0, 0, 160, 255), 
    'text':'Input Voltage  :', 
    },

{'type':'BitmapCanvas', 
    'name':'BitmapCanvasSWR', 
    'position':(69, 55), 
    'size':(568, 35), 
    'backgroundColor':(255, 255, 255, 255), 
    'font':{'faceName': u'Tahoma', 'family': 'sansSerif', 'size': 8}, 
    },

{'type':'Image', 
    'name':'Image1', 
    'position':(659, 10), 
    'size':(245, 156), 
    'file':'Po_SWR_board_beta.JPG', 
    },

{'type':'Button', 
    'name':'btnPwrButton', 
    'position':(821, 441), 
    'command':'btnPwrButton', 
    'label':'Update', 
    },

{'type':'Button', 
    'name':'btnOffsButton', 
    'position':(848, 238), 
    'size':(50, -1), 
    'command':'CMD68', 
    'label':'Update', 
    },

{'type':'Button', 
    'name':'btnSi570Calibrate', 
    'position':(541, 136), 
    'command':'btnSi570Calibrate', 
    'label':'Si570 Calibrate', 
    },

{'type':'CheckBox', 
    'name':'Repeat', 
    'position':(120, 543), 
    'size':(65, -1), 
    'command':'Repeat', 
    'font':{'style': 'bold', 'faceName': u'Tahoma', 'family': 'sansSerif', 'size': 8}, 
    'label':'Loop live', 
    },

{'type':'Button', 
    'name':'btnFilters', 
    'position':(387, 317), 
    'command':'btnFilters', 
    'label':'All Filters Update', 
    },

{'type':'Button', 
    'name':'btnFactoryReset', 
    'position':(624, 538), 
    'command':'btnFactoryReset', 
    'label':'Factory Reset', 
    },

{'type':'Button', 
    'name':'btnStartUSB', 
    'position':(732, 538), 
    'command':'USB', 
    'label':'Start USB', 
    },

{'type':'BitmapCanvas', 
    'name':'BitmapCanvasPWR', 
    'position':(70, 10), 
    'size':(567, 35), 
    'backgroundColor':(255, 255, 255, 255), 
    'font':{'faceName': u'Tahoma', 'family': 'sansSerif', 'size': 8}, 
    },

{'type':'Button', 
    'name':'btnRefresh', 
    'position':(21, 538), 
    'command':'Refresh', 
    'default':1, 
    'font':{'style': 'bold', 'faceName': u'Tahoma', 'family': 'sansSerif', 'size': 8}, 
    'label':'Poll', 
    },

{'type':'Button', 
    'name':'btnQuit', 
    'position':(821, 538), 
    'command':'Exit', 
    'label':'Quit', 
    },

{'type':'MultiColumnList', 
    'name':'LPFlist', 
    'position':(200, 315), 
    'size':(175, 200), 
    'backgroundColor':(255, 255, 255, 255), 
    'columnHeadings':[u'Filter', 'Crossover (kHz)'], 
    'font':{'faceName': u'Tahoma', 'family': 'sansSerif', 'size': 8}, 
    'items':[], 
    'maxColumns':2, 
    'rules':1, 
    },

{'type':'MultiColumnList', 
    'name':'BPFlist', 
    'position':(15, 315), 
    'size':(175, 200), 
    'backgroundColor':(255, 255, 255, 255), 
    'columnHeadings':[u'Filter', 'Crossover'], 
    'font':{'faceName': u'Tahoma', 'family': 'sansSerif', 'size': 8}, 
    'items':[], 
    'maxColumns':2, 
    'rules':1, 
    },

] # end components
} # end background
] # end backgrounds
} }