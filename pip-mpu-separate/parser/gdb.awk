BEGIN {
    symbols["__root"] = 0
    symbols["__unusedRamStart"] = 0
    symbols["__romRamSize"] = 0
    symbols["__gotSize"] = 0
}

$8 in symbols {
    values[$8] = parse_hexstr($2)
}

END {
    for (s in symbols) {
        if (!(s in values)) {
            print_stderr("The symbol \"" s "\" is not defined...")
            exit 33
        }
    }

    binaryAddr = values["__root"]
    romAddr = binaryAddr + _crt0meta_size
    relGotAddr = values["__unusedRamStart"]
    relRomRamAddr = relGotAddr + values["__gotSize"]
    relRamAddr = relRomRamAddr + values["__romRamSize"]

    print "symbol-file \"" pip "\""

    print "add-symbol-file \"" crt0 "\""\
              " -s .text " binaryAddr

    print "add-symbol-file \"" root "\""\
              " -s .rom " romAddr\
              " -s .got " relGotAddr\
              " -s .rom.ram " relRomRamAddr\
              " -s .ram " relRamAddr
}
