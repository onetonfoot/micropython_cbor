# Location of top-level MicroPython directory
MPY_DIR = /opt/micropython

# Name of module
MOD = ucbor

TINYCBOR_SRC_DIR = ./tinycbor

CFLAGS += -Wno-unused-function -Wno-error -I$(TINYCBOR_SRC_DIR)

# Architecture to build for (x86, x64, armv6m, armv7m, xtensa, xtensawin)
# fails to compile as not hardware float support?
# ARCH = armv7m
ARCH = armv6m
# ARCH = x64

# Source files (.c or .py)
SRC = src/ucbor.c \
		$(TINYCBOR_SRC_DIR)/cborencoder.c \
		$(TINYCBOR_SRC_DIR)/cborerrorstrings.c \
		$(TINYCBOR_SRC_DIR)/cborparser.c \
		$(TINYCBOR_SRC_DIR)/cborparser_dup_string.c

# Include to get the rules for compiling and linking the module
include $(MPY_DIR)/py/dynruntime.mk 