sim:
	verilator -Wall --cc --exe --trace top.v sim.cpp ;\
	$(MAKE) -C obj_dir -f Vtop.mk Vtop ;\
	./obj_dir/Vtop;
wave:
	gtkwave ./obj_dir/wave.vcd
.PHONY: clean
clean:
	rm -rf obj_dir
	rm -rf logs
