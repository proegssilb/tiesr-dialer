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
CC=arm-none-linux-gnueabi-gcc
CCC=arm-none-linux-gnueabi-g++
CXX=arm-none-linux-gnueabi-g++
FC=
AS=arm-none-linux-gnueabi-as

# Macros
CND_PLATFORM=arm-none-linux-gnueabi-Linux-x86
CND_CONF=ArmLinuxDebugGnueabi
CND_DISTDIR=dist

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=build/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/_DOTDOT/src/TIesrFA_ALSA.o

# C Compiler Flags
CFLAGS=-v -idirafter ../../Tools/ARM/include

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-L../../Tools/ARM/lib -Wl,-rpath ${CND_BASEDIR}/../../Tools/ARM/lib -lpthread -lasound

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-ArmLinuxDebugGnueabi.mk ../../Dist/ArmLinuxDebugGnueabi/lib/libTIesrFA_ALSA.so.1

../../Dist/ArmLinuxDebugGnueabi/lib/libTIesrFA_ALSA.so.1: ${OBJECTFILES}
	${MKDIR} -p ../../Dist/ArmLinuxDebugGnueabi/lib
	${LINK.c} -Wl,-znow,-zdefs -Wl,-h,libTIesrFA_ALSA.so.1 -Wl,--version-script=../resource/TIesrFAso.ver -shared -o ../../Dist/${CND_CONF}/lib/libTIesrFA_ALSA.so.1 -fPIC ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/_ext/_DOTDOT/src/TIesrFA_ALSA.o: nbproject/Makefile-${CND_CONF}.mk ../src/TIesrFA_ALSA.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/src
	${RM} $@.d
	$(COMPILE.c) -g -Wall -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/src/TIesrFA_ALSA.o ../src/TIesrFA_ALSA.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf:
	${RM} -r build/ArmLinuxDebugGnueabi
	${RM} ../../Dist/ArmLinuxDebugGnueabi/lib/libTIesrFA_ALSA.so.1

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
