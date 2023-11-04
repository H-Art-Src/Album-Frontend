#Author: James Raw

import ctypes as c
from ctypes import CDLL, Structure
import discogs_client
import urllib.request 
import os
from PIL import Image


#--Query Discogs for release data.
d = discogs_client.Client('my_user_agent/1.0', user_token='ithHqUJMHmgldAjtELqHEzMyzQyxJDeaIdLpoLsp')
#test list
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


#--ctypes struct for raylib c struct, must match the exact variable names and types of the struct declared in c file.
class albumEntry(Structure):
    _fields_ = [('title', c.c_char_p),
                ('desc', c.c_char_p),
                ('img', c.c_char_p)]
#--prepare passable array of ctypes struct.
print("\n Preparing array \n")
entryArr = (albumEntry * len(finalList))()
i = 0
for release in finalList:
    path = "coverImages/" + str(release.id) + ".jpeg"
    path2 = "coverImages/" + str(release.id) + ".png"
    if not os.path.isfile(path2):
        urllib.request.urlretrieve(release.images[0]['uri'], path)
        #raylib does not support jpg, convert to png.
        im = Image.open(path)
        im.save(path2)
        os.remove(path)

    #raylib struct: titles, description body (for now just first artist), image/album cover path
    entryArr.__setitem__(i , (albumEntry( release.title.encode("ascii") , release.artists[0].name.encode("ascii")  , path2.encode("ascii") ) ))
    for field_name, field_type in entryArr[i]._fields_:
        print (field_name, getattr(entryArr[i], field_name))
    print("~")
    i += 1

#--Pass the array of structs to c library.
try:
    uiFunc = CDLL("build/ui.so")
    @c.CFUNCTYPE(None)
    def callback():
        print("test callback")
    #call raylib loop
    uiFunc.justgo(callback , entryArr , len(finalList))
except:
    print("No ui.so file found or error.")
