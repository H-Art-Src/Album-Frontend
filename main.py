import ctypes as c
from ctypes import CDLL, Structure, POINTER
import discogs_client
import urllib.request 
import os
from PIL import Image

uiFunc = CDLL("build/ui.so")
d = discogs_client.Client('my_user_agent/1.0', user_token='ithHqUJMHmgldAjtELqHEzMyzQyxJDeaIdLpoLsp')

class albumEntry(Structure):
    _fields_ = [('title', c.c_char_p),
                ('desc', c.c_char_p),
                ('img', c.c_char_p)]

enteredList = [133048 , 20017387 , 1010101010101 , 451 , 163867 , 1337]
finalList = []
for releaseNum in enteredList:
    testR = 0
    try:
        testR = d.release(releaseNum)
        print(testR.title) #this triggered the error
    except:
        print("ERROR- " + str(releaseNum) + " not found.")
    else:
        finalList.append(testR)

entryArr = (albumEntry * len(finalList))()
i = 0
for release in finalList:
    print("final listing - " + release.title)
    path = "coverImages/" + str(release.id) + ".jpeg"
    path2 = "coverImages/" + str(release.id) + ".png"
    if not os.path.isfile(path2):
        urllib.request.urlretrieve(release.images[0]['uri'], path)
        im = Image.open(path)
        im.save(path2)
        os.remove(path)
    print(release.images[0]['uri'])
    entryArr.__setitem__(i , (albumEntry( release.title.encode("ascii") , release.artists[0].name.encode("ascii")  , path2.encode("ascii") )))
    i += 1

@c.CFUNCTYPE(None)
def callback():
    print("Yes!!!!")
uiFunc.justgo(callback , entryArr , len(finalList))

print("end python main")