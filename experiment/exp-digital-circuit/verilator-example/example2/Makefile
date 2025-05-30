default: sim
sim:
	verilator -Wall --cc --exe top.v sim.cpp ;\
	$(MAKE) -C obj_dir -f Vtop.mk Vtop
.PHONY: clean
clean:
	rm -rf obj_dir
	rm -rf logs
