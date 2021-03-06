BUILD = $(PUPPY_ROOT)/out

include ../../py/mkenv.mk

PROG = $(PUPPY_ROOT)/$(OUTWHERE)/micropython
QSTR_DEFS = qstrdefsport.h

UNAME_S = puppy

include $(TOP)/py/py.mk

INC +=  -I.
INC +=  -I$(TOP)
INC += -I$(BUILD)

CWARN = -Wall -Werror
CWARN += -Wpointer-arith -Wuninitialized
CFLAGS = $(INC) $(CWARN) -std=gnu99 $(CFLAGS_MOD) $(COPT) $(CFLAGS_EXTRA)
CXXFLAGS = $(INC) $(CWARN) -std=c++14 $(CFLAGS_MOD) $(COPT) $(CFLAGS_EXTRA)

SRC_C = \
	main.c \
	mphalstdio.c \
	mphalport.c \
	execute.c \
	file.c \
	modtime.c \
	moduos.c \

SRC_CXX = \
	repl.cpp \
	puppy_system_impl.cpp \

SRC_QSTR += $(SRC_C)

OBJ = $(PY_O)
OBJ += $(addprefix $(BUILD)/, $(SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_CXX:.cpp=.o))

LD = $(CC)
LDFLAGS += -llinenoise -lshell

define compile_cpp
$(ECHO) "CXX $<"
$(Q)$(CXX) $(CXXFLAGS) -c -MD -o $@ $<
@# The following fixes the dependency file.
@# See http://make.paulandlesley.org/autodep.html for details.
@# Regex adjusted from the above to play better with Windows paths, etc.
@$(CP) $(@:.o=.d) $(@:.o=.P); \
  $(SED) -e 's/#.*//' -e 's/^.*:  *//' -e 's/ *\\$$//' \
      -e '/^$$/ d' -e 's/$$/ :/' < $(@:.o=.d) >> $(@:.o=.P); \
  $(RM) -f $(@:.o=.d)
endef
vpath %.cpp . $(TOP)
$(BUILD)/%.o: %.cpp
	$(call compile_cpp)

include $(TOP)/py/mkrules.mk
