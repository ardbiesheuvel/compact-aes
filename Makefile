##
## Compact AES using standard C
##
## Copyright (C) 2017 Ard Biesheuvel <ard.biesheuvel@gmail.com>
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License version 2 as
## published by the Free Software Foundation.
##

SRCS := caes.c test.c
BIN := caes

$(BIN): $(SRCS)
	$(CC) -Wall -o $(@) $(^)
