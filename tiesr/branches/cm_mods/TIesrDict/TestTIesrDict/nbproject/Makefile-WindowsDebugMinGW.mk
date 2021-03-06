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
CC=
CCC=
CXX=
FC=
AS=

# Macros
CND_PLATFORM=Cygwin-Linux-x86
CND_CONF=WindowsDebugMinGW
CND_DISTDIR=dist

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=build/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/_DOTDOT/src/TestTIesrDict.o

# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=-v -mno-cygwin
CXXFLAGS=-v -mno-cygwin

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-L../../Dist/${CND_CONF}/ -lTIesrDict

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-WindowsDebugMinGW.mk ../../Dist/WindowsDebugMinGW/testtiesrdict.exe

../../Dist/WindowsDebugMinGW/testtiesrdict.exe: ${OBJECTFILES}
	${MKDIR} -p ../../Dist/WindowsDebugMinGW
	${LINK.cc} -v -mno-cygwin -o ../../Dist/${CND_CONF}/testtiesrdict.exe ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/_ext/_DOTDOT/src/TestTIesrDict.o: nbproject/Makefile-${CND_CONF}.mk ../src/TestTIesrDict.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I../src -I../../TIesrDT/src -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/src/TestTIesrDict.o ../src/TestTIesrDict.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf:
	${RM} -r build/WindowsDebugMinGW
	${RM} ../../Dist/WindowsDebugMinGW/testtiesrdict.exe

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
