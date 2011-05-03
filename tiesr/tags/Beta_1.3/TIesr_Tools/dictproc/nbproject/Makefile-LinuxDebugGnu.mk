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
	${OBJECTDIR}/src/dictproc.o \
	${OBJECTDIR}/src/english.o \
	${OBJECTDIR}/src/map.o \
	${OBJECTDIR}/src/phoneme.o \
	${OBJECTDIR}/src/parse.o \
	${OBJECTDIR}/src/spellword.o \
	${OBJECTDIR}/src/saynum.o \
	${OBJECTDIR}/src/phones.o \
	${OBJECTDIR}/src/compress_entry.o

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
LDLIBSOPTIONS=-L../../Dist/${CND_CONF}/lib -Wl,-rpath ${CND_BASEDIR}/../../Dist/${CND_CONF}/lib -lTIesrDT

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-LinuxDebugGnu.mk ../../Dist/LinuxDebugGnu/bin/dictproc

../../Dist/LinuxDebugGnu/bin/dictproc: ${OBJECTFILES}
	${MKDIR} -p ../../Dist/LinuxDebugGnu/bin
	${LINK.c} -v -o ../../Dist/${CND_CONF}/bin/dictproc ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/src/dictproc.o: nbproject/Makefile-${CND_CONF}.mk src/dictproc.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.c) -g -Wall -Isrc -I../../TIesrDT/src -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/dictproc.o src/dictproc.c

${OBJECTDIR}/src/english.o: nbproject/Makefile-${CND_CONF}.mk src/english.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.c) -g -Wall -Isrc -I../../TIesrDT/src -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/english.o src/english.c

${OBJECTDIR}/src/map.o: nbproject/Makefile-${CND_CONF}.mk src/map.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.c) -g -Wall -Isrc -I../../TIesrDT/src -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/map.o src/map.c

${OBJECTDIR}/src/phoneme.o: nbproject/Makefile-${CND_CONF}.mk src/phoneme.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.c) -g -Wall -Isrc -I../../TIesrDT/src -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/phoneme.o src/phoneme.c

${OBJECTDIR}/src/parse.o: nbproject/Makefile-${CND_CONF}.mk src/parse.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.c) -g -Wall -Isrc -I../../TIesrDT/src -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/parse.o src/parse.c

${OBJECTDIR}/src/spellword.o: nbproject/Makefile-${CND_CONF}.mk src/spellword.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.c) -g -Wall -Isrc -I../../TIesrDT/src -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/spellword.o src/spellword.c

${OBJECTDIR}/src/saynum.o: nbproject/Makefile-${CND_CONF}.mk src/saynum.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.c) -g -Wall -Isrc -I../../TIesrDT/src -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/saynum.o src/saynum.c

${OBJECTDIR}/src/phones.o: nbproject/Makefile-${CND_CONF}.mk src/phones.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.c) -g -Wall -Isrc -I../../TIesrDT/src -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/phones.o src/phones.c

${OBJECTDIR}/src/compress_entry.o: nbproject/Makefile-${CND_CONF}.mk src/compress_entry.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.c) -g -Wall -Isrc -I../../TIesrDT/src -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/compress_entry.o src/compress_entry.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf:
	${RM} -r build/LinuxDebugGnu
	${RM} ../../Dist/LinuxDebugGnu/bin/dictproc

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
