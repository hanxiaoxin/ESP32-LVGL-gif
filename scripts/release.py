import sys
import os
import json
import zipfile

# 切换到项目根目录
os.chdir(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

def get_project_version():
    with open("CMakeLists.txt") as f:
        for line in f:
            if line.startswith("set(PROJECT_VER"):
                return line.split("\"")[1].split("\"")[0].strip()
    return None

def merge_bin():
    if os.system("idf.py merge-bin") != 0:
        print("merge bin failed")
        sys.exit(1)

def zip_bin(board_type, project_version):
    if not os.path.exists("releases"):
        os.makedirs("releases")
    output_path = f"releases/v{project_version}.zip"
    if os.path.exists(output_path):
        os.remove(output_path)
    with zipfile.ZipFile(output_path, 'w', compression=zipfile.ZIP_DEFLATED) as zipf:
        zipf.write("build/merged-binary.bin", arcname="merged-binary.bin")
    print(f"zip bin to {output_path} done")
    

def release_current():
    merge_bin()
    project_version = get_project_version()
    print("project version:", project_version)
    zip_bin("qrcode", project_version)

if __name__ == "__main__":
    release_current()
