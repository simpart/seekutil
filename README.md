# seekutil
a function for simple access to SeekThermal by python

it can get the following data:

- image data
- max temperature
- min temperature

## sample image

![sample](https://simpart.github.io/seekutil/sample.bmp)


# support
SeekThermal Compact XR (maybe Compact is ok)

# require
python2.7, numpy, pyusb, Pillow

# SeekThermal.getInfo()
get information from SeekThermal Device

## return
{ 'image': (PIL.Image.Image), 'temperature': { max: (float), min: (float) } }

# sample
please see also ./src/sample.py 

```
seek = SeekThermal()
inf  = seek.getInfo()
print(inf)
inf['image'].save('./thermo.bmp')
```

