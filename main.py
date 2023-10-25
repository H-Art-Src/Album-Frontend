import ctypes as c
from ctypes import CDLL
import discogs_client
#from discogs_client import *

d = discogs_client.Client('my_user_agent/1.0', user_token='ithHqUJMHmgldAjtELqHEzMyzQyxJDeaIdLpoLsp')

print(d.release(20017387).artists[0].name, type(d.release(1443762).artists))

uiFunc = CDLL("ui.so")


@c.CFUNCTYPE(None)
def callback():
    print("Yes!!!!")

uiFunc.justgo(callback)

print("end python main")