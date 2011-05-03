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
	${OBJECTDIR}/_ext/_DOTDOT/src/jac_one.o \
	${OBJECTDIR}/_ext/_DOTDOT/src/liveproc.o \
	${OBJECTDIR}/_ext/_DOTDOT/src/engine_sireco_init.o \
	${OBJECTDIR}/_ext/_DOTDOT/src/jac-estm.o \
	${OBJECTDIR}/_ext/_DOTDOT/src/siinit.o \
	${OBJECTDIR}/_ext/_DOTDOT/src/siproc.o

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
LDLIBSOPTIONS=-Wl,-rpath ${CND_BASEDIR}/../../Dist/${CND_CONF}/lib ../TIesrEngineCoreso/../../Dist/ArmLinuxDebugGnueabi/lib/libTIesrEngineCore.so.1

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-ArmLinuxDebugGnueabi.mk ../../Dist/ArmLinuxDebugGnueabi/lib/libTIesrEngineSI.so.1

../../Dist/ArmLinuxDebugGnueabi/lib/libTIesrEngineSI.so.1: ../TIesrEngineCoreso/../../Dist/ArmLinuxDebugGnueabi/lib/libTIesrEngineCore.so.1

../../Dist/ArmLinuxDebugGnueabi/lib/libTIesrEngineSI.so.1: ${OBJECTFILES}
	${MKDIR} -p ../../Dist/ArmLinuxDebugGnueabi/lib
	${LINK.c} -Wl,-znow,-zdefs -Wl,-h,libTIesrEngineSI.so.1 -Wl,--version-script=../resource/TIesrEngineSI.ver -shared -o ../../Dist/${CND_CONF}/lib/libTIesrEngineSI.so.1 -fPIC ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/_ext/_DOTDOT/src/jac_one.o: nbproject/Makefile-${CND_CONF}.mk ../src/jac_one.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/src
	${RM} $@.d
	$(COMPILE.c) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/src/jac_one.o ../src/jac_one.c

${OBJECTDIR}/_ext/_DOTDOT/src/liveproc.o: nbproject/Makefile-${CND_CONF}.mk ../src/liveproc.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/src
	${RM} $@.d
	$(COMPILE.c) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/src/liveproc.o ../src/liveproc.c

${OBJECTDIR}/_ext/_DOTDOT/src/engine_sireco_init.o: nbproject/Makefile-${CND_CONF}.mk ../src/engine_sireco_init.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/src
	${RM} $@.d
	$(COMPILE.c) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/src/engine_sireco_init.o ../src/engine_sireco_init.c

${OBJECTDIR}/_ext/_DOTDOT/src/jac-estm.o: nbproject/Makefile-${CND_CONF}.mk ../src/jac-estm.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/src
	${RM} $@.d
	$(COMPILE.c) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/src/jac-estm.o ../src/jac-estm.c

${OBJECTDIR}/_ext/_DOTDOT/src/siinit.o: nbproject/Makefile-${CND_CONF}.mk ../src/siinit.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/src
	${RM} $@.d
	$(COMPILE.c) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/src/siinit.o ../src/siinit.c

${OBJECTDIR}/_ext/_DOTDOT/src/siproc.o: nbproject/Makefile-${CND_CONF}.mk ../src/siproc.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/src
	${RM} $@.d
	$(COMPILE.c) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/src/siproc.o ../src/siproc.c

# Subprojects
.build-subprojects:
	cd ../TIesrEngineCoreso && ${MAKE}  -f Makefile CONF=ArmLinuxDebugGnueabi

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/ArmLinuxDebugGnueabi
	${RM} ../../Dist/ArmLinuxDebugGnueabi/lib/libTIesrEngineSI.so.1

# Subprojects
.clean-subprojects:
	cd ../TIesrEngineCoreso && ${MAKE}  -f Makefile CONF=ArmLinuxDebugGnueabi clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
