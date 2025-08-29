python tool/replace_code.py

del STM32F429XX_FLASH.ld
del .mxproject
del text_processor.log
mkdir build
cd build
del OS.elf

cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build . -j 16
cd ..
openocd -f interface/stlink.cfg -f stm32429i_eval_stlink.cfg -c "program build/OS.elf reset exit"
