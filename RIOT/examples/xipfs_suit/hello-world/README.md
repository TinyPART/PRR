# Post-issuance binary generation template

## Context

In today's technology-driven world, deploying post-issuance software is
crucial. This process provides numerous advantages for manufacturers,
retailers, and consumers. It allows for the execution of single-shot
software, enables software personalization, opens up support for
third-party software, and facilitates software provisioning.
Nevertheless, implementing post-issuance software deployment in the
Internet of Things (IoT) ecosystem comes with challenges such as
ecosystem's diversity, resource limitations, security issues, and more.

## Post-issuance code generation for the IoT

The deployment of post-issuance software in IoT devices requires the
generation of position-independent code, which can run independently of
its location in the target's memory. This is because the exact memory
location of the code cannot be predicted. IoT devices typically have two
types of memory: non-volatile memory (often flash-based) and volatile
memory (usually RAM). These constraints raise two main issues.

Firstly, the compiler must be instructed to access global variables
through a table called the Global Offset Table (GOT) in ELF files,
rather than using program counter-relative instruction sequences. The
GOT contains the offsets of global variables relative to the beginning
of the generated binary file. Before execution, the GOT must be copied
from flash to RAM, and its offsets must be patched to match the actual
addresses of global variables.

Secondly, all global variables containing addresses of other global
variables must also be patched. Since memory addresses for global
variables are unknown at compile-time, relocation tables are needed to
perform this task. Relocation tables in ELF files contain information
about the locations in the binary code that need to be modified at load
time by the CRT0.

To address these issues, the compiler must be instructed to generate the
correct instruction sequence to access global variables using the
following arguments: `-fPIC`, `-msingle-pic-base`, and
`-mpic-register=r10`. Additionally, the GOT can be generated with the
argument `-mno-pic-data-is-text-relative`, and relevant information such
as relocation tables can be kept with the option `-Wl,--emit-relocs`. It
may also be useful to include debugging information, such as DWARF, with
the argument `-ggdb` to facilitate software debugging.

This repository simplifies the generation of post-issuance software.
This template produces three files: the ELF file of the software, an *ad
hoc* binary file generated from the previous ELF, and a `gdbinit` file.
Note that the custom binary file is the one that will be deployed on the
microcontroller, not the ELF file.

## *Ad hoc* binary format for post-issuance deployment

ELF files contain many information, including code, data, symbols, and
metadata, which are essential for an operating system to load and
execute software. However, the complex structure of ELF files makes them
error-prone to parse. Moreover, only a subset of the ELF information is
required by the CRT0 to perform relocation tasks.

We have designed an *ad hoc* binary file format that keeps only the
necessary information for relocation. This format consists of four main
components. The first component contains the CRT0 code, responsible for
performing relocation tasks before executing the software code. The
second component comprises metadata, including a table of symbol values
and relocation tables. The table of symbol values mainly stores software
section sizes and entry point offsets. The third component consists of
the software sections, including the `.text` section (software code),
`.got` section (Global Offset Table), and `.data` section (initialized
global variables). The fourth and final component is optional padding,
ensuring the binary file is a multiple of 32 bytes.

Our *ad hoc* binary file format significantly reduces file size,
shrinking it to approximately 20% of the equivalent ELF file size for
the same software.

Below is a diagram of our *ad hoc* binary that illustrates all its
sections, followed by a diagram that correlates these sections with the
software components:

```
+-------+---------------+------------+-------+------+-------+---------+
|       |               |            |       |      |       |         |
| .text | symbol values | relocation | .text | .got | .data | padding |
|       |     table     |   tables   |       |      |       |         |
+-------+---------------+------------+-------+------+-------+---------+

+-------+----------------------------+----------------------+---------+
|       |                            |                      |         |
| crt0  |          metadata          |    post-issuance     | padding |
|       |                            |      software        |         |
+-------+----------------------------+----------------------+---------+
```

## Debugging post-issuance software

Since we only keep the necessary information from the ELF file for
relocation purposes, our binary format no longer includes debugging
information like DWARF. However, we generate a `gdbinit` file that
contains `symbol-file` and `add-symbol-file` instructions, which allow
GDB to read additional symbol table information from the ELF file. The
generated `gdbinit` file includes the actual memory addresses where the
software sections were copied by the CRT0. Simply replace `# Set binary
address in NVM here #` with the actual address where the binary is
loaded. The file can be loaded with the `source gdbinit` command during
a debugging session.

## Funding

This repository is part of the TinyPART project funded by the MESRI-BMBF
German-French cybersecurity program under grant agreements
n°ANR-20-CYAL-0005 and 16KIS1395K.
