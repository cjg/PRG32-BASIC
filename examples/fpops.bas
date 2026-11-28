n = 1000
ops = 0
elapsed = 0

t0 = TICKS()
WHILE elapsed < 30000
    i = 0.0
    WHILE i < n
        i = i + 1.0
    END WHILE
    ops = ops + n + 4
    elapsed = TICKS() - t0
END WHILE

PRINT "Executed about "
PRINT ops
PRINT " fixed-point operations in "
PRINT elapsed
PRINT " milliseconds"