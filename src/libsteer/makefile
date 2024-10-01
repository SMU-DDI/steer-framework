# =================================================================================================
#
#   makefile
#
#   Copyright (c) 2024 Anametric, Inc. All rights reserved.
#
#   Supported host operating systems:
#       Any *nix
#
#   Description:
#      	This makefile builds the STEER sample test.
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

# Define output library path/name
ifeq ($(BUILD_SHARED_LIB),0)
OUTFILE=$(BINDIR)/libsteer.a
else
OUTFILE=$(BINDIR)/libsteer.so
endif

# Create bin and obj directories
$(shell $(MKDIR) -p $(BINDIR))
$(shell $(MKDIR) -p $(OBJDIR))

# Define include directory paths
CFG_INC=-I. \
	-I../../include \
	-I../nist-sts/common \
	-I../../third-party/cJSON \
	-I../../third-party/whereami/src  

# Define library dependencies and directory paths
CFG_LIB=
CFG_LIB_INC=-L.

# Define C compiler flags
CFLAGS=

# Define object files
CFG_OBJ=
COMMON_OBJ=$(OBJDIR)/steer_file_system_utilities.o \
	$(OBJDIR)/steer_json_utilities.o \
	$(OBJDIR)/steer_nist_sts_utilities.o \
	$(OBJDIR)/steer_parameter_set_utilities.o \
	$(OBJDIR)/steer_parameters_info_utilities.o \
	$(OBJDIR)/steer_report_json_utilities.o \
	$(OBJDIR)/steer_report_utilities.o \
	$(OBJDIR)/steer_schedule_utilities.o \
	$(OBJDIR)/steer_string_utilities.o \
	$(OBJDIR)/steer_test_info_utilities.o \
	$(OBJDIR)/steer_test_shell.o \
	$(OBJDIR)/steer_utilities.o \
	$(OBJDIR)/steer_value_utilities.o \
	$(OBJDIR)/cephes.o \
	$(OBJDIR)/dfft.o \
	$(OBJDIR)/matrix.o \
	$(OBJDIR)/cJSON.o \
	$(OBJDIR)/whereami.o 
OBJ=$(COMMON_OBJ) $(CFG_OBJ)

#
# Configuration: Debug
#
ifeq ($(BUILD_CFG),Debug)
ifeq ($(BUILD_PROFILE),0)
COMPILE=$(CC) -Wall -c -g -fPIC -o "$(OBJDIR)/$(*F).o" $(CFG_INC) $(CFLAGS) "$<"
else
COMPILE=$(CC) -Wall -pg -c -g -fPIC -o "$(OBJDIR)/$(*F).o" $(CFG_INC) $(CFLAGS) "$<"
endif
ifeq ($(BUILD_SHARED_LIB),0)
LINK=$(AR) cr "$(OUTFILE)" $(OBJ) $(CFG_LIB)
else
ifeq ($(BUILD_PROFILE),0)
LINK=$(CC) -Wall "$(CFG_LIB_INC)" -g -o "$(OUTFILE)" $(OBJ) $(CFG_LIB) -shared -fPIC
else
LINK=$(CC) -Wall -pg "$(CFG_LIB_INC)" -g -o "$(OUTFILE)" $(OBJ) $(CFG_LIB) -shared -fPIC
endif
endif
endif

#
# Configuration: Release
#
ifeq ($(BUILD_CFG),Release)
ifeq ($(BUILD_PROFILE),0)
COMPILE=$(CC) -Wall -c -Os -DNDEBUG -fPIC -o "$(OBJDIR)/$(*F).o" $(CFG_INC) $(CFLAGS) "$<"
else
COMPILE=$(CC) -Wall -pg -c -Os -DNDEBUG -fPIC -o "$(OBJDIR)/$(*F).o" $(CFG_INC) $(CFLAGS) "$<"
endif
ifeq ($(BUILD_SHARED_LIB),0)
LINK=$(AR) cr "$(OUTFILE)" $(OBJ) $(CFG_LIB)
else
ifeq ($(BUILD_PROFILE),0)
LINK=$(CC) -Wall "$(CFG_LIB_INC)" -o "$(OUTFILE)" $(OBJ) $(CFG_LIB) -shared -fPIC
else
LINK=$(CC) -Wall -pg "$(CFG_LIB_INC)" -o "$(OUTFILE)" $(OBJ) $(CFG_LIB) -shared -fPIC
endif
endif
endif

# Pattern rules
$(OBJDIR)/%.o : %.c
	$(COMPILE)

$(OBJDIR)/%.o : ../utilities/%.c
	$(COMPILE)

$(OBJDIR)/%.o : ../nist-sts/common/%.c
	$(COMPILE)

$(OBJDIR)/%.o : ../../third-party/cJSON/%.c
	$(COMPILE)

$(OBJDIR)/%.o : ../../third-party/whereami/src/%.c
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
