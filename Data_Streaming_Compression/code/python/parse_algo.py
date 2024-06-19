import os
import sys
import json

def create_dir(path):
    if not os.path.exists(os.path.dirname(path)):
        os.makedirs(os.path.dirname(path))
    
    f = open(path, 'w+')
    f.close()

if __name__ == "__main__":
    with open(sys.argv[1], mode="r") as f:
        config = json.load(f)
    
    temp = "input:{}\n@\n".format(config["input"])
    for method in config["methods"]:
        for key in method:
            if key == "method":
                temp += "{}:{}\n".format(key, method[key].lower())
            elif key == "mode":
                temp += "{}:{}\n".format(key, method[key].lower())
            elif key == "out_appro" or key == "out_model":
                create_dir(method[key])
                temp += "{}:{}\n".format(key, method[key])
            else:
                temp += "{}:{}\n".format(key, method[key])
                
        temp += "!\n"
        
    with open(".algo_config.temp", mode="w") as f:
        f.write(temp[:-1])
        
    print(".algo_config.temp")