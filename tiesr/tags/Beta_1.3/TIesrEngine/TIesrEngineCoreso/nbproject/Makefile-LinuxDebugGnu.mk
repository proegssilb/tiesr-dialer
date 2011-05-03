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
CND_CONF=LinuxDebugGnu
CND_DISTDIR=dist

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=build/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/_DOTDOT/src/gmhmm_type_common.o \
	${OBJECTDIR}/_ext/_DOTDOT/src/noise_sub.o \
	${OBJECTDIR}/_ext/_DOTDOT/src/rapidsearch.o \
	${OBJECTDIR}/_ext/_DOTDOT/src/pmc_f.o \
	${OBJECTDIR}/_ext/_DOTDOT/src/uttdet.o \
	${OBJECTDIR}/_ext/_DOTDOT/src/obsprob.o \
	${OBJECTDIR}/_ext/_DOTDOT/src/mfcc_f.o \
	${OBJECTDIR}/_ext/_DOTDOT/src/load.o \
	${OBJECTDIR}/_ext/_DOTDOT/src/pack.o \
	${OBJECTDIR}/_ext/_DOTDOT/src/dist.o \
	${OBJECTDIR}/_ext/_DOTDOT/src/sbc.o \
	${OBJECTDIR}/_ext/_DOTDOT/src/search.o \
	${OBJECTDIR}/_ext/_DOTDOT/src/volume.o

# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=-v
CXXFLAGS=-v

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-LinuxDebugGnu.mk ../../Dist/LinuxDebugGnu/lib/libTIesrEngineCore.so.1

../../Dist/LinuxDebugGnu/lib/libTIesrEngineCore.so.1: ${OBJECTFILES}
	${MKDIR} -p ../../Dist/LinuxDebugGnu/lib
	${LINK.cc} -Wl,-znow,-zdefs -Wl,-h,libTIesrEngineCore.so.1 -Wl,--version-script=../resource/TIesrEngineCore.ver -shared -o ../../Dist/${CND_CONF}/lib/libTIesrEngineCore.so.1 -fPIC ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/_ext/_DOTDOT/src/gmhmm_type_common.o: nbproject/Makefile-${CND_CONF}.mk ../src/gmhmm_type_common.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/src/gmhmm_type_common.o ../src/gmhmm_type_common.cpp

${OBJECTDIR}/_ext/_DOTDOT/src/noise_sub.o: nbproject/Makefile-${CND_CONF}.mk ../src/noise_sub.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/src/noise_sub.o ../src/noise_sub.cpp

${OBJECTDIR}/_ext/_DOTDOT/src/rapidsearch.o: nbproject/Makefile-${CND_CONF}.mk ../src/rapidsearch.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/src/rapidsearch.o ../src/rapidsearch.cpp

${OBJECTDIR}/_ext/_DOTDOT/src/pmc_f.o: nbproject/Makefile-${CND_CONF}.mk ../src/pmc_f.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/src/pmc_f.o ../src/pmc_f.cpp

${OBJECTDIR}/_ext/_DOTDOT/src/uttdet.o: nbproject/Makefile-${CND_CONF}.mk ../src/uttdet.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/src/uttdet.o ../src/uttdet.cpp

${OBJECTDIR}/_ext/_DOTDOT/src/obsprob.o: nbproject/Makefile-${CND_CONF}.mk ../src/obsprob.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/src/obsprob.o ../src/obsprob.cpp

${OBJECTDIR}/_ext/_DOTDOT/src/mfcc_f.o: nbproject/Makefile-${CND_CONF}.mk ../src/mfcc_f.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/src/mfcc_f.o ../src/mfcc_f.cpp

${OBJECTDIR}/_ext/_DOTDOT/src/load.o: nbproject/Makefile-${CND_CONF}.mk ../src/load.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/src/load.o ../src/load.cpp

${OBJECTDIR}/_ext/_DOTDOT/src/pack.o: nbproject/Makefile-${CND_CONF}.mk ../src/pack.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/src/pack.o ../src/pack.cpp

${OBJECTDIR}/_ext/_DOTDOT/src/dist.o: nbproject/Makefile-${CND_CONF}.mk ../src/dist.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/src/dist.o ../src/dist.cpp

${OBJECTDIR}/_ext/_DOTDOT/src/sbc.o: nbproject/Makefile-${CND_CONF}.mk ../src/sbc.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/src/sbc.o ../src/sbc.cpp

${OBJECTDIR}/_ext/_DOTDOT/src/search.o: nbproject/Makefile-${CND_CONF}.mk ../src/search.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/src/search.o ../src/search.cpp

${OBJECTDIR}/_ext/_DOTDOT/src/volume.o: nbproject/Makefile-${CND_CONF}.mk ../src/volume.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/src
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/src/volume.o ../src/volume.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf:
	${RM} -r build/LinuxDebugGnu
	${RM} ../../Dist/LinuxDebugGnu/lib/libTIesrEngineCore.so.1

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
