# Pip MPU Separate Compilation

You can find more about the Pip protokernel at its
[website](http://pip.univ-lille.fr/).

The source code is covered by CeCILL-A licence.

This repository contains what is required to generate a bootable image given
your root partition binary and an already compiled Pip library.


##  Usage

```bash
make PIP_A=<path to your pip.a> PARTITION_BIN=<path to your rootpart.bin>
```

will generate a `pip+root.bin` ready to burn.
It will also generate a `pip+root.elf` for debugging purpose.
