savedcmd_master_i2c.mod := printf '%s\n'   master_i2c.o | awk '!x[$$0]++ { print("./"$$0) }' > master_i2c.mod
