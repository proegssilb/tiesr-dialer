#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc.exe
CCC=g++.exe
CXX=g++.exe
FC=
AS=as.exe

# Macros
CND_PLATFORM=Cygwin-Windows
CND_CONF=WindowsReleaseMinGW
CND_DISTDIR=dist

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=build/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/_DOTDOT/src/TIesrFA_WM5.o

# C Compiler Flags
CFLAGS=-v -mno-cygwin

# CC Compiler Flags
CCFLAGS=-v -mno-cygwin
CXXFLAGS=-v -mno-cygwin

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-lwinmm

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-WindowsReleaseMinGW.mk ../../Dist/WindowsReleaseMinGW/libTIesrFAWin.dll

../../Dist/WindowsReleaseMinGW/libTIesrFAWin.dll: ${OBJECTFILES}
	${MKDIR} -p ../../Dist/WindowsReleaseMinGW
	${LINK.cc} -v -mno-cygwin -shared -o ../../Dist/${CND_CONF}/libTIesrFAWin.dll -fPIC ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/_ext/_DOTDOT/src/TIesrFA_WM5.o: nbproject/Makefile-${CND_CONF}.mk ../src/TIesrFA_WM5.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -Wall -DTIESRFA_EXPORTS -DWIN32 -DCYGWIN -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/src/TIesrFA_WM5.o ../src/TIesrFA_WM5.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf:
	${RM} -r build/WindowsReleaseMinGW
	${RM} ../../Dist/WindowsReleaseMinGW/libTIesrFAWin.dll

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
