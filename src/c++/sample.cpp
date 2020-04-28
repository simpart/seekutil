#include "SeekUtil.h"
#include <libgen.h>
#include <iostream>
#include <fstream>
#include <libgen.h>

int main (int argc, char *argv[]) {
    try {
        SeekUtil seek;
        SeekInfo info;
        ofstream fout;
        
        seek.getinfo(&info);
        
        char thm_path[128] = { 0 };
        snprintf(&thm_path[0], sizeof(thm_path), "%s/thermo.bmp", dirname(argv[0]));
        
        fout.open(thm_path, ios::out|ios::binary|ios::trunc);
        if (!fout) {
            throw "could not open bitmap file";
        }
        
        fout.write((char *) info.bmp, sizeof(info.bmp));
        
        fout.close();

    } catch (char const* err) {
         cout << "[error]" << err << ": " << __FILE__ << "->" << __LINE__ << endl;
    }
}