BEGIN {
    bytereg = "[[:xdigit:]][[:xdigit:]]"
    wordreg = "^" bytereg bytereg bytereg bytereg "$"
    numreg = "^[[:digit:]][[:digit:]]*$"
    entries = 0
    counter = 0
}

/.rel.rom.ram/ {
    if ($8 !~ numreg) {
        print_stderr("Wrongly formatted relocation entry number...")
        exit 33
    } else {
        print_u32le($8)
        entries = $8
    }
}

/.rel.rom.ram/,/^$/ {
    if ($1 ~ wordreg && $4 ~ wordreg && $3 == "R_ARM_ABS32") {
        dump_hex($1)
        counter++
    }
}

END {
    if (entries != counter) {
        print_stderr("Failed to parse some relocation entries...")
        exit 33
    }
    if (entries == 0) {
        print_u32le("0")
    }
}
