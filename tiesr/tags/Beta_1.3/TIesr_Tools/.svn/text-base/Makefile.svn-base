#-------------------------------------------------------------

#
# Makefile
#
# Makefile for TIesr Tools.
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
 
#-------------------------------------------------------------

# This Makefile will build all of the TIesr Tools for a given
# configuration. It is assumed that the gnu tools, such as gcc and g++ exist
# and are on the PATH so that they can be accessed by specifying "gcc" and
# "g++" on the command line (or "gcc.exe" an "g++.exe for Windows).  The
# proper method of using this Makefile is to build for a given configuration,
# for example:
#
# make LinuxDebugGnu
#
# The valid configurations are:
#
# LinuxDebugGnu
# LinuxReleaseGnu
# WindowsDebugMinGW
# WindowsReleaseMinGW
#
# The Makefile also supports building any subset of projects for the selected
# configuration by specifying the list of projects as the PROJECTS variable
# on the command line.  For example, to build the TIesrDT and TestTIesrDT 
# projects for the LinuxDebugGnu configuration:
#
# make LinuxDebugGnu  PROJECTS='dictproc'
#
# Supported projects are:
#
# dictproc
#
# Build steps that can be performed are specified by the STEP variable, which 
# may be set to "clean", "build", or "clobber".  If not set, "build" is the default.
#

# Supported configurations
WINCONFS = WindowsDebugMinGW WindowsReleaseMinGW
LINUXCONFS = LinuxDebugGnu LinuxReleaseGnu 

# Configurations are phony targets
.PHONY : $(WINCONFS) $(LINXCONFS)

# Supported projects that are OS independent
TOOLPROJECTS = dictproc gaussiancluster

# Projects that are OS dependent
WINPROJECTS = 
LINUXPROJECTS = 

# Projects are phony targets
.PHONY : $(TOOLPROJECTS) $(WINPROJECTS) $(LINUXPROJECTS)

# Unless PROJECTS is specified on the command line, the target will build all
# projects for the given configuration
BUILDWINPROJECTS =   $(WINPROJECTS) $(TOOLPROJECTS)
BUILDLINUXPROJECTS =   $(LINUXPROJECTS) $(TOOLPROJECTS)

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

# Rules to build all requested projects for the target configuration
$(WINCONFS) : $(BUILDWINPROJECTS)

$(LINUXCONFS) : $(BUILDLINUXPROJECTS)

# Rules to build each project
dictproc : 
	cd dictproc && $(MAKE) SUB=no CONF=$(BUILDCONF) $(WINTOOLS) $(STEP)

gaussiancluster :
	cd GaussianCluster && $(MAKE) SUB=no CONF=$(BUILDCONF) $(WINTOOLS) $(STEP)

.PHONY : help
help : 
	@echo 'This Makefile will build all of the TIesr Tools for a given'
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
	@echo 'WindowsDebugMinGW'
	@echo 'WindowsReleaseMinGW'
	@echo ' '
	@echo 'The Makefile also supports building any subset of projects for the selected'
	@echo 'configuration by specifying the list of projects as the PROJECTS variable'
	@echo 'on the command line.  For example, to build the dictproc'
	@echo 'project for the LinuxDebugGnu configuration:'
	@echo ' '
	@echo 'make LinuxDebugGnu  PROJECTS="dictproc"'
	@echo ' '
	@echo 'Supported projects are:'
	@echo ' '
	@echo 'dictproc'
	@echo 'gaussiancluster'
	@echo ' '
	@echo 'Build steps that can be performed are specified by the STEP variable, which'
	@echo 'may be set to "clean", "build", or "clobber".  If not set, "build" is the default.'
	@echo 'For example:'
	@echo ' '
	@echo 'make LinuxDebugGnu PROJECTS=dictproc STEP=clean'
