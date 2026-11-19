import sys
from pathlib import Path

def main():
    prg32_home = sys.argv[1]
    sys.path.append(prg32_home)
    import prg32.prg32
    prg32.cartridge.build_cartridge.write_portable_stubs(Path(sys.argv[2]), "basic_init", "basic_update", "basic_draw")
    prg32.cartridge.build_cartridge.write_linker(Path(sys.argv[3]),
                                                 0x40800000,
                                                 "prg32_entry_init")

if __name__ == '__main__':
    main()