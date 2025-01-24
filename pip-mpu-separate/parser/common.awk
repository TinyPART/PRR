function parse_hexstr(hs) {
    if (hs !~ /^[[:xdigit:]]+$/) {
        print "Error: \"" hs "\" is not a hexadecimal number..." > "/dev/stderr"
        exit 32
    }
    r = 0
    l = length(hs)
    hs = tolower(hs)
    for(i = 1; i <= l; i++)
        r = r * 16 + index("123456789abcdef", substr(hs, i, 1))
    return r
}

# Warning: the following function uses "%c", its behaviour can depend
# on the locale. Use the C locale, or an option (such as -b for gawk)
# to avoid interpreting bytes as characters
function dump_hex(hs) {
    hs = tolower(hs)
    for (i = 7; i >= 1; i -= 2) {
        printf("%c", 16 * index("123456789abcdef", substr(hs, i, 1)) + index("123456789abcdef", substr(hs, i + 1, 1)))
    }
}

# Warning: the following function uses "%c", its behaviour can depend
# on the locale. Use the C locale, or an option (such as -b for gawk)
# to avoid interpreting bytes as characters
function print_u32le(x) {
    for(i = 0; i < 4; i++) {
        printf("%c", x%256)
        x /= 256
    }
}

function print_stderr(msg) {
    print "\033[91;1m" msg "\033[0m" > "/dev/stderr"
}
