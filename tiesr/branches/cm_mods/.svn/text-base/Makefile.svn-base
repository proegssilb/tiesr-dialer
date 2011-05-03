# Makefile for TIesr Libraries and test programs
#
# Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
#
# This program is free software; you can redistribute it and/or modify 
# it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation version 2.1 of the License.
#
# This program is distributed "as is" WITHOUT ANY WARRANTY of any kind,
# whether express or implied; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#

# This Makefile will build all of the TIesr APIs and test programs for a given
# configuration. It is assumed that the gnu tools, such as gcc and g++ exist and
# are on the PATH so that they can be accessed by specifying "gcc" and "g++" on the
# command line (or "gcc.exe" an "g++.exe for Windows).  The proper method of
# using this Makefile is to build for a given configuration, for example:
#
# make LinuxDebugGnu
#
# The valid configurations are:
#
# LinuxDebugGnu
# LinuxReleaseGnu
# ArmLinuxDebugGnueabi
# ArmLinuxReleaseGnueabi
# WindowsDebugMinGW
# WindowsReleaseMinGW
#
# The Makefile also supports building any subset of projects for the selected
# configuration by specifying the list of projects as the PROJECTS variable
# on the command line.  For example, to build the TIesrDT and TestTIesrDT 
# projects for the LinuxDebugGnu configuration:
#
# make LinuxDebugGnu  PROJECTS='TIesrDTso TestTIesrDT'
#
# Supported projects are:
#
# TIesrDTso TestTIesrDT TIesrDictso TestTIesrDict TIesrFlexso TestTIesrFlex
# TestTIesrFA TIesrEngineCoreso TIesrEngineSIso TIesrSIso TestTIesrSI
# TIesrFAWinso (for Windows only)
# TIesrFA_ALSAso (for Linux with ALSA support only)
#
# Build steps that can be performed are specified by the STEP variable, which 
# may be set to "clean", "build", or "clobber".  If not set, "build" is the default.
#

# Supported configurations
WINCONFS = WindowsDebugMinGW WindowsReleaseMinGW
LINUXCONFS = LinuxDebugGnu LinuxReleaseGnu ArmLinuxDebugGnueabi ArmLinuxReleaseGnueabi

# Configurations are phony targets
.PHONY : $(WINCONFS) $(LINXCONFS)

# Supported projects that are OS independent
FLEXPROJECTS = TIesrDTso TestTIesrDT TIesrDictso TestTIesrDict TIesrFlexso TestTIesrFlex
RECOPROJECTS = TestTIesrFA TIesrEngineCoreso TIesrEngineSIso TIesrSIso TestTIesrSI

# Projects that are OS dependent - the TIesrFA API. This must be built prior to the 
# RECOPROJECTS being built.
WINPROJECTS = TIesrFAWinso
LINUXPROJECTS = TIesrFA_ALSAso

#Projects are phony targets
.PHONY : $(FLEXPROJECTS) $(RECOPROJECTS) $(WINPROJECTS) $(LINUXPROJECTS)

# Unless PROJECTS is specified on the command line, the target will build all
# projects for the given configuration
BUILDWINPROJECTS =   $(WINPROJECTS) $(RECOPROJECTS) $(FLEXPROJECTS)
BUILDLINUXPROJECTS =   $(LINUXPROJECTS) $(RECOPROJECTS) $(FLEXPROJECTS)

#If PROJECTS is defined on the command line, it replaces projects to build
ifdef PROJECTS
BUILDWINPROJECTS = $(PROJECTS)
BUILDLINUXPROJECTS = $(PROJECTS)
endif

# Unless STEP is specified, default to build
STEP=build

# Specify use of windows Cygwin gcc tools if building for Windows
$(WINCONFS) : WINTOOLS=CC=gcc.exe CCC=g++.exe CXX=g++.exe AS=as.exe CND_PLATFORM=Cygwin-Windows
$(LINUXCONFS) : WINTOOLS=

# Define a variable holding the configuration to be built
WindowsDebugMinGW : BUILDCONF=WindowsDebugMinGW
WindowsReleaseMinGW : BUILDCONF=WindowsReleaseMinGW
LinuxDebugGnu : BUILDCONF=LinuxDebugGnu
LinuxReleaseGnu : BUILDCONF= LinuxReleaseGnu
ArmLinuxDebugGnueabi : BUILDCONF= ArmLinuxDebugGnueabi
ArmLinuxReleaseGnueabi : BUILDCONF=ArmLinuxReleaseGnueabi

# Rules to build all requested projects for the target configuration
$(WINCONFS) : $(BUILDWINPROJECTS)

$(LINUXCONFS) : $(BUILDLINUXPROJECTS)

# Rules to build each project
TIesrDTso : 
	cd TIesrDT/TIesrDTso && $(MAKE) SUB=no CONF=$(BUILDCONF) $(WINTOOLS) $(STEP)
	
TestTIesrDT :
	cd TIesrDT/TestTIesrDT && $(MAKE) SUB=no CONF=$(BUILDCONF) $(WINTOOLS) $(STEP)
	
TIesrDictso :
	cd TIesrDict/TIesrDictso && $(MAKE) SUB=no CONF=$(BUILDCONF) $(WINTOOLS) $(STEP)
	
TestTIesrDict :
	cd TIesrDict/TestTIesrDict && $(MAKE) SUB=no CONF=$(BUILDCONF) $(WINTOOLS) $(STEP)
	
TIesrFlexso :
	cd TIesrFlex/TIesrFlexso && $(MAKE) SUB=no CONF=$(BUILDCONF) $(WINTOOLS) $(STEP)
	
TestTIesrFlex :
	cd TIesrFlex/TestTIesrFlex && $(MAKE) SUB=no CONF=$(BUILDCONF) $(WINTOOLS) $(STEP)
	
TIesrFAWinso :
	cd TIesrFA/TIesrFAWinso && $(MAKE) SUB=no CONF=$(BUILDCONF) $(WINTOOLS) $(STEP)
	
TIesrFA_ALSAso :
	cd TIesrFA/TIesrFA_ALSAso && $(MAKE) SUB=no CONF=$(BUILDCONF) $(WINTOOLS) $(STEP)
	
TestTIesrFA :
	cd TIesrFA/TestTIesrFA && $(MAKE) SUB=no CONF=$(BUILDCONF) $(WINTOOLS) $(STEP)
	
TIesrEngineCoreso :
	cd TIesrEngine/TIesrEngineCoreso && $(MAKE) SUB=no CONF=$(BUILDCONF) $(WINTOOLS) $(STEP)
	
TIesrEngineSIso :
	cd TIesrEngine/TIesrEngineSIso && $(MAKE) SUB=no CONF=$(BUILDCONF) $(WINTOOLS) $(STEP)
	
TIesrSIso :
	cd TIesrSI/TIesrSIso && $(MAKE) SUB=no CONF=$(BUILDCONF) $(WINTOOLS) $(STEP)
	
TestTIesrSI :
	cd TIesrSI/TestTIesrSI && $(MAKE) SUB=no CONF=$(BUILDCONF) $(WINTOOLS) $(STEP)	


.PHONY : help
help : 
	@echo 'This Makefile will build all of the TIesr APIs and test programs for a given'
	@echo 'configuration. It is assumed that the gnu tools, such as gcc and g++ exist and'
	@echo 'are on the PATH so that they can be accessed by specifying "gcc" and "g++" on the'
	@echo 'command line (or "gcc.exe" an "g++.exe" for Windows).  The proper method of'
	@echo 'using this Makefile is to build for a given configuration, for example:'
	@echo ' '
	@echo 'make LinuxDebugGnu'
	@echo ' '
	@echo 'The valid configurations are:'
	@echo ' '
	@echo 'LinuxDebugGnu'
	@echo 'LinuxReleaseGnu'
	@echo 'ArmLinuxDebugGnueabi'
	@echo 'ArmLinuxReleaseGnueabi'
	@echo 'WindowsDebugMinGW'
	@echo 'WindowsReleaseMinGW'
	@echo ' '
	@echo 'The Makefile also supports building any subset of projects for the selected'
	@echo 'configuration by specifying the list of projects as the PROJECTS variable'
	@echo 'on the command line.  For example, to build the TIesrDT and TestTIesrDT'
	@echo 'projects for the LinuxDebugGnu configuration:'
	@echo ' '
	@echo 'make LinuxDebugGnu  PROJECTS="TIesrDTso TestTIesrDT"'
	@echo ' '
	@echo 'Supported projects are:'
	@echo ' '
	@echo 'TIesrDTso TestTIesrDT TIesrDictso TestTIesrDict TIesrFlexso TestTIesrFlex'
	@echo 'TestTIesrFA TIesrEngineCoreso TIesrEngineSIso TIesrSIso TestTIesrSI'
	@echo 'TIesrFAWinso (for Windows only)'
	@echo 'TIesrFA_ALSAso (for Linux with ALSA support only)'
	@echo ' '
	@echo 'Build steps that can be performed are specified by the STEP variable, which'
	@echo 'may be set to "clean", "build", or "clobber".  If not set, "build" is the default.'
	@echo 'For example:'
	@echo ' '
	@echo 'make LinuxDebugGnu PROJECTS=TIesrDTso STEP=clean'
	


