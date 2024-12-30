import sys
from enum import Enum

class Error(Enum):
    INVALID_ALGO=1
    MISSING_PARAM=2
    INVALID_PARAM=3
    

if __name__ == "__main__":
    ALGO = sys.argv[1]
    
    if ALGO == "pmc":
        if len(sys.argv) < 3:
            print(Error.MISSING_PARAM.value)
        else:
            MODE = sys.argv[2]
            if MODE != "midrange" and MODE != "mean":
                print(Error.INVALID_PARAM.value)
            else:
                print(0)
            
    elif ALGO == "hybrid-pca":
        if len(sys.argv) < 4:
            print(Error.MISSING_PARAM.value)
        else:
            if not sys.argv[2].isdigit() or not sys.argv[3].isdigit():
                print(Error.INVALID_PARAM.value)
            else:
                print(0)
            
    elif ALGO == "normal-equation":
        if len(sys.argv) < 4:
            print(Error.MISSING_PARAM.value)
        else:
            MODE = sys.argv[2]
            if MODE != "individual" and MODE != "accumulate":
                print(Error.INVALID_PARAM.value)
            elif not sys.argv[3].isdigit():
                print(Error.INVALID_PARAM.value)
            else:
                print(0)
    
    elif ALGO == "optimal-pla":
        print(0)
        
    elif ALGO == "swing-filter":
        print(0)
        
    elif ALGO == "slide-filter":
        print(0)
        
    else:
        print(Error.INVALID_ALGO.value)
        