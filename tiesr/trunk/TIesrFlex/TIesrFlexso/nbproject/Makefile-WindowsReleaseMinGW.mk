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
	${OBJECTDIR}/_ext/_DOTDOT/src/TIesrFlex.o

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
LDLIBSOPTIONS=../../TIesrDict/TIesrDictso/../../Dist/WindowsReleaseMinGW/libTIesrDict.dll

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-WindowsReleaseMinGW.mk ../../Dist/WindowsReleaseMinGW/libTIesrFlex.dll

../../Dist/WindowsReleaseMinGW/libTIesrFlex.dll: ../../TIesrDict/TIesrDictso/../../Dist/WindowsReleaseMinGW/libTIesrDict.dll

../../Dist/WindowsReleaseMinGW/libTIesrFlex.dll: ${OBJECTFILES}
	${MKDIR} -p ../../Dist/WindowsReleaseMinGW
	${LINK.cc} -v -shared -o ../../Dist/${CND_CONF}/libTIesrFlex.dll -fPIC ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/_ext/_DOTDOT/src/TIesrFlex.o: nbproject/Makefile-${CND_CONF}.mk ../src/TIesrFlex.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -Wall -DTIESRFLEX_EXPORTS -DWIN32 -I../../TIesrDict/src -I../../TIesrDT/src -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/src/TIesrFlex.o ../src/TIesrFlex.cpp

# Subprojects
.build-subprojects:
	cd ../../TIesrDict/TIesrDictso && ${MAKE}  -f Makefile CONF=WindowsReleaseMinGW

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/WindowsReleaseMinGW
	${RM} ../../Dist/WindowsReleaseMinGW/libTIesrFlex.dll

# Subprojects
.clean-subprojects:
	cd ../../TIesrDict/TIesrDictso && ${MAKE}  -f Makefile CONF=WindowsReleaseMinGW clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
