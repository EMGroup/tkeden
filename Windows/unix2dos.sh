#!/bin/sh
#
# $Id: unix2dos.sh,v 1.1 2001/07/27 17:07:41 cssbz Exp $
#
# unix2dos.sh by Ashley Ward for cygwin
# (the line should include some nasty control characters - this
# appears to be the only way to force cygwin to perform this task)

sed 's/$//' $1 | tr '' '\r' > $2
