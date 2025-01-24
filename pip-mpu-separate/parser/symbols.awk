BEGIN {
    symbols["start"] = 0
    symbols["__romSize"] = 0
    symbols["__romRamSize"] = 0
    symbols["__ramSize"] = 0
    symbols["__gotSize"] = 0
    symbols["__romRamEnd"] = 0
}

$8 in symbols {
    values[$8] = $2
}

END {
    for (s in symbols) {
        if (!(s in values)) {
            print_stderr("The symbol \"" s "\" is not defined...")
            exit 33
        }
    }

    dump_hex(values["start"])
    dump_hex(values["__romSize"])
    dump_hex(values["__romRamSize"])
    dump_hex(values["__ramSize"])
    dump_hex(values["__gotSize"])
    dump_hex(values["__romRamEnd"])
}
