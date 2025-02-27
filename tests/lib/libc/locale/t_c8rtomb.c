/*	$NetBSD: t_c8rtomb.c,v 1.2 2024/08/17 21:31:22 riastradh Exp $	*/

/*-
 * Copyright (c) 2002 Tim J. Robbins
 * All rights reserved.
 *
 * Copyright (c) 2013 Ed Schouten <ed@FreeBSD.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/*
 * Test program for c8rtomb() as specified by C23.
 */

#include <sys/cdefs.h>
__RCSID("$NetBSD: t_c8rtomb.c,v 1.2 2024/08/17 21:31:22 riastradh Exp $");

#include <errno.h>
#include <limits.h>
#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <uchar.h>

#include <atf-c.h>

static void
require_lc_ctype(const char *locale_name)
{
	char *lc_ctype_set;

	lc_ctype_set = setlocale(LC_CTYPE, locale_name);
	if (lc_ctype_set == NULL)
		atf_tc_fail("setlocale(LC_CTYPE, \"%s\") failed; errno=%d",
		    locale_name, errno);

	ATF_REQUIRE_EQ_MSG(strcmp(lc_ctype_set, locale_name), 0,
	    "lc_ctype_set=%s locale_name=%s", lc_ctype_set, locale_name);
}

static mbstate_t s;
static char buf[MB_LEN_MAX + 1];

ATF_TC_WITHOUT_HEAD(c8rtomb_c_locale_test);
ATF_TC_BODY(c8rtomb_c_locale_test, tc)
{
	size_t n;

	require_lc_ctype("C");

	/*
	 * If the buffer argument is NULL, c8 is implicitly 0,
	 * c8rtomb() resets its internal state.
	 */
	ATF_CHECK_EQ_MSG((n = c8rtomb(NULL, '\0', NULL)), 1, "n=%zu", n);
	ATF_CHECK_EQ_MSG((n = c8rtomb(NULL, 0x80, NULL)), 1, "n=%zu", n);
	ATF_CHECK_EQ_MSG((n = c8rtomb(NULL, 0xc0, NULL)), 1, "n=%zu", n);
	ATF_CHECK_EQ_MSG((n = c8rtomb(NULL, 0xe0, NULL)), 1, "n=%zu", n);
	ATF_CHECK_EQ_MSG((n = c8rtomb(NULL, 0xf0, NULL)), 1, "n=%zu", n);
	ATF_CHECK_EQ_MSG((n = c8rtomb(NULL, 0xf8, NULL)), 1, "n=%zu", n);
	ATF_CHECK_EQ_MSG((n = c8rtomb(NULL, 0xfc, NULL)), 1, "n=%zu", n);
	ATF_CHECK_EQ_MSG((n = c8rtomb(NULL, 0xfe, NULL)), 1, "n=%zu", n);
	ATF_CHECK_EQ_MSG((n = c8rtomb(NULL, 0xff, NULL)), 1, "n=%zu", n);

	/* Null wide character. */
	memset(&s, 0, sizeof(s));
	memset(buf, 0xcc, sizeof(buf));
	ATF_CHECK_EQ_MSG((n = c8rtomb(buf, 0, &s)), 1, "n=%zu", n);
	ATF_CHECK_MSG(((unsigned char)buf[0] == 0 &&
		(unsigned char)buf[1] == 0xcc),
	    "buf=[%02x %02x]", buf[0], buf[1]);

	/* Latin letter A, internal state. */
	ATF_CHECK_EQ_MSG((n = c8rtomb(NULL, '\0', NULL)), 1, "n=%zu", n);
	ATF_CHECK_EQ_MSG((n = c8rtomb(NULL, 'A', NULL)), 1, "n=%zu", n);

	/* Latin letter A. */
	memset(&s, 0, sizeof(s));
	memset(buf, 0xcc, sizeof(buf));
	ATF_CHECK_EQ_MSG((n = c8rtomb(buf, 'A', &s)), 1, "n=%zu", n);
	ATF_CHECK_MSG(((unsigned char)buf[0] == 'A' &&
		(unsigned char)buf[1] == 0xcc),
	    "buf=[%02x %02x]", buf[0], buf[1]);

	/* Unicode character 'Pile of poo'. */
	memset(&s, 0, sizeof(s));
	memset(buf, 0xcc, sizeof(buf));
	ATF_CHECK_EQ_MSG((n = c8rtomb(buf, 0xf0, &s)), 0, "n=%zu", n);
	ATF_CHECK_EQ_MSG((n = c8rtomb(buf, 0x9f, &s)), 0, "n=%zu", n);
	ATF_CHECK_EQ_MSG((n = c8rtomb(buf, 0x92, &s)), 0, "n=%zu", n);
	ATF_CHECK_EQ_MSG((n = c8rtomb(buf, 0xa9, &s)), (size_t)-1,
	    "n=%zu", n);
	ATF_CHECK_EQ_MSG(errno, EILSEQ, "errno=%d", errno);
	ATF_CHECK_EQ_MSG((unsigned char)buf[0], 0xcc, "buf=[%02x]", buf[0]);

	/* Incomplete Unicode character 'Pile of poo', interrupted by NUL. */
	memset(&s, 0, sizeof(s));
	memset(buf, 0xcc, sizeof(buf));
	ATF_CHECK_EQ_MSG((n = c8rtomb(buf, 0xf0, &s)), 0, "n=%zu", n);
	atf_tc_expect_fail("PR lib/58615:"
	    " incomplete c8rtomb, c16rtomb handles NUL termination wrong");
	ATF_CHECK_EQ_MSG((n = c8rtomb(buf, '\0', &s)), 1, "n=%zu", n);
	ATF_CHECK_MSG(((unsigned char)buf[0] == '\0' &&
		(unsigned char)buf[1] == 0xcc),
	    "buf=[%02x %02x]", buf[0], buf[1]);

	memset(&s, 0, sizeof(s));
	memset(buf, 0xcc, sizeof(buf));
	ATF_CHECK_EQ_MSG((n = c8rtomb(buf, 0xf0, &s)), 0, "n=%zu", n);
	ATF_CHECK_EQ_MSG((n = c8rtomb(buf, 0x9f, &s)), 0, "n=%zu", n);
	ATF_CHECK_EQ_MSG((n = c8rtomb(buf, '\0', &s)), 1, "n=%zu", n);
	ATF_CHECK_MSG(((unsigned char)buf[0] == '\0' &&
		(unsigned char)buf[1] == 0xcc),
	    "buf=[%02x %02x]", buf[0], buf[1]);

	memset(&s, 0, sizeof(s));
	memset(buf, 0xcc, sizeof(buf));
	ATF_CHECK_EQ_MSG((n = c8rtomb(buf, 0xf0, &s)), 0, "n=%zu", n);
	ATF_CHECK_EQ_MSG((n = c8rtomb(buf, 0x9f, &s)), 0, "n=%zu", n);
	ATF_CHECK_EQ_MSG((n = c8rtomb(buf, 0x92, &s)), 0, "n=%zu", n);
	ATF_CHECK_EQ_MSG((n = c8rtomb(buf, '\0', &s)), 1, "n=%zu", n);
	ATF_CHECK_MSG(((unsigned char)buf[0] == '\0' &&
		(unsigned char)buf[1] == 0xcc),
	    "buf=[%02x %02x]", buf[0], buf[1]);
}

ATF_TC_WITHOUT_HEAD(c8rtomb_iso_8859_1_test);
ATF_TC_BODY(c8rtomb_iso_8859_1_test, tc)
{
	size_t n;

	require_lc_ctype("en_US.ISO8859-1");

	/* Unicode character 'Euro sign'. */
	memset(&s, 0, sizeof(s));
	memset(buf, 0xcc, sizeof(buf));
	ATF_CHECK_EQ_MSG((n = c8rtomb(buf, 0xe2, &s)), 0, "n=%zu", n);
	ATF_CHECK_EQ_MSG((n = c8rtomb(buf, 0x82, &s)), 0, "n=%zu", n);
	ATF_CHECK_EQ_MSG((n = c8rtomb(buf, 0xac, &s)), (size_t)-1,
	    "n=%zu", n);
	ATF_CHECK_EQ_MSG(errno, EILSEQ, "errno=%d", errno);
	ATF_CHECK_EQ_MSG((unsigned char)buf[0], 0xcc, "buf=[%02x]", buf[0]);
}

ATF_TC_WITHOUT_HEAD(c8rtomb_iso_8859_15_test);
ATF_TC_BODY(c8rtomb_iso_8859_15_test, tc)
{
	size_t n;

	require_lc_ctype("en_US.ISO8859-15");

	/* Unicode character 'Euro sign'. */
	memset(&s, 0, sizeof(s));
	memset(buf, 0xcc, sizeof(buf));
	ATF_CHECK_EQ_MSG((n = c8rtomb(buf, 0xe2, &s)), 0, "n=%zu", n);
	ATF_CHECK_EQ_MSG((n = c8rtomb(buf, 0x82, &s)), 0, "n=%zu", n);
	ATF_CHECK_EQ_MSG((n = c8rtomb(buf, 0xac, &s)), 1, "n=%zu", n);
	ATF_CHECK_MSG(((unsigned char)buf[0] == 0xa4 &&
		(unsigned char)buf[1] == 0xcc),
	    "buf=[%02x %02x]", buf[0], buf[1]);
}

ATF_TC_WITHOUT_HEAD(c8rtomb_utf_8_test);
ATF_TC_BODY(c8rtomb_utf_8_test, tc)
{
	size_t n;

	require_lc_ctype("en_US.UTF-8");

	/* Unicode character 'Pile of poo'. */
	memset(&s, 0, sizeof(s));
	memset(buf, 0xcc, sizeof(buf));
	ATF_CHECK_EQ_MSG((n = c8rtomb(buf, 0xf0, &s)), 0, "n=%zu", n);
	ATF_CHECK_EQ_MSG((n = c8rtomb(buf, 0x9f, &s)), 0, "n=%zu", n);
	ATF_CHECK_EQ_MSG((n = c8rtomb(buf, 0x92, &s)), 0, "n=%zu", n);
	ATF_CHECK_EQ_MSG((n = c8rtomb(buf, 0xa9, &s)), 4, "n=%zu", n);
	ATF_CHECK_MSG(((unsigned char)buf[0] == 0xf0 &&
		(unsigned char)buf[1] == 0x9f &&
		(unsigned char)buf[2] == 0x92 &&
		(unsigned char)buf[3] == 0xa9 &&
		(unsigned char)buf[4] == 0xcc),
	    "buf=[%02x %02x %02x %02x %02x]",
	    buf[0], buf[1], buf[2], buf[3], buf[4]);

	/* Invalid code; 'Pile of poo' without the last byte. */
	memset(&s, 0, sizeof(s));
	memset(buf, 0xcc, sizeof(buf));
	ATF_CHECK_EQ_MSG((n = c8rtomb(buf, 0xf0, &s)), 0, "n=%zu", n);
	ATF_CHECK_EQ_MSG((n = c8rtomb(buf, 0x9f, &s)), 0, "n=%zu", n);
	ATF_CHECK_EQ_MSG((n = c8rtomb(buf, 0x92, &s)), 0, "n=%zu", n);
	ATF_CHECK_EQ_MSG((n = c8rtomb(buf, 'A', &s)), (size_t)-1,
	    "n=%zu", n);
	ATF_CHECK_EQ_MSG(errno, EILSEQ, "errno=%d", errno);
	ATF_CHECK_EQ_MSG((unsigned char)buf[0], 0xcc, "buf=[%02x]", buf[0]);

	/* Invalid code; 'Pile of poo' without the first byte. */
	memset(&s, 0, sizeof(s));
	memset(buf, 0xcc, sizeof(buf));
	ATF_CHECK_EQ_MSG((n = c8rtomb(buf, 0x9f, &s)), (size_t)-1,
	    "n=%zu", n);
	ATF_CHECK_EQ_MSG(errno, EILSEQ, "errno=%d", errno);
	ATF_CHECK_EQ_MSG((unsigned char)buf[0], 0xcc, "buf=[%02x]", buf[0]);

	/* Incomplete Unicode character 'Pile of poo', interrupted by NUL. */
	memset(&s, 0, sizeof(s));
	memset(buf, 0xcc, sizeof(buf));
	ATF_CHECK_EQ_MSG((n = c8rtomb(buf, 0xf0, &s)), 0, "n=%zu", n);
	atf_tc_expect_fail("PR lib/58615:"
	    " incomplete c8rtomb, c16rtomb handles NUL termination wrong");
	ATF_CHECK_EQ_MSG((n = c8rtomb(buf, '\0', &s)), 1, "n=%zu", n);
	ATF_CHECK_MSG(((unsigned char)buf[0] == '\0' &&
		(unsigned char)buf[1] == 0xcc),
	    "buf=[%02x %02x]", buf[0], buf[1]);

	memset(&s, 0, sizeof(s));
	memset(buf, 0xcc, sizeof(buf));
	ATF_CHECK_EQ_MSG((n = c8rtomb(buf, 0xf0, &s)), 0, "n=%zu", n);
	ATF_CHECK_EQ_MSG((n = c8rtomb(buf, 0x9f, &s)), 0, "n=%zu", n);
	ATF_CHECK_EQ_MSG((n = c8rtomb(buf, '\0', &s)), 1, "n=%zu", n);
	ATF_CHECK_MSG(((unsigned char)buf[0] == '\0' &&
		(unsigned char)buf[1] == 0xcc),
	    "buf=[%02x %02x]", buf[0], buf[1]);

	memset(&s, 0, sizeof(s));
	memset(buf, 0xcc, sizeof(buf));
	ATF_CHECK_EQ_MSG((n = c8rtomb(buf, 0xf0, &s)), 0, "n=%zu", n);
	ATF_CHECK_EQ_MSG((n = c8rtomb(buf, 0x9f, &s)), 0, "n=%zu", n);
	ATF_CHECK_EQ_MSG((n = c8rtomb(buf, 0x92, &s)), 0, "n=%zu", n);
	ATF_CHECK_EQ_MSG((n = c8rtomb(buf, '\0', &s)), 1, "n=%zu", n);
	ATF_CHECK_MSG(((unsigned char)buf[0] == '\0' &&
		(unsigned char)buf[1] == 0xcc),
	    "buf=[%02x %02x]", buf[0], buf[1]);
}

ATF_TP_ADD_TCS(tp)
{

	ATF_TP_ADD_TC(tp, c8rtomb_c_locale_test);
	ATF_TP_ADD_TC(tp, c8rtomb_iso_8859_1_test);
	ATF_TP_ADD_TC(tp, c8rtomb_iso_8859_15_test);
	ATF_TP_ADD_TC(tp, c8rtomb_utf_8_test);

	return (atf_no_error());
}
