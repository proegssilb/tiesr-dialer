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
	${OBJECTDIR}/src/ClusterUtil.o \
	${OBJECTDIR}/src/GaussianCluster.o

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
LDLIBSOPTIONS=-lm

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-WindowsDebugMinGW.mk ../../Dist/WindowsDebugMinGW/gaussiancluster.exe

../../Dist/WindowsDebugMinGW/gaussiancluster.exe: ${OBJECTFILES}
	${MKDIR} -p ../../Dist/WindowsDebugMinGW
	${LINK.c} -v -mno-cygwin -o ../../Dist/${CND_CONF}/gaussiancluster.exe ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/src/ClusterUtil.o: nbproject/Makefile-${CND_CONF}.mk src/ClusterUtil.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.c) -g -Wall -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/ClusterUtil.o src/ClusterUtil.c

${OBJECTDIR}/src/GaussianCluster.o: nbproject/Makefile-${CND_CONF}.mk src/GaussianCluster.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.c) -g -Wall -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/GaussianCluster.o src/GaussianCluster.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf:
	${RM} -r build/WindowsDebugMinGW
	${RM} ../../Dist/WindowsDebugMinGW/gaussiancluster.exe

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
