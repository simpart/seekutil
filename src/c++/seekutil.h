#ifndef SEEK_UTIL_H
#define SEEK_UTIL_H

#include <iostream>
#include <vector>
#include <libusb-1.0/libusb.h>

//#define EXEC_DIR ::exec_dir

using namespace std;


/**
 * @define common flag
 */
#define COM_OK 0
#define COM_NG -1

/**
 * @define device unique number
 */
#define SEEK_ID_VEND 0x289d
#define SEEK_ID_PROD 0x0010

#define RAW_FRAME_WIDTH  208
#define RAW_FRAME_HEIGHT 156
#define RAW_FRAME_SIZE   RAW_FRAME_WIDTH * RAW_FRAME_HEIGHT

typedef struct SeekInfo {
    unsigned char bmp[97398] = {
        66, 77, 118, 124,  1,   0, 0, 0,   0,  0,
        54,  0,   0,   0, 40,   0, 0, 0, 208,  0,
        0,   0, 156,   0,  0,   0, 1, 0,  24,  0,
        0,   0,   0,   0, 64, 124, 1, 0, 195, 14,
        0,   0, 195,  14,  0,   0, 0, 0,   0,  0,
        0,   0,   0,   0
    };
    float  min_temp;
    float  max_temp;
} SeekInfo_t;

struct CmdType {
    enum Enum {
     BEGIN_MEMORY_WRITE              = 82,
     COMPLETE_MEMORY_WRITE           = 81,
     GET_BIT_DATA                    = 59,
     GET_CURRENT_COMMAND_ARRAY       = 68,
     GET_DATA_PAGE                   = 65,
     GET_DEFAULT_COMMAND_ARRAY       = 71,
     GET_ERROR_CODE                  = 53,
     GET_FACTORY_SETTINGS            = 88,
     GET_FIRMWARE_INFO               = 78,
     GET_IMAGE_PROCESSING_MODE       = 63,
     GET_OPERATION_MODE              = 61,
     GET_RDAC_ARRAY                  = 77,
     GET_SHUTTER_POLARITY            = 57,
     GET_VDAC_ARRAY                  = 74,
     READ_CHIP_ID                    = 54,
     RESET_DEVICE                    = 89,
     SET_BIT_DATA_OFFSET             = 58,
     SET_CURRENT_COMMAND_ARRAY       = 67,
     SET_CURRENT_COMMAND_ARRAY_SIZE  = 66,
     SET_DATA_PAGE                   = 64,
     SET_DEFAULT_COMMAND_ARRAY       = 70,
     SET_DEFAULT_COMMAND_ARRAY_SIZE  = 69,
     SET_FACTORY_SETTINGS            = 87,
     SET_FACTORY_SETTINGS_FEATURES   = 86,
     SET_FIRMWARE_INFO_FEATURES      = 85,
     SET_IMAGE_PROCESSING_MODE       = 62,
     SET_OPERATION_MODE              = 60,
     SET_RDAC_ARRAY                  = 76,
     SET_RDAC_ARRAY_OFFSET_AND_ITEMS = 75,
     SET_SHUTTER_POLARITY            = 56,
     SET_VDAC_ARRAY                  = 73,
     SET_VDAC_ARRAY_OFFSET_AND_ITEMS = 72,
     START_GET_IMAGE_TRANSFER        = 83,
     TARGET_PLATFORM                 = 84,
     TOGGLE_SHUTTER                  = 55,
     UPLOAD_FIRMWARE_ROW_SIZE        = 79,
     WRITE_MEMORY_DATA               = 80,
    };
};

class Calibration {
    private:
        uint16_t    *raw_dat = NULL;
        uint16_t     raw_dat2[RAW_FRAME_SIZE] = { 0 };
        bool         bad_pixel[RAW_FRAME_SIZE] = { 0 };
        float        gain_cal[RAW_FRAME_SIZE] = { 0 };
        uint16_t     arr_id1[RAW_FRAME_SIZE] = { 0 };
        uint16_t     arr_id3[RAW_FRAME_SIZE] = { 0 };
        void frame3stuff (void);
        void frame4stuff (void);
        int  getmode (void);
        void mark_badpixels (void);
        void fix_badpixels (void);
        void remove_noise (void);
        unsigned short highest (unsigned short *); 
        unsigned short lowest (unsigned short *); 
        void get_histogram (void);
        void fill_imgbuff (void);
        
    public:
        unsigned char bmpdat[97344] = { 0 };
        float          max_temp = 0;
        float          min_temp = 0;
        bool execute(uint16_t *);
        //Calibration(void);
};

class SeekThermal {
    private:
        libusb_context       *contxt; // context
        libusb_device_handle *handle; // handle
        void initconf (void);
        void transfer (uint8_t, uint8_t, uint16_t, uint16_t, std::vector<uint8_t>&);
        
    public:
        void open (void);
        void close (void);
        void send (CmdType::Enum, std::vector<uint8_t>&);
        void read (CmdType::Enum, std::vector<uint8_t>&);
        void fetch (uint16_t*, std::size_t);
};

class SeekUtil {
    private:
        SeekThermal device;  // device controller
        Calibration carib;
        void init (void);
        //void getframe (void);
        uint16_t buffer[RAW_FRAME_WIDTH * RAW_FRAME_HEIGHT];
        
    public:
        SeekUtil();
        ~SeekUtil();
        void getinfo (SeekInfo_t *);
        void close (void);
};

#endif
