import os

def compile_shaders():
    for root, dirs, files in os.walk("."):
        for file in files:
            if(file.endswith(".spv") or file.endswith(".py")):
                continue

            path = os.path.join(root, file)
            command = f"glslangValidator -V {path} -o {path}.spv"
            with os.popen(command) as proc:
                    output = proc.read()
                    return_code = proc.close()

if __name__ == "__main__":
    compile_shaders()