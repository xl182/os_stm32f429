import os
import re
import sys
import struct

img_dir = r"Gui\generated\images"
img_path = r"tool\converted_img"
screen_dir = r"Gui\generated"

if not os.path.exists(img_path):
    os.mkdir(img_path)


def process_file_img(f):
    with open(f, "r") as fp:
        content = fp.read()
    with open(f, "w") as fp:
        if not content.startswith("#if 0"):
            content = "#if 0\n" + content + "\n#endif"
        fp.write(content)
    print(f"process {f}")
    pattern = r"lv_image_dsc_t\s\w{1,}"
    res = re.findall(pattern, content)
    if not res:
        print("not found lv_image_dsc_t")
        return
    res = res[0].replace("lv_image_dsc_t _", "")
    img_name, color_format, size = res.rsplit("_", 2)

    print(f"img_name: {img_name}, color_format: {color_format}, size: {size}")

    pattern = r".header.w.?=.?\w*"
    res = re.findall(pattern, content)
    if not res:
        print("not found w")
        return
    w = res[0].replace(".header.w = ", "")

    pattern = r".header.h.?=.?\w*"
    res = re.findall(pattern, content, re.DOTALL)
    if not res:
        print("not found h")
        return
    h = res[0].replace(".header.h = ", "")

    pattern = r'_map\[\]=\{(0x[0-9A-Fa-f]{2},*)*'
    res = re.search(pattern,
                    content.replace("\n", "").replace(" ", ""), re.DOTALL)
    if not res:
        print("not found map")
        return
    map = res.group(0).replace("_map[]={", "").split(",")[:-1]

    f = open(os.path.join(img_path, img_name + ".bin"), "wb")
    if color_format == "RGB565A8":
        f.write(bytes.fromhex("0x01"[2:]))
    elif color_format == "RGB565":
        f.write(bytes.fromhex("0x00"[2:]))
    elif color_format == "ARGB8565":
        f.write(bytes.fromhex("0x02"[2:]))
    f.write(struct.pack('>HH', int(w), int(h)))
    for data in map:
        f.write(bytes.fromhex(data[2:]))
    f.close()
    
def process_file_screen(file_path):
    print("process file:", file_path)
    with open(file_path, "r") as fp:
        content = fp.read()
   
    pattern = r"_src\(ui->\w*, &\w* *.*\);"
    res = re.findall(pattern, content)
    if not res:
        print("not found _src")
        return

    for code_line in res:
        pattern = r"&\w*"
        img_desc = re.findall(pattern, code_line)[0]
        file_name = img_desc.rsplit("_", 2)[0][2:]
        print(f"img file_name: {file_name}")
        content = content.replace(img_desc, f'get_image_desc("{file_name}")')
    with open(file_path, "w") as fp:
        fp.write(content)

def process():
    for f in os.listdir(img_dir):
        if not f.endswith(".c"): continue
        process_file_img(os.path.join(img_dir, f))
        os.remove(os.path.join(img_dir, f))

    for file_name in os.listdir(screen_dir):
        if not file_name.startswith("setup_scr_screen"): continue
        process_file_screen(os.path.join(screen_dir, file_name))

if __name__ == "__main__":
    process()
    