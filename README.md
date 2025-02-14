# PRR
Pip, RIOT and rBPF

The artifact is structured as follows:

* RIOT/ contains our RIOT fork
  * To build
        cd RIOT/examples/default
        BOARD=dwm1001_pip make
  * The  dwm1001_pip board is a configuration of RIOT for the dwm1001 board with PIP para-virtualization
  * XiPFS sources are in RIOT/sys/fs/xipfs and RIOT/sys/include/fs/xipfs.h
  * We added new shell commands (RIOT/sys/shell/cmds) to support XiPFS operations (file creation, exec, etc.)
  * The pyterm utility was extended to support these commands
  * Baudrate was lowered to 78600 to prevent inconsistencies
* pip-mpu-separate/ contains PIP compiled for a Cortex M4
  * To build run make
  * Makefile looks for root.elf and pip.elf files, paths may be specified in Makefile.config
  * Compiling pip.elf is tedious (requires a specific Coq toolchain), we provided a pip.bin which is already well-formed for ARM Cortex M4
  * root.elf could be any freestanding binary, not just our RIOT-fork
 * 06-rbpf-bench/ contains our rBPF VM for time measurements
   * Run make to build
   * Inside pyterm run "exec rbpf-bench.bin 10000 fletcher32.rbpf data.txt" to
     * Load the rbpf-bench.bin binary and initialize it
     * Execute 10000 times the fletcher32.rbpf bytecode with input file data.txt
     * All files must be inside the XiPFS filesystem
   * exec is a pyterm command that uses the XiPFS exec command (no memory safety; for safety use safe_exec)
 * 08-fletcher32/ contain the fletcher32 rBPF benchmark
   * Run make to build
   * Yields a fletcher32.rbpf bytecode file
   * Upload to your board's XiPFS via the pyterm put command
