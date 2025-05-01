import os

ext = '.bin'

# 获取当前文件的目录
current_dir = os.path.dirname(os.path.abspath(__file__))

# 拼接目标目录
directory = "../main/files/frames"
full_path = os.path.join(current_dir, directory)

# 规范化路径（处理 `..` 和 `.`）
full_path = os.path.normpath(full_path)

print("工作目录: ",full_path)


# Loop through the files in the directory
for filename in os.listdir(full_path):
    if filename.startswith("frame_") and "_delay" in filename:
        # Extract the frame part before "_delay"
        new_filename = filename.split("_delay")[0] + ext

        # Full path to the current file
        file_path = os.path.join(full_path, filename)
        # print(file_path)

        # Full path for the new file name
        new_file_path = os.path.join(full_path, new_filename)

        # Rename the file
        os.rename(file_path, new_file_path)
        print(f"Renamed {filename} to {new_filename}")

print("重命名完成-------------")
