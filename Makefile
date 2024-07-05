BUILD_DIR = ./build

#export PATH := $(PATH):$(abspath ./utils)
export NPC_HOME := $(abspath .)

### Extract instruction set architecture (`ISA`) and platform from `$ARCH`. Example: `ARCH=x86_64-qemu -> ISA=x86_64; PLATFORM=qemu`
ARCH?=riscv64-npcsoc
ARCH_SPLIT = $(subst -, ,$(ARCH))
ISA        = $(word 1,$(ARCH_SPLIT))
PLATFORM   = $(word 2,$(ARCH_SPLIT))
#$(info ARCH=$(ARCH) ISA=$(ISA) PLATFORM=$(PLATFORM))

### name val
ifeq ($(PLATFORM), npc)
GENTOPNAME := RVNoobSim
SIMTOPNAME := RVNoobSim
else
GENTOPNAME := RVNoobTile
SIMTOPNAME := ysyxSoCFull
endif
SRC_DIR = ./playground/src
# name of object to generate the verilog of design
TOPMODULE_GEN := $(GENTOPNAME)Gen

### src dir
SRC_CODE_DIR = $(shell find $(abspath $(SRC_DIR)) -maxdepth 2 -type d -name RVNoob)
CSRC_DIR = $(SRC_CODE_DIR)/sim
PACKAGE = $(subst /,.,$(subst $(abspath $(SRC_DIR))/,,$(SRC_CODE_DIR))).Core

### gen dir
GEN_DIR = $(subst $(abspath $(SRC_DIR)),$(BUILD_DIR),$(SRC_CODE_DIR))/$(PLATFORM)# $(subst FROM, TO, TEXT)，即将字符串TEXT中的子串FROM变为TO
OBJ_DIR = $(GEN_DIR)/obj_dir
VERILOG_OBJ_DIR = $(GEN_DIR)/Verilog_Gen
SIM_BIN = $(GEN_DIR)/$(SIMTOPNAME)

### project hardware source
VSRCS = $(shell find $(abspath $(VERILOG_OBJ_DIR)) -name  "*.v")
VSRCS += $(shell find $(abspath $(SRC_CODE_DIR)) -name  "*.v")# add blackbox verilog file
ifeq ($(PLATFORM), npcsoc)
VSRCS += $(shell find $(abspath $(YSYXSOC_HOME)/perip) -name  "*.v")# add soc verilog file
VSRCS += $(shell find $(abspath $(YSYXSOC_HOME)/build) -name  "$(SIMTOPNAME).v")# add soc verilog file
endif
SCALASRCS = $(shell find $(abspath $(SRC_CODE_DIR)) -name  "*.scala")
RVNoob_CONFIG = $(shell find $(abspath $(SRC_CODE_DIR)) -name  "RVNoobConfig.scala")

### project software source
CSRCS_SIM = $(shell find $(abspath $(CSRC_DIR)) -name  "$(SIMTOPNAME)_sim.cpp")
ifeq ($(PLATFORM), npcsoc)
CSRCS_SIM += $(shell find $(abspath $(CSRC_DIR)) -name  "*.c" ! -name "difftest.c")
endif
ifeq ($(PLATFORM), npc)
CSRCS_SIM += $(shell find $(abspath $(CSRC_DIR)) -name  "*.c")
endif
SIM_CONFIG = $(SRC_CODE_DIR)/sim/include/conf.h

### wave for verilator
WAVE_FORMAT ?= FST #(FST, VCD)
TRACE_FORMAT ?= --trace-fst
WAVE_FILE ?= $(GEN_DIR)/$(SIMTOPNAME).fst
ifeq ($(WAVE_FORMAT), VCD)
    TRACE_FORMAT := --trace
	WAVE_FILE ?= $(GEN_DIR)/$(SIMTOPNAME).vcd
endif

### FLAGS for verilator
# VERILATOR_FLAGS += -MMD --build -cc -O3 --x-assign fast --x-initial fast --noassert --autoflush
VERILATOR_FLAGS += --autoflush
VERILATOR_FLAGS += -cc --exe --build
VERILATOR_FLAGS += -O3 --timescale "1ns/1ns" --no-timing
ifeq ($(PLATFORM), npcsoc)
VERILATOR_FLAGS += -I$(YSYXSOC_HOME)/perip/uart16550/rtl -I$(YSYXSOC_HOME)/perip/spi/rtl
endif
ifeq ($(shell grep -q "^#define CONFIG_DUMPWAVE" $(SIM_CONFIG) && echo yes || echo no),yes)
VERILATOR_FLAGS += $(TRACE_FORMAT)
OPEN_GTK = gtkwave $(WAVE_FILE) $(WAVE_SAVE_FILE)
else
OPEN_GTK =
endif

### FLAGS for CSIM
DISASM_CXXSRC = $(SRC_CODE_DIR)/sim/src/trace/disasm.cc
DISASM_CXXFLAGS = $(shell llvm-config --cxxflags) -fPIE
VERILAOTR_LDFLAGS = $(shell llvm-config --libs) -O3 -pie -ldl -lSDL2
VERILAOTR_LDFLAGS += -fsanitize=address
VERILATOR_CFLAGS = -DNPC_HOME=\\\"$(NPC_HOME)\\\" -DGEN_DIR=\\\"$(GEN_DIR)\\\" -O3 -I$(SRC_CODE_DIR)/sim/include
# ifeq ($(PRJNAME), ysyxSoCFull)
# VERILATOR_CFLAGS += -I$(YSYXSOC_HOME)/perip/uart16550/rtl -I$(YSYXSOC_HOME)/perip/spi/rtl
# endif

### args for SIM EXE order: sdb elf diff other
IMG=../am-kernels/tests/cpu-tests/build/dummy-riscv64-npc.bin
#IMG=default
SDB=sdb=no  # 跳过sdb则改为sdb=no, 否则是sdb=yes
ARGS=$(SDB) elf=$(basename $(IMG)).elf diff=../nemu/build/riscv64-nemu-interpreter-so
ifeq ($(PLATFORM), npcsoc)
# ARGS += load=$(TEST_DIR)/char-test.bin
ARGS += 
endif

echo_val:
	@#echo PLATFORM:$(PLATFORM)SCALASRCS
	@#echo SCALASRCS:$(SCALASRCS)
	@#echo SRC_CODE_DIR:$(SRC_CODE_DIR)
	@#echo GEN_DIR:$(GEN_DIR)
	@#echo OBJ_DIR:$(OBJ_DIR)
	@#echo VERILOG_OBJ_DIR:$(VERILOG_OBJ_DIR)
	@#echo PACKAGE:$(PACKAGE)
	@#echo YSYXSOC_HOME:$(YSYXSOC_HOME)
	@#echo VSRCS:$(VSRCS)
	@echo ARGS:$(ARGS)

### >>>>>>>>>>>>>>>> soc tapeout project
SOC_DIR = $(NPC_HOME)/build/soc
tapeout:
	rm -rf $(SOC_DIR)
	sed -i 's/\(val spmu_en: *Boolean = \)true/\1false/' $(RVNoob_CONFIG)
	sed -i 's/\(val tapeout: *Boolean = \)false/\1true/g' $(RVNoob_CONFIG)
	sed -i 's/\(val soc_sim: *Boolean = \)true/\1false/g' $(RVNoob_CONFIG)
	./mill -i __.test.runMain $(PACKAGE).RVNoobCoreGen
	make verilog_post_processing VPPFILE=$(SOC_DIR)/ysyx_22040495.v

### >>>>>>>>>>>>>>>> general sim project
update_config_spmu:
	@if grep -q "//\s*#define SPMU_ENABLE" $(SIM_CONFIG); then \
		echo -e "\n[scala config]: SPMU set to false"; \
		if grep -q "val spmu_en: *Boolean = true" $(RVNoob_CONFIG); then \
    		sed -i 's/\(val spmu_en: *Boolean = \)true/\1false/' $(RVNoob_CONFIG); \
		fi \
    else \
    	echo -e "\n[scala config]: SPMU set to true"; \
		if grep -q "val spmu_en: *Boolean = false" $(RVNoob_CONFIG); then \
    		sed -i 's/\(val spmu_en: *Boolean = \)false/\1true/' $(RVNoob_CONFIG); \
		fi \
    fi

ifeq ($(PLATFORM), npc)
update_config_soc:
	@if grep -q "val soc_sim: *Boolean = true" $(RVNoob_CONFIG); then \
    	sed -i 's/\(val soc_sim: *Boolean = \)true/\1false/' $(RVNoob_CONFIG); \
	fi
    @echo -e "[scala config]: soc_sim set to false";
	@sed -i 's/^\s*#define SOC_SIM/\/\/ #define SOC_SIM/g' $(SIM_CONFIG)
	@echo -e "[C++   config]: SOC_SIM set to false\n"

socgen:

else
update_config_soc:
	@if grep -q "val soc_sim: *Boolean = false" $(RVNoob_CONFIG); then \
   		sed -i 's/\(val soc_sim: *Boolean = \)false/\1true/' $(RVNoob_CONFIG); \
	fi
	@echo -e "[scala config]: soc_sim set to true";
	@sed -i 's/^\s*\/\/\s*#define SOC_SIM/#define SOC_SIM/g' $(SIM_CONFIG)
	@echo -e "[C++   config]: SOC_SIM set to true"
	@sed -i 's/^\s*#define CONFIG_DIFFTEST/\/\/ #define CONFIG_DIFFTEST/g' $(SIM_CONFIG)
	@echo "[C++   config]: CONFIG_DIFFTEST set to false"

socgen:
	make -C $(YSYXSOC_HOME) verilog
endif	

update_config: update_config_spmu update_config_soc
	@if grep -q "val tapeout: *Boolean = true" $(RVNoob_CONFIG); then \
		sed -i 's/\(val tapeout: *Boolean = \)true/\1false/g' $(RVNoob_CONFIG); \
	fi
	@echo "[scala config]: tapeout set to false"

### 将一个总的verilog拆分到多个verilog子文件
VPPFILE ?= $(VERILOG_OBJ_DIR)/$(GENTOPNAME).v
split_verilog:
	for file in $(VPPFILE); do \
		python3 split_modules.py $$file; \
	done

verilog_post_processing:
	@#sed -i '/initial begin/,/end /d;/`ifdef/,/`endif/d;/`ifndef/,/`endif/d;/`endif/d' $(VPPFILE)
	@sed -i '/firrtl_black_box_resource_files.f/, $$d' $(VPPFILE)
	@sed -i '/^\/\//d' $(VPPFILE)
	@sed -i '/^$$/N;/^\n$$/D' $(VPPFILE)
	@# make split_verilog VPPFILE=$(VPPFILE)

ifeq ($(PLATFORM), npcsoc)
GENTOPNAME:=ysyx_22040495
WAVE_SAVE_FILE = wavefile/soc/init.gtkw
endif

verilog: $(VERILOG_OBJ_DIR)/$(GENTOPNAME).v

$(VERILOG_OBJ_DIR)/$(GENTOPNAME).v: $(SCALASRCS)
	rm -rf $(VERILOG_OBJ_DIR)
	mkdir -p $(VERILOG_OBJ_DIR)
	./mill -i __.test.runMain $(PACKAGE).$(TOPMODULE_GEN) -td $(VERILOG_OBJ_DIR)
	make verilog_post_processing VPPFILE=$(VERILOG_OBJ_DIR)/$(GENTOPNAME).v

sim_npc_vcd: update_config socgen verilog
	mkdir -p $(OBJ_DIR)
	g++ -O3 -MMD -Wall -Werror -save-temps $(DISASM_CXXFLAGS) -c -o $(abspath $(OBJ_DIR)/disasm.o) $(DISASM_CXXSRC)
	verilator $(VERILATOR_FLAGS) --top $(SIMTOPNAME) --Mdir $(OBJ_DIR) \
		$(VSRCS) \
		$(CSRCS_SIM) $(abspath $(OBJ_DIR)/disasm.o) \
		-o $(abspath $(SIM_BIN)) $(addprefix -LDFLAGS ,$(VERILAOTR_LDFLAGS))  $(addprefix -CFLAGS ,$(VERILATOR_CFLAGS))
	$(SIM_BIN) $(IMG) $(ARGS)
	$(OPEN_GTK)

gtk:
	gtkwave $(WAVE_FILE)

perf_sim_npc_nanoslite_pal:
	make split_verilog
	mkdir -p $(OBJ_DIR)
	verilator --prof-cfuncs --top $(SIMTOPNAME) -O3 --cc $(VSRCS) --Mdir $(OBJ_DIR) --exe --build $(CSRCS_SIM) -o $(abspath $(SIM_BIN)) \
		$(addprefix -LDFLAGS ,$(VERILAOTR_LDFLAGS))  $(addprefix -CFLAGS ,$(VERILATOR_CFLAGS))
	# $(SIM_BIN) $(IMG)
	$(SIM_BIN) /home/jiexxpu/ysyx/ysyx-workbench/nanos-lite/build/nanos-lite-riscv64-npc.bin
	gprof $(SIM_BIN) gmon.out > gprof.out
	verilator_profcfunc gprof.out > report.out

TEST_DIR = ./playground/src/RVNoob/sim/app/ctest
char-test:
	riscv64-unknown-linux-gnu-gcc -ffreestanding -nostdlib -march=rv64im_zicsr_zifencei -static \
		-mabi=lp64 -Wl,-Ttext=0 -O2 $(TEST_DIR)/char-test.c -o $(TEST_DIR)/char-test
	# riscv64-linux-gnu-gcc -e _start -pie -nostdlib -nolibc -nodefaultlibs -Wl,-Ttext=0 $(TEST_DIR)/char-test.o -o $(TEST_DIR)/char-test
	riscv64-linux-gnu-objcopy -O binary --only-section=.text $(TEST_DIR)/char-test $(TEST_DIR)/char-test.bin
	riscv64-linux-gnu-objdump -D $(TEST_DIR)/char-test > $(TEST_DIR)/char-test.txt
	hexdump -C $(TEST_DIR)/char-test.bin > $(TEST_DIR)/char-test-bin.txt

#	gprof $(SIM_BIN) gmon.out > gprof.out
#	verilator_profcfunc gprof.out > report.out
# perf record /home/jiexxpu/ysyx/ysyx-workbench/npc/build/RVNoob/RVNoob /home/jiexxpu/ysyx/ysyx-workbench/nanos-lite/build/nanos-lite-riscv64-npc.bin

# sim_npc_vcd_without_regen_gdb:
# 	$(call git_commit, "sim $(GENTOPNAME) RTL") # DO NOT REMOVE THIS LINE!!!
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
