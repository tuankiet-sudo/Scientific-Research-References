import sys
import json

if __name__ == "__main__":
    with open(sys.argv[1], mode="r") as f:
        config = json.load(f)
    
    if 'str' in str(type(config[sys.argv[2]])):
        print(config[sys.argv[2]])
    elif 'list' in str(type(config[sys.argv[2]])):        
        print(' '.join(ele.lower() for ele in config[sys.argv[2]]))