include ../Makefile

all:
	@echo "Write this Makefile by your self."

sim:
	$(call git_commit, "sim RTL");  \
	verilator -Wall --cc --exe --trace-fst ./vsrc/top.v ./csrc/sim.cpp; \
	$(MAKE) -C obj_dir -f Vtop.mk Vtop; \
	./obj_dir/Vtop;

wave:
	gtkwave -o ./obj_dir/wave.fst

.PHONY: clean
clean:
	rm -rf obj_dir
	rm -rf logs
