#include "seekutil.h"

void SeekThermal::open (void) {
    try {
        int ret  = 0;    // for return check
        int dcnt = 0;    // device count
        struct libusb_device **devs = NULL;    // device list
        struct libusb_device_descriptor desc;
        
        ret = libusb_init(&contxt);
        if (COM_OK != ret) {
            throw "could not get usb context";
        }
        
        /* find seekthermal device */
        dcnt = libusb_get_device_list(contxt, &devs);
        if (0 > dcnt) {
            throw "there is no devices.";
        }
        
        int seek_idx = COM_NG;
        for (int didx = 0; didx < dcnt; didx++) {
            ret = libusb_get_device_descriptor(devs[didx], &desc);
            if (COM_OK != ret) {
                libusb_free_device_list(devs, 1);
                throw "could not get device descriptor";
            }
            if ( (desc.idVendor == SEEK_ID_VEND) && (desc.idProduct == SEEK_ID_PROD) ) {
                seek_idx = didx;
                break;
            }
        }
        
        if (COM_NG == seek_idx) {
            libusb_free_device_list(devs, 1);
            throw "could not find seekthermal device";
        }
        
        ret = libusb_open(devs[seek_idx], &handle);
        if (COM_OK != ret) {
            libusb_free_device_list(devs, 1);
            throw "could not open seekthermal device";
        }
        
        libusb_free_device_list(devs, 1);
        
        initconf();
    } catch (char const *err) {
        cout << "[error]" << err << ": " << __FILE__ << " -> " << __LINE__ << endl;
        throw;
    }
}

void SeekThermal::initconf () {
    try {
        int ret     = 0;
        int cnf_val = 0;
        
        ret = libusb_get_configuration(handle, &cnf_val);
        if (COM_OK != ret) {
            close();
            throw "could not get libusb configuration";
        }

        if (cnf_val != 1) {
            ret = libusb_set_configuration(handle, 1);
            if (COM_OK != ret) {
                close();
                throw "could not set libusb configuration";
            }
        }
        
        ret = libusb_claim_interface(handle, 0);
        if (0 > ret) {
            close();
            throw "failed to claim interface";
        }
    } catch (char const *err) {
        cout << "[error]" << err << ": " << __FILE__ << " -> " << __LINE__ << endl;
        throw;
    }
}

void SeekThermal::close (void) {
    try {
        if (handle != NULL) {
            libusb_release_interface(handle, 0);  /* release claim */
            libusb_close(handle);                 /* revert open */
            handle = NULL;
        }
        
        if (contxt != NULL) {
            libusb_exit(contxt);                  /* revert exit */
            contxt = NULL;
        }
    } catch (char const *err) {
        cout << "[error]" << err << ": " << __FILE__ << " -> " << __LINE__ << endl;
        throw;
    }
}

void SeekThermal::send (CmdType::Enum cmd, std::vector<uint8_t>& data) {
    try {
        transfer(0x41, static_cast<char>(cmd), 0, 0, data);
    } catch (char const *err) {
        cout << "[error]" << err << ": " << __FILE__ << " -> " << __LINE__ << endl;
        throw;
    }
}

void SeekThermal::read (CmdType::Enum cmd, std::vector<uint8_t>& data) {
    try {
        transfer(0xC1, static_cast<char>(cmd), 0, 0, data);
    } catch (char const *err) {
        cout << "[error]" << err << ": " << __FILE__ << " -> " << __LINE__ << endl;
        throw;
    }
}

void SeekThermal::fetch (uint16_t* buffer, std::size_t size) {
    try {
        int ret;
        int actual_length;
        int todo     = size * sizeof(uint16_t);
        uint8_t* buf = reinterpret_cast<uint8_t*>(buffer);
        int done     = 0;
        
        while (todo != 0) {
            ret = libusb_bulk_transfer(handle, 0x81, &buf[done], todo, &actual_length, 0);
            if (COM_OK != ret) {
                throw libusb_error_name(ret);
            }
//        debug("Actual length %d\n", actual_length);
            todo -= actual_length;
            done += actual_length;
        }
        
        /* correct endianness */
        for (int i=0; i < (int) size; i++) {
            buffer[i] = le16toh(buffer[i]);
        }
    } catch (char const *err) {
        cout << "[error]" << err << ": " << __FILE__ << " -> " << __LINE__ << endl;
        throw;
    }
}

void SeekThermal::transfer(uint8_t req_tp, uint8_t req, uint16_t value, uint16_t index, std::vector<uint8_t>& data) {
    try {
        int ret = 0;
        if (data.size() == 0) {
            data.reserve(16);
        }
        
        /* send to device */
        ret = libusb_control_transfer(handle, req_tp, req, value, index, data.data(), data.size(), 0);
        if (ret < 0) {
            throw libusb_error_name(ret);
        }
        if (ret != (int) data.size()) {
            throw "mismatched transfer bytes";
        }
    } catch (char const *err) {
        cout << "[error]" << err << ": " << __FILE__ << " -> " << __LINE__ << endl;
        throw;
    }
}


/* end of file */
