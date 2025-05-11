default: sim
sim:
	verilator -Wall --trace -cc our.v --exe sim_main.cpp
	make -C obj_dir -f Vour.mk Vour
.PHONY: clean
clean:
	rm -rf obj_dir