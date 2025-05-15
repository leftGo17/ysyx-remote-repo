PROJ_PATH = /home/zjp/code/ysyx-workbench/exp-digital-circuit/ex0-flowed-led/sta

DESIGN ?= top
SDC_FILE ?= $(PROJ_PATH)/top.sdc
RTL_FILES ?= $(shell find /home/zjp/code/ysyx-workbench/exp-digital-circuit/ex0-flowed-led/vsrc/ -name "*.v")
export CLK_FREQ_MHZ ?= 500

RESULT_DIR = $(PROJ_PATH)/result/$(DESIGN)-$(CLK_FREQ_MHZ)MHz
SCRIPT_DIR = /home/zjp/code/yosys-sta/scripts
NETLIST_SYN_V   = $(RESULT_DIR)/$(DESIGN).netlist.syn.v
NETLIST_FIXED_V = $(RESULT_DIR)/$(DESIGN).netlist.fixed.v
TIMING_RPT = $(RESULT_DIR)/$(DESIGN).rpt


syn: $(NETLIST_SYN_V)
$(NETLIST_SYN_V): $(RTL_FILES) $(SCRIPT_DIR)/yosys.tcl
	mkdir -p $(@D)
	echo tcl $(SCRIPT_DIR)/yosys.tcl $(DESIGN) \"$(RTL_FILES)\" $@ | yosys -l $(@D)/yosys.log -s -

fix-fanout: $(NETLIST_FIXED_V)
$(NETLIST_FIXED_V): $(SCRIPT_DIR)/fix-fanout.tcl $(SDC_FILE) $(NETLIST_SYN_V)
	/home/zjp/code/yosys-sta/bin/iEDA -script $^ $(DESIGN) $@ 2>&1 | tee $(RESULT_DIR)/fix-fanout.log

sta: $(TIMING_RPT)
$(TIMING_RPT): $(SCRIPT_DIR)/sta.tcl $(SDC_FILE) $(NETLIST_FIXED_V)
	/home/zjp/code/yosys-sta/bin/iEDA -script $^ $(DESIGN) 2>&1 | tee $(RESULT_DIR)/sta.log

clean:
	-rm -rf result/

.PHONY: init syn fix-fanout sta clean
.PHONY: print_proj_path
print:
	@echo "PROJ_PATH:$(PROJ_PATH)"
	@echo "SDC_FILE:$(SDC_FILE)"
	@echo "RTL_FILES:$(RTL_FILES)"
	@echo "RESULT_DIR:$(RESULT_DIR)"