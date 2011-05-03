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
CND_CONF=ArmLinuxReleaseGnueabi
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
CCFLAGS=-v
CXXFLAGS=-v

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-L../../Dist/${CND_CONF}/lib -Wl,-rpath ${CND_BASEDIR}/../../Dist/${CND_CONF}/lib ../../TIesrDict/TIesrDictso/../../Dist/ArmLinuxReleaseGnueabi/lib/libTIesrDict.so.1

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-ArmLinuxReleaseGnueabi.mk ../../Dist/ArmLinuxReleaseGnueabi/lib/libTIesrFlex.so.1

../../Dist/ArmLinuxReleaseGnueabi/lib/libTIesrFlex.so.1: ../../TIesrDict/TIesrDictso/../../Dist/ArmLinuxReleaseGnueabi/lib/libTIesrDict.so.1

../../Dist/ArmLinuxReleaseGnueabi/lib/libTIesrFlex.so.1: ${OBJECTFILES}
	${MKDIR} -p ../../Dist/ArmLinuxReleaseGnueabi/lib
	${LINK.cc} -Wl,-znow,-zdefs -Wl,-h,libTIesrFlex.so.1 -Wl,--version-script=../resource/TIesrFlexso.ver -shared -o ../../Dist/${CND_CONF}/lib/libTIesrFlex.so.1 -fPIC ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/_ext/_DOTDOT/src/TIesrFlex.o: nbproject/Makefile-${CND_CONF}.mk ../src/TIesrFlex.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/_DOTDOT/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -Wall -I../../TIesrDict/src -I../../TIesrDT/src -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/_DOTDOT/src/TIesrFlex.o ../src/TIesrFlex.cpp

# Subprojects
.build-subprojects:
	cd ../../TIesrDict/TIesrDictso && ${MAKE}  -f Makefile CONF=ArmLinuxReleaseGnueabi

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/ArmLinuxReleaseGnueabi
	${RM} ../../Dist/ArmLinuxReleaseGnueabi/lib/libTIesrFlex.so.1

# Subprojects
.clean-subprojects:
	cd ../../TIesrDict/TIesrDictso && ${MAKE}  -f Makefile CONF=ArmLinuxReleaseGnueabi clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
