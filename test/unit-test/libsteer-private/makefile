# =================================================================================================
#
#   makefile
#
#   Copyright (c) 2024 Anametric, Inc. All rights reserved.
#
#   Supported host operating systems:
#       Any
#
#   Description:
#      	This makefile builds the private interface unit test program for libsteer.
#
#   Notes:
#  		1)  This makefile assumes the use of ANSI C99 compliant compilers.
#
# =================================================================================================

# Command aliases
MAKE=MAKE
MKDIR=mkdir
CC=gcc
AR=ar
RM=rm

# If no build products root is specified, "$HOME" will be used
ifndef BUILD_ROOT
BUILD_ROOT="$(HOME)"
endif 

# If no build products directory name is specified, "steer-framework" will be used
ifndef BUILD_PRODUCTS_DIR_NAME
BUILD_PRODUCTS_DIR_NAME=steer-framework
endif

# If no binary directory is specified, "bin" will be used
ifndef BUILD_PRODUCTS_BIN_DIR
BUILD_PRODUCTS_BIN_DIR=bin
endif

# If no object directory is specified, "obj" will be used
ifndef BUILD_PRODUCTS_OBJ_DIR
BUILD_PRODUCTS_OBJ_DIR=obj
endif

# If no operating environment is specified, "darwin" will be used
ifndef BUILD_OPERATING_ENV
BUILD_OPERATING_ENV=darwin
endif

# If no architecture is specified, "x64" will be used
ifndef BUILD_ARCH
BUILD_ARCH=x64
endif

# If no configuration is specified, "Debug" will be used
ifndef BUILD_CFG
BUILD_CFG=Debug
endif

# If no library type is specified, "static" will be built
ifndef BUILD_SHARED_LIB
BUILD_SHARED_LIB=0
endif

# If no profiling is specified, profiling will be disabled
ifndef BUILD_PROFILE
BUILD_PROFILE=0
endif

# Define build and obj directories
BINDIR="$(BUILD_ROOT)/$(BUILD_PRODUCTS_DIR_NAME)/$(BUILD_PRODUCTS_BIN_DIR)/$(BUILD_OPERATING_ENV)/$(BUILD_ARCH)/$(BUILD_CFG)"
OBJDIR="$(BUILD_ROOT)/$(BUILD_PRODUCTS_DIR_NAME)/$(BUILD_PRODUCTS_OBJ_DIR)/$(BUILD_OPERATING_ENV)/$(BUILD_ARCH)/$(BUILD_CFG)"

# Define output executable path/name
OUTFILE=$(BINDIR)/libsteer_private_unit_test

# Create bin and obj directories
$(shell $(MKDIR) -p $(BINDIR))
$(shell $(MKDIR) -p $(OBJDIR))

# Define include directory paths
CFG_INC=-I../../../include \
	-I../../../src/utilities \
	-I/usr/include/CUnit \
	-I/opt/local/include/CUnit \
	-I../../../third-party/cJSON

# Define library dependencies and directory paths
CFG_LIB=
CFG_LIB_INC=-L.

# Need to work on this
ifeq ($(BUILD_OPERATING_ENV),darwin)
CFG_LIB=/opt/local/lib/libcunit.a
endif
ifeq ($(BUILD_OPERATING_ENV),linux)
ifeq ($(BUILD_ARCH),x64)
CFG_LIB=/usr/lib/x86_64-linux-gnu/libcunit.a \
	/usr/lib/x86_64-linux-gnu/libm.so
endif
ifeq ($(BUILD_ARCH),arm64)
CFG_LIB=/usr/lib/aarch64-linux-gnu/libcunit.a \
	/usr/lib/aarch64-linux-gnu/libm.so
endif
endif

# Define object files
CFG_OBJ=
COMMON_OBJ=$(OBJDIR)/libsteer_private_unit_test.o
OBJ=$(COMMON_OBJ) $(CFG_OBJ)

#
# Configuration: Debug
#
ifeq ($(BUILD_CFG),Debug)
ifeq ($(BUILD_PROFILE),0)
COMPILE=$(CC) -Wall -pthread -c -g -o "$(OBJDIR)/$(*F).o" $(CFG_INC) "$<"
else
COMPILE=$(CC) -Wall -pthread -pg -c -g -o "$(OBJDIR)/$(*F).o" $(CFG_INC) "$<"
endif
ifeq ($(BUILD_SHARED_LIB),0)
ifeq ($(BUILD_PROFILE),0)
LINK=$(CC) -Wall -pthread "$(CFG_LIB_INC)" -g -o "$(OUTFILE)" $(OBJ) $(BINDIR)/libsteer.a $(CFG_LIB)
else
LINK=$(CC) -Wall -pthread -pg "$(CFG_LIB_INC)" -g -o "$(OUTFILE)" $(OBJ) $(BINDIR)/libsteer.a $(CFG_LIB)
endif
else
ifeq ($(BUILD_PROFILE),0)
LINK=$(CC) -Wall -pthread "$(CFG_LIB_INC)" -g -o "$(OUTFILE)" $(OBJ) $(BINDIR)/libsteer.so $(CFG_LIB) 
else
LINK=$(CC) -Wall -pthread -pg "$(CFG_LIB_INC)" -g -o "$(OUTFILE)" $(OBJ) $(BINDIR)/libsteer.so $(CFG_LIB) 
endif
endif
endif

#
# Configuration: Release
#
ifeq ($(BUILD_CFG),Release)
ifeq ($(BUILD_PROFILE),0)
COMPILE=$(CC) -Wall -pthread -c -Os -DNDEBUG -o "$(OBJDIR)/$(*F).o" $(CFG_INC) "$<"
else
COMPILE=$(CC) -Wall -pthread -pg -c -Os -DNDEBUG -o "$(OBJDIR)/$(*F).o" $(CFG_INC) "$<"
endif
ifeq ($(BUILD_SHARED_LIB),0)
ifeq ($(BUILD_PROFILE),0)
LINK=$(CC) -Wall -pthread "$(CFG_LIB_INC)" -o "$(OUTFILE)" $(OBJ) $(BINDIR)/libsteer.a $(CFG_LIB) 
else
LINK=$(CC) -Wall -pthread -pg "$(CFG_LIB_INC)" -o "$(OUTFILE)" $(OBJ) $(BINDIR)/libsteer.a $(CFG_LIB) 
endif
else
ifeq ($(BUILD_PROFILE),0)
LINK=$(CC) -Wall -pthread "$(CFG_LIB_INC)" -o "$(OUTFILE)" $(OBJ) $(BINDIR)/libsteer.so $(CFG_LIB) 
else
LINK=$(CC) -Wall -pthread -pg "$(CFG_LIB_INC)" -o "$(OUTFILE)" $(OBJ) $(BINDIR)/libsteer.so $(CFG_LIB) 
endif
endif
endif

# Pattern rules
$(OBJDIR)/%.o : %.c
	$(COMPILE)

# Build rules
all: $(OUTFILE)

$(OUTFILE): $(OUTDIR)  $(OBJ)
	$(LINK)

# Rebuild this project
rebuild: cleanall all

# Clean this project
clean:
	$(RM) -f $(OUTFILE)
	$(RM) -f $(OBJ)

# Clean this project and all dependencies
cleanall: clean
