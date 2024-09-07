import os
import subprocess

def compile_shaders():
    for root, dirs, files in os.walk("."):
        for file in files:
            if(file.endswith(".spv") or file.endswith(".py")):
                continue

            path = os.path.join(root, file)
            command = ["glslangValidator", "-V", path, "-o", f"{path}.spv"]
            try:
                result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

                if result.returncode != 0:
                    print(f"Error compiling {path}:\n{result.stdout}")
                else:
                    print(f"Successfully compiled {path}")

            except Exception as e:
                print(f"Failed to run command: {command}")
                print(e)

if __name__ == "__main__":
    compile_shaders()