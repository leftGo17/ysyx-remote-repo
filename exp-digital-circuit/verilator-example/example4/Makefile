sim:
	verilator -Wall --cc --exe --trace-fst ./vsrc/*.v ./csrc/*.cpp; \
	$(MAKE) -C obj_dir -f Vtop.mk Vtop; \
	./obj_dir/Vtop;
wave:
	gtkwave -o ./obj_dir/wave.fst
.PHONY: clean
clean:
	rm -rf obj_dir
	rm -rf logs
