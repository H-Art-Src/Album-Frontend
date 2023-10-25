from ctypes import *
from discogs_client import *
from multiprocessing import Process

so_ui = "ui.so"
uiFunc = CDLL(so_ui)

def ui():
    uiFunc.main()

def uiVar():
    uiFunc.discogs()

if __name__ == "__main__":
    p1 = Process(target=ui)
    p1.start()
    p2 = Process(target=uiVar)
    p2.start()
    p1.join()
    p2.join()

#uiFunc.init()

#while uiFunc.UIrender():
#    print("running")

#uiFunc.deInit()


print("end python main")