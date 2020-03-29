#!/usr/bin/env python
# -*- coding: utf-8 -*-
from SeekThermal import SeekThermal

if __name__ == "__main__":
    try:
        seek = SeekThermal()
        inf  = seek.getInfo()
        print(inf)

        inf['image'].save('./thermo.bmp')

    except:
        import traceback
        traceback.print_exc()
# end of file
