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
CND_CONF=WindowsReleaseMinGW
CND_DISTDIR=dist

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=build/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/_DOTDOT/src/TIesrDict.o

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
LDLIBSOPTIONS=../../TIesrDT/TIesrDTso/../../Dist/WindowsReleaseMinGW/libTIesrDT.dll

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-WindowsReleaseMinGW.mk ../../Dist/WindowsReleaseMinGW/libTIesrDict.dll

../../Dist/WindowsReleaseMinGW/libTIesrDict.dll: ../../TIesrDT/TIesrDTso/../../Dist/WindowsReleaseMinGW/libTIesrDT.dll

../../Dist/WindowsReleaseMinGW/libTIesrDict.dll: ${OBJECTFILES}
	${MKDIR} -p ../../Dist/WindowsReleaseMinGW
	${LINK.cc} -shared -o ../../Dist/${CND_CONF}/libTIesrDict.dll -fPIC ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/_ext/_DOTDOT/src/TIesrDict.o: nbproject/Makefile-${CND_CONF}.mk ../src/TIesrDict.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -Wall -DTIESRDICT_EXPORTS -DWIN32 -I../../TIesrDT/src -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/src/TIesrDict.o ../src/TIesrDict.cpp

# Subprojects
.build-subprojects:
	cd ../../TIesrDT/TIesrDTso && ${MAKE}  -f Makefile CONF=WindowsReleaseMinGW

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/WindowsReleaseMinGW
	${RM} ../../Dist/WindowsReleaseMinGW/libTIesrDict.dll

# Subprojects
.clean-subprojects:
	cd ../../TIesrDT/TIesrDTso && ${MAKE}  -f Makefile CONF=WindowsReleaseMinGW clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
