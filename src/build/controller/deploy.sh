make -j4
sudo openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 3000" -c " program controller.elf verify reset exit"
