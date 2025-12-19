###############################################################################
 #
 # Copyright (C) 2024 Analog Devices, Inc.
 #
 # Licensed under the Apache License, Version 2.0 (the "License");
 # you may not use this file except in compliance with the License.
 # You may obtain a copy of the License at
 #
 #     http://www.apache.org/licenses/LICENSE-2.0
 #
 # Unless required by applicable law or agreed to in writing, software
 # distributed under the License is distributed on an "AS IS" BASIS,
 # WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 # See the License for the specific language governing permissions and
 # limitations under the License.
 #
 ##############################################################################
# This file can be used to set build configuration
# variables.  These variables are defined in a file called
# "Makefile" that is located next to this one.

# For instructions on how to use this system, see
# https://analogdevicesinc.github.io/msdk/USERGUIDE/#build-system

#VERBOSE=1

#DEBUG=1
# MXC_OPTIMIZE_CFLAGS = -O0
# ^ For example, you can uncomment this line to
# optimize the project for debugging

# PROJ_CFLAGS += --verbose
# PROJ_CFLAGS += -DMSDK_NO_GPIO_CLK_INIT
PROJ_CFLAGS += -DFL_ARM_NO_CYCCNT

IPATH += include
IPATH += libraries/FastLED/src/
IPATH += libraries/AnimatedGIF/src/

VPATH += src/
VPATH += libraries/FastLED/src/
VPATH += libraries/FastLED/src/fl
VPATH += libraries/FastLED/src/platforms/arm/max32
VPATH += libraries/AnimatedGIF/src/

MFLOAT_ABI = hard

# Enable SDHC library
LIB_SDHC = 1

# Use FatFS version R0.15
FATFS_VERSION = ff15

# **********************************************************

# Add your config here!



