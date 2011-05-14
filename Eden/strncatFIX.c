
/* There is a bug in strncat on 10.5 which only really affects things
   when compiling on 10.6.  See this post where someone has tracked down
   the details (note sadly there is no resolution given there, however). 
   <http://lists.apple.com/archives/cocoa-dev/2009/Aug/msg01763.html>. 
   To work around this, we'll compile our own strncat and make sure this
   version is used on the Mac.  The following source is from
   <http://www.opensource.apple.com/source/Libc/Libc-498/string/strncat-fbsd.c>.
   [Ash] */

#include <string.h>

/*
 * Concatenate src on the end of dst.  At most strlen(dst)+n+1 bytes
 * are written at dst (at most n+1 bytes being appended).  Return dst.
 */
char *
strncatFIX(char * __restrict dst, const char * __restrict src, size_t n)
{
	if (n != 0) {
		char *d = dst;
		const char *s = src;

		while (*d != 0)
			d++;
		do {
			if ((*d = *s++) == 0)
				break;
			d++;
		} while (--n != 0);
		*d = 0;
	}
	return (dst);
}
