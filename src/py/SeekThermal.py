import time
import usb
import sys
import array
from Calibration import Calibration
from PIL import Image

class SeekThermal:
    __dev    = None
    __rcvmsg = None
    __calib  = Calibration()

    def __init__(self):
        self.__dev = usb.core.find(idVendor=0x289d, idProduct=0x0010)
        if None == self.__dev:
            raise ValueError('Device not found')
        
        # set the active configuration. With no arguments, the first configuration will be the active one
        self.__dev.set_configuration()
        
        # alias method to make code easier to read
        self.__rcvmsg = self.__dev.ctrl_transfer
        
        # get an endpoint instance
        cfg = self.__dev.get_active_configuration()
        intf = cfg[(0,0)]
        
        custom_match = lambda e: usb.util.endpoint_direction(e.bEndpointAddress) == usb.util.ENDPOINT_OUT
        ep = usb.util.find_descriptor(intf, custom_match=custom_match)   # match the first OUT endpoint
        assert ep is not None
        
        self.__setup()
    
    def __del__(self):
        for i in range(3):
            self.__send(0x41, 0x3C, 0, 0, '\x00\x00')
    

    def __send(self,bmRequestType, bRequest, wValue=0, wIndex=0, data_or_wLength=None, timeout=None):
        assert (self.__dev.ctrl_transfer(bmRequestType, bRequest, wValue, wIndex, data_or_wLength, timeout) == len(data_or_wLength))

    def __setup(self,ini=False):
        if True == ini:
            for i in range(3):
                self.__send(0x41, 0x3C, 0, 0, '\x00\x00')
        
        try:
            msg = '\x01'
            self.__send(0x41, 0x54, 0, 0, msg)
        except Exception as e:
            for i in range(3):
                self.__send(0x41, 0x3C, 0, 0, '\x00\x00')
            self.__send(0x41, 0x54, 0, 0, '\x01')
    
        #  Some day we will figure out what all this init stuff is and
        #  what the returned values mean.
        self.__send(0x41, 0x3C, 0, 0, '\x00\x00')
        ret = self.__rcvmsg(0xC1, 0x4E, 0, 0, 4)
        ret = self.__rcvmsg(0xC1, 0x36, 0, 0, 12)
        
        self.__send(0x41, 0x56, 0, 0, '\x20\x00\x30\x00\x00\x00')
        ret = self.__rcvmsg(0xC1, 0x58, 0, 0, 0x40)
        
        self.__send(0x41, 0x56, 0, 0, '\x20\x00\x50\x00\x00\x00')
        ret = self.__rcvmsg(0xC1, 0x58, 0, 0, 0x40)
        
        self.__send(0x41, 0x56, 0, 0, '\x0C\x00\x70\x00\x00\x00')
        ret = self.__rcvmsg(0xC1, 0x58, 0, 0, 0x18)

        self.__send(0x41, 0x56, 0, 0, '\x06\x00\x08\x00\x00\x00')
        ret = self.__rcvmsg(0xC1, 0x58, 0, 0, 0x0C)
        
        self.__send(0x41, 0x3E, 0, 0, '\x08\x00')
        ret = self.__rcvmsg(0xC1, 0x3D, 0, 0, 2)

        self.__send(0x41, 0x3E, 0, 0, '\x08\x00')
        self.__send(0x41, 0x3C, 0, 0, '\x01\x00')
        ret = self.__rcvmsg(0xC1, 0x3D, 0, 0, 2)


    def __getframe(self):
        while True:
            # Send read frame request
            self.__send(0x41, 0x53, 0, 0, '\xC0\x7E\x00\x00')
            try:
                dat  = self.__dev.read(0x81, 0x3F60, 1000)
                dat += self.__dev.read(0x81, 0x3F60, 3000)
                dat += self.__dev.read(0x81, 0x3F60, 3000)
                dat += self.__dev.read(0x81, 0x3F60, 3000)
            except usb.USBError as e:
                # deinit
                self.__setup(True)
                return self.__getframe()
        
            tf = ThermalFrame(dat)
            # calibration
            ret = self.__calib.execute(tf)
            if False == ret:
                continue
            
            bmp = Image.new('RGB', (208, 156))
            idx = 0
            for hei in reversed(range(0,156)):
                for wid in reversed(range(0,208)):
                    r = self.__calib.bmpdat[idx*3]
                    g = self.__calib.bmpdat[idx*3+1]
                    b = self.__calib.bmpdat[idx*3+2]
                    bmp.putpixel((wid, hei), (r, g, b))
                    idx += 1

            return bmp

    def getInfo(self):
        frm  = self.__getframe()
        maxt = float(self.__calib.max_val - 5950) / 40  # C
        mint = float(self.__calib.min_val - 5950) / 40  # C
        
        return { 'image': frm, 'temperature': { 'max': maxt, 'min': mint } }


class ThermalFrame:
    __wid     = 208
    __hei     = 156
    __raw     = None
    
    raw_u16 = None
    status  = None
    isCalibrationFrame = None
    isUsableFrame      = False

    def __init__(self,dat):
        self.__raw   = dat
        self.raw_u16 = array.array('H');
        self.status  = dat[20]

        self.isCalibrationFrame = (1 == self.status)
        self.isUsableFrame      = (3 == self.status)        
        
        
        didx = 0
        for dat_elm in range(len(dat)/2):
            shf = hex(dat[(didx*2)+1]<<8)
            self.raw_u16.append(int(shf,16) + dat[didx*2])
            didx += 1
        

