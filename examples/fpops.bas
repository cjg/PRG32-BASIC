n = 1000
ops = 0
elapsed = 0

duration = 30000

PRINT "This experiment will require about ", duration / 1000.0, " seconds"

t0 = TICKS()
WHILE elapsed < 30000
    i = 0.0
    WHILE i < n
        i = i + 1.0
    END WHILE
    ops = ops + n + 4
    elapsed = TICKS() - t0
END WHILE

seconds = elapsed / 1000.0
ops_per_second = ops / seconds

PRINT "Executed about ", ops, " fixed-point operations in ", seconds, " seconds. OPS/S: ", ops_per_second