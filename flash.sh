make -j
openocd -f interface/stlink.cfg -f target/stm32g4x.cfg -c "program build/RMCONTROL-G431.bin 0x08000000 verify exit"