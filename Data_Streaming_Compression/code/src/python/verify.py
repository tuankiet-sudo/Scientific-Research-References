import sys
from enum import Enum

class Error(Enum):
    INVALID_ALGO=1
    MISSING_PARAM=2
    

if __name__ == "__main__":
    ALGO = sys.argv[1]
    
    if ALGO == "pmc":
        if len(sys.argv) < 2:
            print(Error.MISSING_PARAM.value)
        else:
            print(0)
            
    elif ALGO == "hybrid-pmc":
        if len(sys.argv) < 3:
            print(Error.MISSING_PARAM.value)
        else:
            print(0)
            
    elif ALGO == "normal-equation":
        if len(sys.argv) < 2:
            print(Error.MISSING_PARAM.value)
        else:
            print(0)
            
    else:
        print(Error.INVALID_ALGO.value)
        