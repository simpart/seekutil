#include "seekutil.h"
#include <cstring>

SeekUtil::SeekUtil () {
    try {
        device.open();
        init();
    } catch (char const *err) {
        printf("deinit\n");
        std::vector<uint8_t> data = { 0x00, 0x00 };
        device.send(CmdType::SET_OPERATION_MODE, data);
        device.send(CmdType::SET_OPERATION_MODE, data);
        device.send(CmdType::SET_OPERATION_MODE, data);
        device.close();
        device.open();
        init();
    }
}

SeekUtil::~SeekUtil () {
    try {
        close();
    } catch (char const *err) {
        cout << "[error]" << err << ": " << __FILE__ << "->" << __LINE__ << endl;
    }
}

void SeekUtil::close () {
    try {
        /* send reset */
        std::vector<uint8_t> data;
        data = { 0x00, 0x00 };
        device.send(CmdType::SET_OPERATION_MODE, data);
        device.send(CmdType::SET_OPERATION_MODE, data);
        device.send(CmdType::SET_OPERATION_MODE, data);

        device.close();
    } catch (char const *err) {
        cout << "[error]" << err << ": " << __FILE__ << "->" << __LINE__ << endl;
    }
}

void SeekUtil::init () {
    try {
        std::vector<uint8_t> data;
        
        try {
            data = { 0x01 };
            device.send(CmdType::TARGET_PLATFORM, data);
        } catch (...) {
            throw "failed init";
        }

        data = { 0x00, 0x00 };
        device.send(CmdType::SET_OPERATION_MODE, data);
        
        std::vector<uint8_t> ret_1(4);
        device.read(CmdType::GET_FIRMWARE_INFO, ret_1);

        std::vector<uint8_t> ret_2(12);
        device.read(CmdType::READ_CHIP_ID, ret_2);

        data = { 0x20, 0x00, 0x30, 0x00, 0x00, 0x00 };
        device.send(CmdType::SET_FACTORY_SETTINGS_FEATURES, data);
        
        std::vector<uint8_t> ret_3(64);
        device.read(CmdType::GET_FACTORY_SETTINGS, ret_3);

        data = { 0x20, 0x00, 0x50, 0x00, 0x00, 0x00 };
        device.send(CmdType::SET_FACTORY_SETTINGS_FEATURES, data);
        
        std::vector<uint8_t> ret_4(64);
        device.read(CmdType::GET_FACTORY_SETTINGS, ret_4);
        
        data = { 0x0c, 0x00, 0x70, 0x00, 0x00, 0x00 };
        device.send(CmdType::SET_FACTORY_SETTINGS_FEATURES, data);
        
        std::vector<uint8_t> ret_5(24);
        device.read(CmdType::GET_FACTORY_SETTINGS, ret_5);

        data = { 0x06, 0x00, 0x08, 0x00, 0x00, 0x00 };
        device.send(CmdType::SET_FACTORY_SETTINGS_FEATURES, data);

        std::vector<uint8_t> ret_6(12);
        device.read(CmdType::GET_FACTORY_SETTINGS, ret_6);
        
        data = { 0x08, 0x00 };
        device.send(CmdType::SET_IMAGE_PROCESSING_MODE, data);
        
        std::vector<uint8_t> ret_7(2);
        device.read(CmdType::GET_OPERATION_MODE, ret_7);
        
        data = { 0x08, 0x00 };
        device.send(CmdType::SET_IMAGE_PROCESSING_MODE, data);

        data = { 0x01, 0x00 };
        device.send(CmdType::SET_OPERATION_MODE, data);        

        std::vector<uint8_t> ret_8(2);
        device.read(CmdType::GET_OPERATION_MODE, ret_8);
        
    } catch (char const *err) {
        cout << "[error]" << err << ": " << __FILE__ << " -> " << __LINE__ << endl;
        throw;
    }
}

void SeekUtil::getinfo (SeekInfo_t *info) {
    try {
        if (NULL == info) {
            throw "invalid parameter";
        }
        
        bool isframe = false;
        std::vector<uint8_t> data = { 0xC0, 0x7E, 0x00, 0x00 };
        
        while (!isframe) {
            device.send(CmdType::START_GET_IMAGE_TRANSFER, data);
            device.fetch(buffer, RAW_FRAME_SIZE);
            //printf("frame id:%d\n", buffer[10]);
            isframe = carib.execute(buffer);
        }
        printf("\x1b[1A");
        memcpy(&(info->bmp[54]), carib.bmpdat, sizeof(carib.bmpdat));
        
        info->min_temp = (float) (carib.min_temp - 5950) / 40;
        info->max_temp = (float) (carib.max_temp - 5950) / 40;
        
    } catch (char const *err) {
        cout << "[error]" << err << ": " << __FILE__ << " -> " << __LINE__ << endl;
        throw;
    }
}
