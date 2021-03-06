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
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU_current-Linux-x86
CND_CONF=LinuxReleaseGnu
CND_DISTDIR=dist

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=build/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/_DOTDOT/src/TIesrFA_ALSA.o

# C Compiler Flags
CFLAGS=-v

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-lpthread -lasound

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-LinuxReleaseGnu.mk ../../Dist/LinuxReleaseGnu/lib/libTIesrFA_ALSA.so.1

../../Dist/LinuxReleaseGnu/lib/libTIesrFA_ALSA.so.1: ${OBJECTFILES}
	${MKDIR} -p ../../Dist/LinuxReleaseGnu/lib
	${LINK.c} -Wl,-znow,-zdefs -Wl,-h,libTIesrFA_ALSA.so.1 -Wl,--version-script=../resource/TIesrFAso.ver -shared -o ../../Dist/${CND_CONF}/lib/libTIesrFA_ALSA.so.1 -fPIC ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/_ext/_DOTDOT/src/TIesrFA_ALSA.o: nbproject/Makefile-${CND_CONF}.mk ../src/TIesrFA_ALSA.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/src
	${RM} $@.d
	$(COMPILE.c) -O2 -Wall -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/src/TIesrFA_ALSA.o ../src/TIesrFA_ALSA.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf:
	${RM} -r build/LinuxReleaseGnu
	${RM} ../../Dist/LinuxReleaseGnu/lib/libTIesrFA_ALSA.so.1

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
