import os
import csv
import array
import numpy
import sys

class Calibration:
    bmpdat  = array.array('H', [0] * 97344)
    min_val = None
    max_val = None
    
    __tf = None
    __badPixelArr = array.array('b', [0] * 32448)
    __gainCalArr  = array.array('d', [0] * 32448)
    __arrID1      = None
    __arrID3      = None
    __firstAfterCal   = False
    __lastUsableFrame = None
    __maxTempRaw      = 0
    __gMode           = 0
    __gModePeakCnt    = 0
    __gModePeakIx     = 0
    __pallete         = []

    def __init__(self):
        with open(os.path.dirname(os.path.abspath(__file__)) + '/pallete.csv') as f:
            reader = csv.reader(f)
            idx = 0
            for row in reader:
                if 0 == idx:
                    idx += 1
                    continue
                self.__pallete.append([int(row[0]),int(row[1]),int(row[2])])
            

    def execute(self,tf):
        self.__tf = tf
        ret       = False

        if 4 == tf.status:
            # gain calibration
            self.__frame4Stuff()
        elif 1 == tf.status:
            # shutter calibration
            self.__markBadPixels()
            self.__arrID1 = tf.raw_u16
            self.__firstAfterCal = True
        elif 3 == tf.status:
            # image frame
            self.__markBadPixels()
            self.__frame3Stuff()
            self.__lastUsableFrame = self.__tf
            return True
            
        return ret

    def __frame3Stuff(self):
        self.__arrID3 = self.__tf.raw_u16

        for i in range(32448):
            if self.__arrID3[i] > 2000:
                self.__arrID3[i] = int((self.__arrID3[i] - self.__arrID1[i]) * self.__gainCalArr[i] + 7500)
            else:
                self.__arrID3[i] = 0
                self.__badPixelArr[i] = True

        self.__fixBadPixels()
        self.__removeNoise()
        self.__getHistogram()
        self.__fillImgBuffer()

    def __frame4Stuff(self):
        arr_id4     = self.__tf.raw_u16
        avg_id4     = self.__getMode()
        
        for i in range(32448):
            if (arr_id4[i] > 2000) and (arr_id4[i] < 8000):
                self.__gainCalArr[i] = float(avg_id4) / float(arr_id4[i])
            else:
                self.__gainCalArr[i]  = 1
                self.__badPixelArr[i] = True
        

    def __getMode(self):
        arrMode = array.array('H', [0] * 320)
        topPos  = 0
        arr     = self.__tf.raw_u16

        for elm in arr:
            if (elm > 1000) and ((elm / 100) != 0):
                arrMode[(elm) / 100] += 1
        
        topPos = arrMode.index(numpy.amax(arrMode))
        return topPos * 100

    def __markBadPixels(self):
        raw = self.__tf.raw_u16
        for i in range(32448):
            if (raw[i] < 2000) or (raw[i] > 22000):
                self.__badPixelArr[i] = True
    
    
    def __fixBadPixels(self):
        i   = 0
        nr  = 0
        val = 0

        for y in range(156):
            for x in range(208):
                if self.__badPixelArr[i] and (x < 206):
                    val = 0
                    nr  = 0
                    
                    # top pixel
                    if (y > 0) and (False == self.__badPixelArr[i - 208]):
                        val += self.__arrID3[i - 208]
                        nr += 1

                    # bottom pixel
                    if (y < 155) and (False == self.__badPixelArr[i + 208]):
                        val += self.__arrID3[i + 208]
                        nr += 1

                    # left pixel
                    if (x > 0) and (False == self.__badPixelArr[i - 1]):
                        val += self.__arrID3[i - 1]
                        nr += 1

                    # right pixel
                    if (x < 205) and (False == self.__badPixelArr[i + 1]):
                        val += self.__arrID3[i + 1]
                        nr += 1

                    if nr > 0:
                        val = val/nr
                        self.__arrID3[i] = val

                i += 1
    
    def __removeNoise(self):
        arrColor = array.array('H', [0,0,0,0])
        val      = 0
        i        = 0

        for y in range(156):
            for x in range(208):
                if (x > 0) and (x < 206) and (y > 0) and (y < 155):
                    arrColor[0] = self.__arrID3[i - 208] # top
                    arrColor[1] = self.__arrID3[i + 208] # bottom
                    arrColor[2] = self.__arrID3[i - 1]   # left
                    arrColor[3] = self.__arrID3[i + 1]   # right

                    val = (arrColor[0] + arrColor[1] + arrColor[2] + arrColor[3] - self.__Highest(arrColor) - self.__Lowest(arrColor))/2
                    
                    if (abs(val - self.__arrID3[i]) > 100) and (val != 0):
                        self.__arrID3[i] = val

                    i += 1


    def __Highest(self,num):
        highest = 0

        for i in range(4):
            if num[i] > highest:
                highest = num[i]
        return highest

    def __Lowest(self,num):
        lowest = 30000

        for i in range(4):
            if num[i] < lowest:
                lowest = num[i]
    
        return lowest


    def __getHistogram(self):
        self.__maxTempRaw = numpy.amax(self.__arrID3)

        arrMode = array.array('H', [0] * 2100)
        topPos  = 0

        for i in range(32448):
            if (self.__arrID3[i] > 1000) and (self.__arrID3[i] / 10 != 0) and (False == self.__badPixelArr[i]):
                arrMode[self.__arrID3[i] / 10] += 1

        topPos = arrMode.index(numpy.amax(arrMode))

        self.__gMode        = arrMode
        self.__gModePeakCnt = numpy.amax(arrMode)
        self.__gModePeakIx  = topPos * 10
        
        # lower it to 100px
        for i2 in range(2100):

            self.__gMode[i2] = int(float(arrMode[i2]) / float(self.__gModePeakCnt) * 100)
        
        self.min_val = self.__gModePeakIx
        self.max_val = self.__gModePeakIx
        
        # find left border
        for i3 in range(topPos):
            if arrMode[i3] > arrMode[topPos] * 0.01:
                self.min_val = i3 * 10
                break
        
        # find right border
        i4 = 2099
        for _i4 in range(2099):
            if arrMode[i4] > arrMode[topPos] * 0.01:
                self.max_val = i4 * 10
                break
            i4 -= 1


    def __fillImgBuffer(self):
        v   = 0
        loc = 0
        iScaler = float((self.max_val - self.min_val)) / float(1000)

        for i in range(32448):
            v = self.__arrID3[i]
            if v < self.min_val:
                v = self.min_val

            if v > self.max_val:
                v = self.max_val
            
            v   = v - self.min_val
            loc = int(v / iScaler)
            
            self.bmpdat[i*3]   = self.__pallete[loc][0]
            self.bmpdat[i*3+1] = self.__pallete[loc][1]
            self.bmpdat[i*3+2] = self.__pallete[loc][2]
            
# end of file
