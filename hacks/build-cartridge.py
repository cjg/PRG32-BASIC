import os
import sys

sys.path.append(os.path.expanduser(os.getenv("PRG32_HOME")))

import prg32.prg32

src_filename = os.path.join(os.path.expanduser(sys.argv[1]), "src", "cartridge.c")
output_filename = os.path.join(os.path.expanduser(sys.argv[2]), "basic.prg32")

sys.exit(prg32.prg32.main(["cartridge", "build", src_filename, "--out", output_filename, "--name", "PRG32 BASIC",
                           "--entry-prefix", "basic"]))

