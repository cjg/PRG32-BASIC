import os
import sys

sys.path.append(os.path.expanduser(os.getenv("PRG32_HOME")))

import prg32.prg32

cartridge_filename = os.path.join(os.path.expanduser(sys.argv[1]), "basic.prg32")

sys.exit(prg32.prg32.main(["esp32c6", "upload", "--url", "http://192.168.1.216", "--slot", "cart0",
                           "--command", "debug", cartridge_filename]))

