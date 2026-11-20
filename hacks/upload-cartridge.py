import os
import sys

sys.path.append(os.path.expanduser(os.getenv("PRG32_HOME")))

import prg32.prg32

address = "192.168.1.216"
slot = "cart0"

cartridge_filename = os.path.join(os.path.expanduser(sys.argv[1]), "basic.prg32")

r = prg32.prg32.main(["esp32c6", "upload", "--url", f"http://{address}", "--slot", slot,
                      "--command", "debug", cartridge_filename])

if r != 0:
    sys.exit(r)

sys.exit(os.system(f"curl 'http://{address}/api/games/select?slot={slot}' -X POST"))


