/*	$NetBSD: msg_296.c,v 1.5 2024/06/08 06:37:06 rillig Exp $	*/
# 3 "msg_296.c"

// Test for message: conversion of negative constant %lld to unsigned type '%s', arg #%d [296]

/* lint1-extra-flags: -X 351 */

void take_unsigned_int(unsigned int);

void
example(void)
{
	/* expect+1: warning: conversion of negative constant -3 to unsigned type 'unsigned int', arg #1 [296] */
	take_unsigned_int(-3);
}
