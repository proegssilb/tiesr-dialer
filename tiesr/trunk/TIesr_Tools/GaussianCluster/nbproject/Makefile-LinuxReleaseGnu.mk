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
	${OBJECTDIR}/src/ClusterUtil.o \
	${OBJECTDIR}/src/GaussianCluster.o

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
LDLIBSOPTIONS=-lm

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-LinuxReleaseGnu.mk ../../Dist/LinuxReleaseGnu/bin/gaussiancluster

../../Dist/LinuxReleaseGnu/bin/gaussiancluster: ${OBJECTFILES}
	${MKDIR} -p ../../Dist/LinuxReleaseGnu/bin
	${LINK.c} -v -o ../../Dist/${CND_CONF}/bin/gaussiancluster ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/src/ClusterUtil.o: nbproject/Makefile-${CND_CONF}.mk src/ClusterUtil.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.c) -O2 -Wall -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/ClusterUtil.o src/ClusterUtil.c

${OBJECTDIR}/src/GaussianCluster.o: nbproject/Makefile-${CND_CONF}.mk src/GaussianCluster.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.c) -O2 -Wall -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/GaussianCluster.o src/GaussianCluster.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf:
	${RM} -r build/LinuxReleaseGnu
	${RM} ../../Dist/LinuxReleaseGnu/bin/gaussiancluster

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
