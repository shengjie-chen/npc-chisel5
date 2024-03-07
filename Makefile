BUILD_DIR = ./build

#export PATH := $(PATH):$(abspath ./utils)
export NPC_HOME := $(abspath .)

# project module: RVNoobSim ysyxSoCFull
PRJNAME = RVNoobSim
ifeq ($(PRJNAME), RVNoobSim)
TOPNAME = RVNoobSim
else
TOPNAME = RVNoobTile
endif
SRC_DIR = ./playground/src
# name of object to generate the verilog of design
TOPMODULE_GEN = $(TOPNAME)Gen

# src dir
SRC_CODE_DIR = $(shell find $(abspath $(SRC_DIR)) -maxdepth 2 -type d -name RVNoob)
PACKAGE = $(subst /,.,$(subst $(abspath $(SRC_DIR))/,,$(SRC_CODE_DIR))).Core

# gen dir
GEN_DIR = $(subst $(abspath $(SRC_DIR)),$(BUILD_DIR),$(SRC_CODE_DIR))/$(PRJNAME)# $(subst FROM, TO, TEXT)，即将字符串TEXT中的子串FROM变为TO
OBJ_DIR = $(GEN_DIR)/obj_dir
VERILOG_OBJ_DIR = $(GEN_DIR)/Verilog_Gen
SIM_BIN = $(GEN_DIR)/$(PRJNAME)

# project source
VSRCS = $(shell find $(abspath $(VERILOG_OBJ_DIR)) -name  "*.v")
VSRCS += $(shell find $(abspath $(SRC_CODE_DIR)) -name  "*.v")# add blackbox verilog file
ifeq ($(PRJNAME), ysyxSoCFull)
VSRCS += $(shell find $(abspath $(YSYXSOC_HOME)/perip) -name  "*.v")# add soc verilog file
endif
CSRCS_SIM = $(shell find $(abspath $(SRC_CODE_DIR)) -name  "$(PRJNAME)_sim.cpp")
CSRCS_SIM += $(shell find $(abspath $(SRC_CODE_DIR)) -name  "*.c")
RVNoob_CONFIG = $(shell find $(abspath $(SRC_CODE_DIR)) -name  "RVNoobConfig.scala")

# rules for verilator
WAVE_FORMAT ?= FST #(FST, VCD)
TRACE_FORMAT ?= --trace-fst
WAVE_FILE ?= $(GEN_DIR)/$(PRJNAME).fst
ifeq ($(WAVE_FORMAT), VCD)
    TRACE_FORMAT := --trace
	WAVE_FILE ?= $(GEN_DIR)/$(PRJNAME).vcd
endif

# VERILATOR_CFLAGS += -MMD --build -cc -O3 --x-assign fast --x-initial fast --noassert
VERILATOR_CFLAGS += -cc --exe --build 
VERILATOR_CFLAGS += -O3 --timescale "1ns/1ns" --no-timing
ifeq ($(PRJNAME), ysyxSoCFull)
VERILATOR_CFLAGS += -I$(YSYXSOC_HOME)/perip/uart16550/rtl -I$(YSYXSOC_HOME)/perip/spi/rtl
endif

INCFLAGS = $(addprefix -I, $(INC_PATH))
CFLAGS += $(INCFLAGS) -DTOP_NAME="\"V$(TOPNAME)\""
LDFLAGS += -lSDL2 -lSDL2_image

echo_val:
	@echo SRC_CODE_DIR:$(SRC_CODE_DIR)
	@echo GEN_DIR:$(GEN_DIR)
	@echo OBJ_DIR:$(OBJ_DIR)
	@echo VERILOG_OBJ_DIR:$(VERILOG_OBJ_DIR)
	@echo PACKAGE:$(PACKAGE)

update_config_spmu:
	@if grep -q "//\s*#define SPMU_ENABLE" $(SRC_CODE_DIR)/sim/conf.h; then \
    		sed -i 's/\(val spmu_en: *Boolean = \)true/\1false/' $(RVNoob_CONFIG); \
    		echo "SPMU set to false"; \
    else \
    		sed -i 's/\(val spmu_en: *Boolean = \)false/\1true/' $(RVNoob_CONFIG); \
    		echo "SPMU set to true"; \
    fi

ifeq ($(TOPNAME), RVNoobSim)
update_config:
	sed -i 's/\(val tapeout: *Boolean = \)true/\1false/g' $(RVNoob_CONFIG)
	sed -i 's/\(val soc_sim: *Boolean = \)true/\1false/g' $(RVNoob_CONFIG)
else
update_config:
	sed -i 's/\(val tapeout: *Boolean = \)true/\1false/g' $(RVNoob_CONFIG)
	sed -i 's/\(val soc_sim: *Boolean = \)false/\1true/g' $(RVNoob_CONFIG)
endif

# ifeq ($(TOPNAME),a)
#     # 如果TOPNAME等于"a"，则执行以下指令
#     update_config:
# 		sed -i 's/\(val tapeout: *Boolean = \)true/\1false/g' $(RVNoob_CONFIG)
# 		sed -i 's/\(val soc_sim: *Boolean = \)true/\1false/g' $(RVNoob_CONFIG)
# endif
	

# 将一个总的verilog拆分到多个verilog子文件
VPPFILE ?= $(VERILOG_OBJ_DIR)/$(TOPNAME).v
# SOURCES=$(VERILOG_OBJ_DIR)/$(TOPNAME).v
split_verilog:
	for file in $(VPPFILE); do \
		python3 split_modules.py $$file; \
	done

# use /home/jiexxpu/ysyx/ysyx-workbench/ysyxSoC/ysyx/test
SOC_DIR = $(NPC_HOME)/build/soc
tapeout:
	rm -rf $(SOC_DIR)
	sed -i 's/\(val spmu_en: *Boolean = \)true/\1false/' $(RVNoob_CONFIG)
	sed -i 's/\(val tapeout: *Boolean = \)false/\1true/g' $(RVNoob_CONFIG)
	sed -i 's/\(val soc_sim: *Boolean = \)true/\1false/g' $(RVNoob_CONFIG)
	./mill -i __.test.runMain $(PACKAGE).RVNoobCoreGen
	make verilog_post_processing VPPFILE=$(SOC_DIR)/ysyx_22040495.v

socsim: update_config_spmu
	rm -rf $(VERILOG_OBJ_DIR)
	sed -i 's/\(val tapeout: *Boolean = \)true/\1false/g' $(RVNoob_CONFIG)
	sed -i 's/\(val soc_sim: *Boolean = \)false/\1true/g' $(RVNoob_CONFIG)
	./mill -i __.test.runMain $(PACKAGE).RVNoobTileGen
	make verilog_post_processing VPPFILE=$(VERILOG_OBJ_DIR)/ysyx_22040495.v

verilog: update_config_spmu update_config
	$(call git_commit, "generate $(TOPNAME) verilog")
#	echo $(SRC_CODE_DIR)
	rm -rf $(VERILOG_OBJ_DIR)
	mkdir -p $(VERILOG_OBJ_DIR)
	./mill -i __.test.runMain $(PACKAGE).$(TOPMODULE_GEN) -td $(VERILOG_OBJ_DIR)
#	sed -i 's/val tapeout: Boolean = false/val tapeout: Boolean = true/g' $(RVNoob_CONFIG)
	make verilog_post_processing VPPFILE=$(VERILOG_OBJ_DIR)/$(TOPNAME).v

verilog_post_processing:
	sed -i '/initial begin/,/end /d;/`ifdef/,/`endif/d;/`ifndef/,/`endif/d;/`endif/d' $(VPPFILE)
	sed -i '/firrtl_black_box_resource_files.f/, $$d' $(VPPFILE)
	sed -i '/^\/\//d' $(VPPFILE)
	sed -i '/^$$/N;/^\n$$/D' $(VPPFILE)
	# make split_verilog VPPFILE=$(VPPFILE)

IMG=../am-kernels/tests/cpu-tests/build/dummy-riscv64-npc.bin
#IMG=default
# sdb itrace mtrace ftrace
SDB=sdb_n  # 跳过sdb则改为sdb_n, 否则是sdb_y
ARGS=$(SDB) elf=$(basename $(IMG)).elf diff=../nemu/build/riscv64-nemu-interpreter-so
DISASM_CXXSRC = $(SRC_CODE_DIR)/sim/src/trace/disasm.cc
DISASM_CXXFLAGS = $(shell llvm-config --cxxflags) -fPIE
VERILAOTR_LDFLAGS = $(shell llvm-config --libs) -O3 -pie -ldl -lSDL2
VERILAOTR_LDFLAGS += -fsanitize=address 
VERILAOTR_CFLAGS = -DNPC_HOME=\\\"$(NPC_HOME)\\\" -DGEN_DIR=\\\"$(GEN_DIR)\\\" -O3 -I$(SRC_CODE_DIR)/sim/include
sim_npc_vcd: verilog
	$(call git_commit, "sim $(TOPNAME) RTL") # DO NOT REMOVE THIS LINE!!!
	@echo "Write this Makefile by yourself."
	# sed -i '1i\/* verilator lint_off WIDTH */' $(VERILOG_OBJ_DIR)/$(TOPNAME).v # 让verilator避免检查div处的错误
	mkdir -p $(OBJ_DIR)
	g++ -O2 -MMD -Wall -Werror -save-temps $(DISASM_CXXFLAGS) -c -o $(abspath $(OBJ_DIR)/disasm.o) $(DISASM_CXXSRC)
	verilator $(VERILATOR_CFLAGS) --top $(TOPNAME) --Mdir $(OBJ_DIR) $(TRACE_FORMAT) \
		$(VSRCS) $(CSRCS_SIM) $(abspath $(OBJ_DIR)/disasm.o) \
		-o $(abspath $(SIM_BIN)) $(addprefix -LDFLAGS ,$(VERILAOTR_LDFLAGS))  $(addprefix -CFLAGS ,$(VERILAOTR_CFLAGS))
	$(SIM_BIN) $(IMG) $(ARGS)
	gtkwave $(WAVE_FILE)

sim_npc_vcd_without_gtk: verilog
	$(call git_commit, "sim $(TOPNAME) RTL") # DO NOT REMOVE THIS LINE!!!
	@echo "Write this Makefile by yourself."
	#sed -i '1i\/* verilator lint_off WIDTH */' $(VERILOG_OBJ_DIR)/$(TOPNAME).v # 让verilator避免检查div处的错误
	mkdir -p $(OBJ_DIR)
	g++ -O3 -MMD -Wall -Werror $(DISASM_CXXFLAGS) -c -o $(abspath $(OBJ_DIR)/disasm.o) $(DISASM_CXXSRC)
	verilator $(VERILATOR_CFLAGS) --top $(TOPNAME) --Mdir $(OBJ_DIR) \
		$(VSRCS) $(CSRCS_SIM) $(abspath $(OBJ_DIR)/disasm.o) \
		-o $(abspath $(SIM_BIN)) $(addprefix -LDFLAGS ,$(VERILAOTR_LDFLAGS))  $(addprefix -CFLAGS ,$(VERILAOTR_CFLAGS))
	$(SIM_BIN) $(IMG) $(ARGS)

sim_npc_vcd_without_regen:
	$(call git_commit, "sim $(TOPNAME) RTL") # DO NOT REMOVE THIS LINE!!!
	@echo "Write this Makefile by yourself."
	mkdir -p $(OBJ_DIR)
	g++ -O2 -MMD -Wall -Werror -save-temps $(DISASM_CXXFLAGS) -c -o $(abspath $(OBJ_DIR)/disasm.o) $(DISASM_CXXSRC)
	verilator -$(VERILATOR_CFLAGS) --top $(TOPNAME) --Mdir $(OBJ_DIR) $(TRACE_FORMAT) \
		$(VSRCS) $(CSRCS_SIM) $(abspath $(OBJ_DIR)/disasm.o) \
		-o $(abspath $(SIM_BIN)) $(addprefix -LDFLAGS ,$(VERILAOTR_LDFLAGS))  $(addprefix -CFLAGS ,$(VERILAOTR_CFLAGS))
	$(SIM_BIN) $(IMG) $(ARGS)
	gtkwave $(WAVE_FILE)

sim_npc_vcd_without_regen_gtk:
	$(call git_commit, "sim $(TOPNAME) RTL") # DO NOT REMOVE THIS LINE!!!
	@echo "Write this Makefile by yourself."
	# sed -i '1i\/* verilator lint_on UNOPTTHREADS */' $(VERILOG_OBJ_DIR)/$(TOPNAME).v # 让verilator避免检查threads error
	mkdir -p $(OBJ_DIR)
	g++ -O3 -MMD -Wall -Werror $(DISASM_CXXFLAGS) -c -o $(abspath $(OBJ_DIR)/disasm.o) $(DISASM_CXXSRC)
	verilator $(VERILATOR_CFLAGS) --top $(TOPNAME) --Mdir $(OBJ_DIR) \
		$(VSRCS) $(CSRCS_SIM) $(abspath $(OBJ_DIR)/disasm.o) \
		-o $(abspath $(SIM_BIN)) $(addprefix -LDFLAGS ,$(VERILAOTR_LDFLAGS))  $(addprefix -CFLAGS ,$(VERILAOTR_CFLAGS))
	$(SIM_BIN) $(IMG) $(ARGS)

gtk:
	gtkwave $(WAVE_FILE)

perf_sim_npc_nanoslite_pal:
	make split_verilog
	mkdir -p $(OBJ_DIR)
	verilator --prof-cfuncs --top $(TOPNAME) -O3 --cc $(VSRCS) --Mdir $(OBJ_DIR) --exe --build $(CSRCS_SIM) -o $(abspath $(SIM_BIN)) \
		$(addprefix -LDFLAGS ,$(VERILAOTR_LDFLAGS))  $(addprefix -CFLAGS ,$(VERILAOTR_CFLAGS))
	# $(SIM_BIN) $(IMG)
	$(SIM_BIN) /home/jiexxpu/ysyx/ysyx-workbench/nanos-lite/build/nanos-lite-riscv64-npc.bin
	gprof $(SIM_BIN) gmon.out > gprof.out
	verilator_profcfunc gprof.out > report.out

#	gprof $(SIM_BIN) gmon.out > gprof.out
#	verilator_profcfunc gprof.out > report.out
# perf record /home/jiexxpu/ysyx/ysyx-workbench/npc/build/RVNoob/RVNoob /home/jiexxpu/ysyx/ysyx-workbench/nanos-lite/build/nanos-lite-riscv64-npc.bin

# sim_npc_vcd_without_regen_gdb:
# 	$(call git_commit, "sim $(TOPNAME) RTL") # DO NOT REMOVE THIS LINE!!!
# 	@echo "Write this Makefile by yourself."
# 	mkdir -p $(OBJ_DIR)
# 	g++ -O2 -MMD -Wall -Werror -save-temps $(DISASM_CXXFLAGS) -c -o $(abspath $(OBJ_DIR)/disasm.o) $(DISASM_CXXSRC)
# 	verilator --cc $(VSRCS) $(TRACE_FORMAT) --exe --build --gdb $(CSRCS_SIM) $(abspath $(OBJ_DIR)/disasm.o) -o $(abspath $(SIM_BIN)) $(addprefix -LDFLAGS ,$(VERILAOTR_LDFLAGS))  
# 	gdb $(abspath $(SIM_BIN)) --args $(IMG) $(ARGS)
# 	gtkwave $(WAVE_FILE)
#$(abspath $(OBJ_DIR)/disasm.o)

test:
	mill -i __.test

help:
	mill -i __.test.runMain Elaborate --help

compile:
	mill -i __.compile

bsp:
	mill -i mill.bsp.BSP/install

reformat:
	mill -i __.reformat

checkformat:
	mill -i __.checkFormat

clean:
	-rm -rf $(BUILD_DIR)
	-rm *.ii *.s *.o *.d *.json *.v *.tmp

clean_object:
	rm -rf $(OBJ_DIR)

.PHONY: test verilog help compile bsp reformat checkformat clean clean_object


-include ../Makefile
