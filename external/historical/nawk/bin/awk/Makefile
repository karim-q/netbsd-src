#	$NetBSD: Makefile,v 1.1 2024/08/17 22:22:29 christos Exp $

WARNS?= 4
CWARNFLAGS.clang+=	-Wno-self-assign

.include <bsd.own.mk>

.if !defined(HOSTPROG)
MAKETABDIR != cd ${.CURDIR}/../maketab && ${PRINTOBJDIR}
MAKETAB = ${MAKETABDIR}/maketab
.else
MAKETABDIR != cd ${NETBSDSRCDIR}/tools/maketab && ${PRINTOBJDIR}
MAKETAB = ${MAKETABDIR}/${_TOOLPREFIX}maketab
.endif


BINDIR?= /usr/bin
DIST=	${.CURDIR}/../../dist
.PATH:	${DIST}

PROG=	awk
SRCS=	awkgram.y b.c lex.c lib.c main.c parse.c proctab.c run.c tran.c
CPPFLAGS+=	-I${DIST} -I. -DHAS_ISBLANK
LDADD+=	-lm
.if !defined(HOSTPROG)
DPADD+=	${LIBM}
.endif
YHEADER=	yes
CWARNFLAGS+=	-Wno-pointer-sign
COPTS.run.c+=	-Wno-format-nonliteral
COPTS.tran.c+=	-Wno-format-nonliteral

# info file originally from GNU awk 3.1.3, adjusted for nawk slightly
.PATH:	${NETBSDSRCDIR}/external/gpl2/gawk/dist
TEXINFO=        awk.info

# During the tools build (from src/tools/awk/Makefile),
# src/tools/Makefile.host changes .CURDIR back and forth between
# src/tools/awk and src/usr.bin/awk.  For some unknown reason, including
# bsd.info.mk here leads to the obj dir being created at the wrong time,
# while .CURDIR is src/usr.bin/awk.  Work around the problem by not
# including bsd.info.mk when MKINFO=no.
.if ${MKINFO} != "no"
.include <bsd.info.mk>
.endif

proctab.c: ${MAKETAB} awkgram.h
	${.ALLSRC} > ${.TARGET}

CLEANFILES += proctab.c

.include <bsd.prog.mk>
