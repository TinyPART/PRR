BEGIN {
    symbols["__root"] = 0
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

    print values["__root"]
}
