import os

def clean():
    for root, dirs, files in os.walk("."):
        for file in files:
            if(file.endswith(".spv")):
                path = os.path.join(root, file)
                os.remove(path)

if __name__ == "__main__":
    clean()