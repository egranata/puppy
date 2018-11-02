BUILD = ../../../../out

include ../../py/mkenv.mk

PROG = $(BUILD)/micropython
QSTR_DEFS = qstrdefsport.h

UNAME_S = puppy

PUPPY_GCC = $(TOP)/../../build/gcc.sh

include $(TOP)/py/py.mk

INC +=  -I.
INC +=  -I$(TOP)
INC += -I$(BUILD)

CWARN = -Wall -Werror
CWARN += -Wpointer-arith -Wuninitialized
CFLAGS = $(INC) $(CWARN) -std=gnu99 -DUNIX $(CFLAGS_MOD) $(COPT) $(CFLAGS_EXTRA)

SRC_C = \
	main.c \
	mphalstdio.c \
	mphalport.c \

OBJ = $(PY_O)
OBJ += $(addprefix $(BUILD)/, $(SRC_C:.c=.o))

CC = $(PUPPY_GCC)
LD = $(PUPPY_GCC)

include $(TOP)/py/mkrules.mk
