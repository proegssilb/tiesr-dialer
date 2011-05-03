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
	${OBJECTDIR}/_ext/_DOTDOT/src/TIesrDT.o

# C Compiler Flags
CFLAGS=-v -mno-cygwin

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-WindowsDebugMinGW.mk ../../Dist/WindowsDebugMinGW/libTIesrDT.dll

../../Dist/WindowsDebugMinGW/libTIesrDT.dll: ${OBJECTFILES}
	${MKDIR} -p ../../Dist/WindowsDebugMinGW
	${LINK.c} -v -shared -o ../../Dist/${CND_CONF}/libTIesrDT.dll -fPIC ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/_ext/_DOTDOT/src/TIesrDT.o: nbproject/Makefile-${CND_CONF}.mk ../src/TIesrDT.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/src
	${RM} $@.d
	$(COMPILE.c) -g -Wall -DTIESRDT_EXPORTS -DWIN32 -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/src/TIesrDT.o ../src/TIesrDT.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf:
	${RM} -r build/WindowsDebugMinGW
	${RM} ../../Dist/WindowsDebugMinGW/libTIesrDT.dll

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
