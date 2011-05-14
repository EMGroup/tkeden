
/* Workaround a bug in strncat if necessary. Note this file is in the Eden
   directory but included from some of the other directories... it should
   really live in the MacOSX directory but I don't want to go to the bother
   of writing a Makefile etc etc for that.  [Ash] */

#ifdef __APPLE__
#define strncat strncatFIX
#endif
