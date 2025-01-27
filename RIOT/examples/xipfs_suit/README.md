# The eXecute In-Place File System

## Description

`xipfs` is a file system designed to streamline post-issuance software
deployment. `xipfs` allows direct execution of programs from flash
memory, eliminating the need for prior copying to RAM. This approach
conserves memory space and accelerates boot times, as the
microcontroller can run code directly from storage memory without
preloading into RAM.

The `xipfs` structure is based on a linked list, where each file
occupies at least one flash memory page. To prevent fragmentation, when
a file is deleted, subsequent files are shifted to fill the vacant
space.

`xipfs` is compatible with all microcontrollers featuring addressable
flash memory and most operating systems, provided they implement the
necessary functions to interact with the flash controller.

## Integration

The `xipfs` module has been integrated into RIOT using the Virtual File
System (VFS) interface. All system calls for manipulating `xipfs` files
have been mapped to corresponding VFS system calls. We have also
introduced two new commands in RIOT.

The first command enables the transfer of binary files from a host
computer to a microcontroller running RIOT using the command-line
interface. To achieve this, we modified the `pyterm` script, which
manages the command-line interface and communicates via UART. We added
a new "virtual command" called `load(1)`, which takes as a parameter
the path of the binary file to transfer relative to the location where
the script is executed. This command is executed on the host computer,
reads the binary file, converts it to base64, and sends the data in
fragments smaller than `SHELL_DEFAULT_BUFSIZE` using the `vfs w`
command.

The second command is the `exec(1)` command, which allows the execution
of loaded binary files. This command requires the path to a binary in
the `xipfs` file system and an optional list of parameters to pass to
the binary. Currently, binaries loaded using this command have access to
a limited set of libc functions and some RIOT functions. This is
achieved through the `execv(2)` function, which creates an array of
function pointers and passes it as a parameter to the binary before its
execution, allowing the binary to call these functions.

## Limitations

`xipfs` has the following limitations:

- No journaling: `xipfs` doesn't provide journaling. Without journaling,
  the file system cannot keep track of changes in a way that allows for
  recovery in the event of a crash or power failure. This can lead to
  data corruption and loss.

- No checksums: `xipfs` doesn't provide checksums. The lack of checksums
  means that the file system cannot verify the integrity of files. This
  increases the risk of undetected data corruption, as there is no
  mechanism to ensure that files have not been altered or damaged.

- Global file system lock: `xipfs` needs a global file system lock. A
  global file system lock can lead to performance bottlenecks, as it
  prevents multiple threads from accessing the file system
  simultaneously.

- Fixed file size: `xipfs` provide fixed file size. By default, a file
  created using `vfs_open(2)` has a fixed space reserved in flash that
  is the size of a flash page. This size cannot be extended later. To
  create a file larger than the fixed size of one flash page, the
  `mk(1)` command or the `xipfs_new_file(3)` function must be used.

- Limited character set: `xipfs` supports only a subset of 7-bit ASCII
  characters, specifically `[0-9A-Za-z\/\.\-_]`.

- Limited path length: `xipfs` maximum path length is 64 characters.

## Tested cards

`xipfs` is expected to be compatible with all boards that feature
addressable NVM and implement `flashpage` module of RIOT. However, only
the `DWM1001` board has been tested and is confirmed to function
correctly.

## Shell commands

The following commands are available for managing files in an `xipfs`
mount point:

| Command    | Arguments                                      | Description                                                                                             |
|------------|------------------------------------------------|---------------------------------------------------------------------------------------------------------|
| `vfs r`    | `<path>` `[bytes]` `[offset]`                  | Read `[bytes]` bytes at `[offset]` in file `<path>`                                                     |
| `vfs w`    | `<path>` `<ascii\|hex\|b64>` `<a\|o>` `<data>` | Write (`<a>`: append, `<o>` overwrite) `<ascii>` or `<hex>` or `<b64>` string `<data>` in file `<path>` |
| `vfs ls`   | `<path>`                                       | List files in `<path>`                                                                                  |
| `vfs cp`   | `<src>` `<dest>`                               | Copy `<src>` file to `<dest>`                                                                           |
| `vfs mv`   | `<src>` `<dest>`                               | Move `<src>` file to `<dest>`                                                                           |
| `vfs mkdir`| `<path>`                                       | Create directory `<path>`                                                                               |
| `vfs rm`   | `<path>`                                       | Unlink (delete) a file or a directory at `<path>`                                                       |
| `vfs df`   | `[path]`                                       | Show file system space utilization stats                                                                |
| `vfs mk`   | `<name>` `<size>` `<exec>`                     | Allocate the space needed to load a file                                                                |
| `vfs exec` | `<file>` `[arg0]` `[arg1]` `...` `[argn]`      | Run a binary                                                                                            |
| `load`     | `<src>` `<dst>`                                | Loads a binary from the host `<src>` path to the MCU `<dst>` path                                       |

## Getting started

### Installing required dependency

The Python script for extracting the necessary information from an ELF
file to generate a post-issuance binary requires the installation of the
`pyelftools` library. The compatible version is `0.30`, as other
versions have not been tested.

### Building RIOT with `xipfs`

To build RIOT with `xipfs`, execute the following command from the
current directory:

```console
$ BOARD=dwm1001 QUIET=0 make
```

### Flashing RIOT with `xipfs` onto the board

Before flashing RIOT with `xipfs`, ensure that the board is properly
connected to the machine. After verification is complete, execute the
following command from the current directory to flash RIOT with `xipfs`
onto the board:

```console
$ BOARD=dwm1001 QUIET=0 make flash
```

Now, RIOT is executed with `xipfs` onto the board.

### Building a post-issuance binary

The `hello-world` directory contains a straightforward program that
outputs "Hello World!" along with any arguments provided to it via the
command line.

To compile this program as a post-issuance binary, run the following
command from the current directory:

```console
$ make -C hello-world
```

This command will generate the `hello-world.bin` post-issuance binary
under the `hello-world` directory.

For further details on generating a post-issuance binary, please refer
to the file located in `hello-world/README.md`

### Connecting to the board through UART using pyterm

Before establishing communication with the board through UART using
`pyterm`, ensure that the `pyserial` library is correctly installed.

Once that is done, run the following command from the current directory
to connect to the card:

```console
$ BOARD=dwm1001 QUIET=0 make term
```

In this demonstration, you will find two mount points that can be viewed
using the following command:

```console
> vfs df
Mountpoint              Total         Used    Available     Use%
/dev/nvme0p0           40 KiB          0 B       40 KiB       0%
/dev/nvme0p1           60 KiB          0 B       60 KiB       0%
```

### Loading a post-issuance binary onto the board

To load the previously compiled post-issuance binary, execute the
following command:

```console
> load hello-world/hello-world.bin /dev/nvme0p0/hello-world.bin
```

The `load(1)` command sends a file creation command and commands to
transfer the file in base 64 encoded chunks, each of which is less than
or equal to `SHELL_DEFAULT_BUFSIZE`. To avoid unintentional characters
being inserted in commands sent to the board via the `load(1)` command,
avoid typing characters in the `pyterm` terminal during transfer.

This command transfers the `hello-world.bin` post-issuance binary from
the host machine to the `xipfs` mount point located within
`/dev/nvme0p0/`.

To check that the file has been successfully transferred, execute the
following command:

```console
> vfs ls /dev/nvme0p0/
hello-world.bin   896 B
total 1 files
```

### Running a post-issuance binary onto the board

To run the previously loaded post-issuance binary, execute the following
command:

```console
> vfs exec /dev/nvme0p0/hello-world.bin
Hello World!
```

It is also possible to provide arguments to the binary:

```console
> vfs exec /dev/nvme0p0/hello-world.bin ARG_0 ARG_1 ARG_2 ARG_3
Hello World!
ARG_0
ARG_1
ARG_2
ARG_3
```

### Add new libc and RIOT functions to the syscall table

To enable the post-issuance software to access new functions of libc
and/or RIOT, the following files must be edited:

1. [stdriot.c](/examples/xipfs/hello-world/stdriot/stdriot.c):

- Edit the enumeration `syscall_index_e` by adding an index for the new
  function in the format `SYSCALL_XXX`, where `XXX` is the name of the
  syscall in uppercase.

- Declare a type for the prototype of the syscall to be added in the
  format `XXX_t`, where `XXX` is the name of the syscall in uppercase.

- Duplicate one of the existing wrappers by substituting the wrapper's
  prototype with that of the new system call to be added. Change the
  type of the `func` pointer to match the type of the previously created
  system call, and update the index in the `syscall_table` to reflect
  the index of the newly created system call.

2. [stdriot.h](/examples/xipfs/hello-world/stdriot/stdriot.h):

- Add the prototype of your system call to the list of already existing
  prototypes while ensuring to maintain alphabetical order.

3. [file.c](/sys/fs/xipfs/file.c):

- Edit the enumeration `syscall_index_e` by adding an index for the new
  function in the format `SYSCALL_XXX`, where `XXX` is the name of the
  syscall in uppercase.

- In the `exec_syscall_table_init` function, which initializes the
  system call table, add the address of your system call at the
  corresponding index in the `syscall_table`.

## Funding

The `xipfs` project is part of the TinyPART project funded by the
MESRI-BMBF German-French cybersecurity program under grant agreements
n°ANR-20-CYAL-0005 and 16KIS1395K.
