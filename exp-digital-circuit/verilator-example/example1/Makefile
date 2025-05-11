# default: sim
# sim:
# 	verilator -Wall --trace -cc top.v --exe sim_main.cpp --coverage \ 
# 	make -C obj_dir -f Vtop.mk Vtop
# .PHONY: clean
# clean:
# 	rm -rf obj_dir
# 	rm -rf logs
VERILATOR = verilator
# Generate C++ in executable form
VERILATOR_FLAGS += -cc --exe
# Generate makefile dependencies (not shown as complicates the Makefile)
#VERILATOR_FLAGS += -MMD
# Optimize
VERILATOR_FLAGS += -x-assign fast
# Warn abount lint issues; may not want this on less solid designs
VERILATOR_FLAGS += -Wall
# Make waveforms
VERILATOR_FLAGS += --trace
# Check SystemVerilog assertions
VERILATOR_FLAGS += --assert
# Generate coverage analysis
VERILATOR_FLAGS += --coverage
# Run Verilator in debug mode
#VERILATOR_FLAGS += --debug
# Add this trace to get a backtrace in gdb
#VERILATOR_FLAGS += --gdbbt

# Input files for Verilator
VERILATOR_INPUT = -f input.vc top.v sim_main.cpp
run:
	$(VERILATOR) $(VERILATOR_FLAGS) $(VERILATOR_INPUT)

	$(MAKE) -j -C obj_dir -f ../Makefile_obj


clean:
	rm -rf obj_dir
	rm -rf logs