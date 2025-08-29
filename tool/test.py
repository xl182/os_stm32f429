with open(r"Gui\generated\guider_fonts\lv_font_montserratMedium_16.c",
          "r",
          encoding="utf-8") as f:
    content = f.read()
    print(content)

import re

pattern = r"U\+\w*"
res = re.findall(pattern, content)
for r in res:
    print(r.replace("U+", "0x"), end=",")
