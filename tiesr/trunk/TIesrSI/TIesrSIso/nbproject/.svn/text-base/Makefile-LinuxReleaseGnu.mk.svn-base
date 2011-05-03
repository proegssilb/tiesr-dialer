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
	${OBJECTDIR}/_ext/_DOTDOT/src/TIesrSI.o

# C Compiler Flags
CFLAGS=-v

# CC Compiler Flags
CCFLAGS=-v
CXXFLAGS=-v

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-L../../Dist/${CND_CONF}/lib -Wl,-rpath ${CND_BASEDIR}/../../Dist/${CND_CONF}/lib -lpthread ../../TIesrEngine/TIesrEngineSIso/../../Dist/LinuxReleaseGnu/lib/libTIesrEngineSI.so.1 ../../TIesrFA/TIesrFA_ALSAso/../../Dist/LinuxReleaseGnu/lib/libTIesrFA_ALSA.so.1

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-LinuxReleaseGnu.mk ../../Dist/LinuxReleaseGnu/lib/libTIesrSI.so.1

../../Dist/LinuxReleaseGnu/lib/libTIesrSI.so.1: ../../TIesrEngine/TIesrEngineSIso/../../Dist/LinuxReleaseGnu/lib/libTIesrEngineSI.so.1

../../Dist/LinuxReleaseGnu/lib/libTIesrSI.so.1: ../../TIesrFA/TIesrFA_ALSAso/../../Dist/LinuxReleaseGnu/lib/libTIesrFA_ALSA.so.1

../../Dist/LinuxReleaseGnu/lib/libTIesrSI.so.1: ${OBJECTFILES}
	${MKDIR} -p ../../Dist/LinuxReleaseGnu/lib
	${LINK.c} -Wl,-znow,-zdefs -Wl,-h,libTIesrSI.so.1 -Wl,--version-script=../resource/TIesrSIso.ver -shared -o ../../Dist/${CND_CONF}/lib/libTIesrSI.so.1 -fPIC ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/_ext/_DOTDOT/src/TIesrSI.o: nbproject/Makefile-${CND_CONF}.mk ../src/TIesrSI.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/src
	${RM} $@.d
	$(COMPILE.c) -O2 -Wall -DLINUX -I../../TIesrEngine/src -I../../TIesrFA/src -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/src/TIesrSI.o ../src/TIesrSI.c

# Subprojects
.build-subprojects:
	cd ../../TIesrEngine/TIesrEngineSIso && ${MAKE}  -f Makefile CONF=LinuxReleaseGnu
	cd ../../TIesrFA/TIesrFA_ALSAso && ${MAKE}  -f Makefile CONF=LinuxReleaseGnu

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/LinuxReleaseGnu
	${RM} ../../Dist/LinuxReleaseGnu/lib/libTIesrSI.so.1

# Subprojects
.clean-subprojects:
	cd ../../TIesrEngine/TIesrEngineSIso && ${MAKE}  -f Makefile CONF=LinuxReleaseGnu clean
	cd ../../TIesrFA/TIesrFA_ALSAso && ${MAKE}  -f Makefile CONF=LinuxReleaseGnu clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
