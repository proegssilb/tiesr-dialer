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
	${OBJECTDIR}/_ext/_DOTDOT/src/TIesrSI.o

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
LDLIBSOPTIONS=../../TIesrEngine/TIesrEngineSIso/../../Dist/WindowsReleaseMinGW/libTIesrEngineSI.dll ../../TIesrFA/TIesrFAWinso/../../Dist/WindowsReleaseMinGW/libTIesrFAWin.dll

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-WindowsReleaseMinGW.mk ../../Dist/WindowsReleaseMinGW/libTIesrSI.dll

../../Dist/WindowsReleaseMinGW/libTIesrSI.dll: ../../TIesrEngine/TIesrEngineSIso/../../Dist/WindowsReleaseMinGW/libTIesrEngineSI.dll

../../Dist/WindowsReleaseMinGW/libTIesrSI.dll: ../../TIesrFA/TIesrFAWinso/../../Dist/WindowsReleaseMinGW/libTIesrFAWin.dll

../../Dist/WindowsReleaseMinGW/libTIesrSI.dll: ${OBJECTFILES}
	${MKDIR} -p ../../Dist/WindowsReleaseMinGW
	${LINK.c} -shared -o ../../Dist/${CND_CONF}/libTIesrSI.dll -fPIC ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/_ext/_DOTDOT/src/TIesrSI.o: nbproject/Makefile-${CND_CONF}.mk ../src/TIesrSI.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/src
	${RM} $@.d
	$(COMPILE.c) -O2 -Wall -DTIESRSI_EXPORTS -DWIN32 -I../../TIesrEngine/src -I../../TIesrFA/src -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/src/TIesrSI.o ../src/TIesrSI.c

# Subprojects
.build-subprojects:
	cd ../../TIesrEngine/TIesrEngineSIso && ${MAKE}  -f Makefile CONF=WindowsReleaseMinGW
	cd ../../TIesrFA/TIesrFAWinso && ${MAKE}  -f Makefile CONF=WindowsReleaseMinGW

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/WindowsReleaseMinGW
	${RM} ../../Dist/WindowsReleaseMinGW/libTIesrSI.dll

# Subprojects
.clean-subprojects:
	cd ../../TIesrEngine/TIesrEngineSIso && ${MAKE}  -f Makefile CONF=WindowsReleaseMinGW clean
	cd ../../TIesrFA/TIesrFAWinso && ${MAKE}  -f Makefile CONF=WindowsReleaseMinGW clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
