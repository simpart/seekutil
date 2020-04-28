#include <fstream>
#include "SeekUtil.h"
#include <libgen.h>
#include <vector>
#include <string>
#include <sstream>
#include <string.h>

#include <iostream>
#include <fstream>

extern char pallete[1001][3];
//extern char bmphdr[54];

//Calibration::Calibration () {
//    try {
//        
//    } catch (char const *err) {
//        cout << "[error]" << err << ": " << __FILE__ << " -> " << __LINE__ << endl;
//        throw;
//    }
//}

bool Calibration::execute (uint16_t *dat) {
    try {
        memcpy(raw_dat2, dat, RAW_FRAME_SIZE);
        raw_dat = dat;
        
        if (NULL == dat) {
            throw "invalid parameter";
        }
        int status = dat[10];
        if (4 == status) {
            /* gain calibration */
            //printf("gain calibration\n");
            frame4stuff();
        } else if (1 == status) {
            /* shutter calibration */
            //printf("shutter calibration\n");
            mark_badpixels();
            memcpy(arr_id1, raw_dat, sizeof(uint16_t) * RAW_FRAME_SIZE);
        } else if (3 == status) {
            /* image frame */
            //printf("image frame\n");
            
            mark_badpixels();
            frame3stuff();
            
            //for (int idx=0; idx < RAW_FRAME_SIZE; idx++) {
            //    printf("%d, ", raw_dat[idx]);
            //}
            //printf("\n");
            
            //for (int idx=0; idx < RAW_FRAME_SIZE; idx++) {
            //    
            //    printf("%d, ", arr_id3[idx]);
            //    if (arr_id3[idx] - arr_id1[idx]) {
            //        printf("%d:%d, ", arr_id1[idx], arr_id3[idx]);
            //    }
            //}
            //printf("\n");
            
            return true;
        }
        return false;
        
        //printf("init pallete\n");
        //pallete = { {0} };
    } catch (char const *err) {
        cout << "[error]" << err << ": " << __FILE__ << " -> " << __LINE__ << endl;
        throw;
    }
}

void Calibration::frame4stuff(void) {
    try {
        int avg_id4 = getmode();
        
        for (int idx=0; idx < RAW_FRAME_SIZE; idx++) {
            if ( (raw_dat[idx] > 2000) && (raw_dat[idx] < 8000) ) {
                gain_cal[idx] = (float)avg_id4 / (float)raw_dat[idx];
                //printf("%f\n", gain_cal[idx]);
            } else {
                gain_cal[idx]  = 1;
                bad_pixel[idx] = true;
            }
        }
    } catch (char const *err) {
        cout << "[error]" << err << ": " << __FILE__ << " -> " << __LINE__ << endl;
        throw;
    }
}

void Calibration::frame3stuff(void) {
    try {
        memcpy(arr_id3, raw_dat, sizeof(uint16_t) * RAW_FRAME_SIZE);
        
        //for (int i3=0; i3 < RAW_FRAME_SIZE ;i3++) {
        //    printf("%d, ", arr_id3[i3]);
        //}
        //printf("\n");
        
        //for (int i1=0; i1 < RAW_FRAME_SIZE ;i1++) {
        //    printf("%d,", arr_id1[i1]);
        //}
        //printf("\n");
        
        //for (int ig=0; ig < RAW_FRAME_SIZE ;ig++) {
        //    printf("%f,", gain_cal[ig]);
        //}
        //printf("\n");
        
        for (int idx=0; idx < RAW_FRAME_SIZE; idx++) {
            if (arr_id3[idx] > 2000) {
                arr_id3[idx] = (uint16_t) ((arr_id3[idx] - arr_id1[idx]) * gain_cal[idx] + 7500);
                //printf("%d,", arr_id3[idx]);
            } else {
                arr_id3[idx]   = 0;
                bad_pixel[idx] = true;
            }
        }
        
        //for (int i3=0; i3 < RAW_FRAME_SIZE ;i3++) {
        //    printf("%d,", arr_id3[i3]);
        //}
        //printf("\n");
        
        //printf("\n");
        fix_badpixels();
        
        //for (int i3=0; i3 < RAW_FRAME_SIZE ;i3++) {
        //    printf("%d,", arr_id3[i3]);
        //}
        //printf("\n");
        
        remove_noise();
        get_histogram();
        fill_imgbuff();
        
    } catch (char const *err) {
        cout << "[error]" << err << ": " << __FILE__ << " -> " << __LINE__ << endl;
        throw;
    }
}

void Calibration::fix_badpixels(void) {
    try {
        int i   = 0;
        int nr  = 0;
        int val = 0;
        
        //for (int i3=0; i3 < RAW_FRAME_SIZE ;i3++) {
        //    printf("%d,", arr_id3[i3]);
        //}
        //printf("\n");
        
        for (int y=0; y < RAW_FRAME_HEIGHT; y++) {
            for (int x=0 ; x < RAW_FRAME_WIDTH ; x++) {
                if ( (0 != bad_pixel[i]) && (x < 206) ) {
                    val = 0;
                    nr  = 0;
                    
                    /* top pixel */
                    if ( (y > 0) && (false == bad_pixel[i - 208]) ) {
                        val += arr_id3[i - 208];
                        nr++;
                    }
                    
                    /* bottom pixel */
                    if ( (y < 155) && (false == bad_pixel[i + 208]) ) {
                        val += arr_id3[i + 208];
                        nr++;
                    }
                       
                    /* left pixel */
                    if ( (x > 0) && (false == bad_pixel[i - 1]) ) {
                        val += arr_id3[i - 1];
                        nr++;
                    }
                    
                    /* right pixel */
                    if ( (x < 205) && (false == bad_pixel[i + 1]) ) {
                        val += arr_id3[i + 1];
                        nr++;
                    }
                    
                    if (nr > 0) {
                        val = val/nr;
                        arr_id3[i] = val;
                    }
                }    
                i += 1;
            }
        }
    } catch (char const *err) {
        cout << "[error]" << err << ": " << __FILE__ << " -> " << __LINE__ << endl;
        throw;
    }
}

void Calibration::remove_noise (void) {
    try {
        unsigned short arr_clr[] = { 0,0,0,0 };
        int val = 0;
        int i   = 0;
        
        //for (int i3=0; i3 < RAW_FRAME_SIZE ;i3++) {
        //    printf("%d,", arr_id3[i3]);
        //}
        //printf("\n");

        
        for (int y=0; y < RAW_FRAME_HEIGHT ;y++) {
            for (int x=0; x < RAW_FRAME_WIDTH ;x++) {
                
                if ( (x > 0) && (x < 206) && (y > 0) && (y < 155) ) {
                    arr_clr[0] = arr_id3[i - 208]; // top
                    arr_clr[1] = arr_id3[i + 208]; // bottom
                    arr_clr[2] = arr_id3[i - 1];   // left
                    arr_clr[3] = arr_id3[i + 1];   // right
                    
                    val = (arr_clr[0] + arr_clr[1] + arr_clr[2] + arr_clr[3] - highest(arr_clr) - lowest(arr_clr))/2;
                    if ( (abs(val - arr_id3[i]) > 100) && (val != 0) ) {
                        arr_id3[i] = val;
                    }
                    i++;
                }
            }
        }
    } catch (char const *err) {
        cout << "[error]" << err << ": " << __FILE__ << " -> " << __LINE__ << endl;
        throw;
    }
}


unsigned short Calibration::highest(unsigned short *num) {
    try {
        unsigned short highest = 0;
        
        for (int i=0; i < 4; i++) {
            if (num[i] > highest) {
                highest = num[i];
            }
        }
        return highest;
    } catch (char const *err) {
        cout << "[error]" << err << ": " << __FILE__ << " -> " << __LINE__ << endl;
        throw;
    }
}

unsigned short Calibration::lowest (unsigned short *num) {
    try {
        unsigned short lowest = 30000;
        
        for (int i=0; i < 4; i++) {
            if (num[i] < lowest) {
                lowest = num[i];
            }
        }
        return lowest;
    } catch (char const *err) {
        cout << "[error]" << err << ": " << __FILE__ << " -> " << __LINE__ << endl;
        throw;
    }
}

void Calibration::get_histogram (void) {
    try {
        //max_temp = 0;
        unsigned short arr_mode[2100] = { 0 };
        int top_pos = 0;
        
        //for (int i3=0; i3 < RAW_FRAME_SIZE ;i3++) {
        //    printf("%d,", arr_id3[i3]);
        //}
        //printf("\n");
        
        for (int idx=0; idx < RAW_FRAME_SIZE ;idx++) {
            if ( (arr_id3[idx] > 1000) && (arr_id3[idx]/10 != 0) && (false == bad_pixel[idx]) ) {
                arr_mode[arr_id3[idx]/10] += 1;
            }
        }

        //for (int arr_idx=0; arr_idx < 2100 ;arr_idx++) {
        //    printf("%d, ", arr_mode[arr_idx]);
        //}
        //printf("\n");

        /* get max of arr mode  */
        unsigned short gmode_peakcnt = 0;
        for (int am_idx=0; am_idx < 2100 ;am_idx++) {
            if (gmode_peakcnt < arr_mode[am_idx]) {
                gmode_peakcnt = arr_mode[am_idx];
            }
        }

        //printf("gmode_peakcnt:%d\n", gmode_peakcnt);
        
        /* get index of max */
        for (int am_idx2=0; am_idx2 < 2100 ;am_idx2++) {
            if (gmode_peakcnt == arr_mode[am_idx2]) {
                top_pos = am_idx2;
                break;
            }
        }

        int gmode_peakidx = top_pos * 10;
        //printf("peadidx:%d\n", gmode_peakidx);
        
        min_temp = gmode_peakidx;
        max_temp = gmode_peakidx;
        
        /* find left border */
        for (int i3=0; i3 < top_pos ;i3++) {
            if (arr_mode[i3] > arr_mode[top_pos] * 0.01) {
                min_temp = (float) i3 * 10;
                //printf("i3:%d\n", i3);
                //printf("i3*10:%d\n", i3 * 10);
                //printf("set min:%f\n", min_temp);
                break;
            }
        }
        
        /* find right border */
        for (int i4=2099; i4 > 0 ;i4--) {
            if (arr_mode[i4] > arr_mode[top_pos] * 0.01) {
                max_temp = (float) i4 * 10;
                break;
            }
        }
        
        
        //min_temp = (float) (min_temp - 5950) / 40;
        //max_temp = (float) (max_temp - 5950) / 40;
        
    } catch (char const *err) {
        cout << "[error]" << err << ": " << __FILE__ << " -> " << __LINE__ << endl;
        throw;
    }
}

void Calibration::fill_imgbuff (void) {
    try {
        int v         = 0;
        int loc       = 0;
        float iscaler = (float) (max_temp - min_temp) / (float) 1000;
        
        //printf("iscaler: %f\n", iscaler);
        //printf("max:%f, min:%f\n", max_temp, min_temp);
        
        
        for (int i=0; i < RAW_FRAME_SIZE; i++) {
            v = arr_id3[i];
            if (v < min_temp) {
                v = min_temp;
            }
            if (v > max_temp) {
                v = max_temp;
            }
            v = v - min_temp;
            loc = (int) v / iscaler;
            
            
            bmpdat[i*3]   = pallete[loc][2];
            bmpdat[i*3+1] = pallete[loc][1];
            bmpdat[i*3+2] = pallete[loc][0];
        }
        printf("\n");
    } catch (char const *err) {
        cout << "[error]" << err << ": " << __FILE__ << " -> " << __LINE__ << endl;
        throw;
    }
}

int Calibration::getmode(void) {
    try {
        unsigned short arr_mode[320] = { 0 };
        
        for (int idx=0; idx < RAW_FRAME_SIZE ; idx++) {
            if ( (raw_dat[idx] > 1000) && ((raw_dat[idx] / 100) != 0)) {
                arr_mode[raw_dat[idx] / 100] += 1;
            }
        }
        /* get max */
        unsigned short max = 0;
        for (int aidx=0; aidx < 320; aidx++) {
            if (max < arr_mode[aidx]) {
                max = arr_mode[aidx];
            }
        }
        /* get max index */
        for (int aidx2=0; aidx2 < 320; aidx2++) {
            if (max == arr_mode[aidx2]) {
                return aidx2 * 100;
            }
        }
        throw "could not find max index";
    } catch (char const *err) {
        cout << "[error]" << err << ": " << __FILE__ << " -> " << __LINE__ << endl;
        throw;
    }
}

void Calibration::mark_badpixels(void) {
    try {
	//printf("mark_badpixels\n");
        for (int idx=0; idx < RAW_FRAME_SIZE ; idx++) {
            if ((raw_dat[idx] < 2000) || (raw_dat[idx] > 22000)) {
                bad_pixel[idx] = true;
            }
        }
    } catch (char const *err) {
        cout << "[error]" << err << ": " << __FILE__ << " -> " << __LINE__ << endl;
        throw;
    }
}
/* end of file */
