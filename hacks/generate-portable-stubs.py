import sys
from pathlib import Path

def main():
    prg32_home = sys.argv[1]
    sys.path.append(prg32_home)
    import prg32.prg32
    print(dir(prg32))
    prg32.cartridge.build_cartridge.write_portable_stubs(Path(sys.argv[2]), "basic_init", "basic_update", "basic_draw")
    #from prg32.abi.cartridge.build_cartridge import write_portable_stubs

if __name__ == '__main__':
    main()