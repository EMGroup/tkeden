/*
 * $Id: print.c,v 1.8 2001/07/27 16:58:33 cssbz Exp $
 *
 *  This file is part of Eden.
 *
 *  Eden is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Eden is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Eden; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

static char rcsid[] = "$Id: print.c,v 1.8 2001/07/27 16:58:33 cssbz Exp $";

#include "../config.h"

#include <stdio.h>

#include "symbol.h"
#include "tree.h"
#include "parser.h"

/* function prototypes */
char *scoutType(int);
char *just_name(int);
void printdef(tree *);
void printsym(symbol *);

#ifdef __STDC__
#define show(field)\
if (w.field) {\
	outputFormattedMessage("\n    %s: ", #field);\
	printdef(w.field);\
}

#define dot(x) \
printdef(t->l.t); \
outputFormattedMessage(".%s", #x)

#else /* not __STDC__ */

#define show(field)\
if (w.field) {\
	outputFormattedMessage("\n    field: ");\
	printdef(w.field);\
}

#define dot(x) \
printdef(t->l.t); \
outputFormattedMessage(".x")
#endif

/*
 scoutType - return the type name of type
 */
char * scoutType(int type) {
	switch (type) {
	case STRVAR:
		return "string";
	case INTVAR:
		return "integer";
	case PTVAR:
		return "point";
	case BOXVAR:
		return "box";
	case FRAMEVAR:
		return "frame";
	case WINVAR:
		return "window";
	case DISPVAR:
		return "display";
	case UNKNOWN:
		return "unknown";
	case IMGVAR:
		return "image";
	default:
		return "error !!!";
	}
}

/*
 just_name - return the corresponding string of just
 */
char * just_name(int just) {
	switch (just) {
	case 0:
		return "NOADJ";
	case 1:
		return "LEFT";
	case 2:
		return "RIGHT";
	case 3:
		return "CENTRE";
	case 4:
		return "EXPAND";
	default:
		return "error !!!";
	}
}

/*
 printdef - unparse the tree t and print the result
 */
void printdef(tree * t) {
	WinStruct w;
	tree *arg;

	if (!t) {
		outputFormattedMessage("@");
		return;
	}
	switch (t->op) {
	case '=':
		if (t->r.t) {
			outputFormattedMessage("%s = ", t->l.v->name);
			printdef(t->r.t);
		} else {
			outputFormattedMessage("%s", t->l.v->name);
		}
		break;
	case NUMBER:
		outputFormattedMessage("%g", t->l.i);
		break;
	case ROW:
		printdef(t->l.t);
		outputFormattedMessage(".r");
		break;
	case COLUMN:
		printdef(t->l.t);
		outputFormattedMessage(".c");
		break;
	case STR:
		outputFormattedMessage("\"%s\"", t->l.s);
		break;
	case IMGVAR:
	case UNKNOWN:
	case STRVAR:
	case INTVAR:
	case PTVAR:
	case BOXVAR:
	case FRAMEVAR:
	case WINVAR:
	case DISPVAR:
		outputFormattedMessage("%s", t->l.v->name);
		break;
	case FORMPT:
		outputFormattedMessage("{");
		printdef(t->l.t);
		outputFormattedMessage(", ");
		printdef(t->r.t);
		outputFormattedMessage("}");
		break;
	case FORMBOX:
		outputFormattedMessage("[");
		printdef(t->l.t);
		outputFormattedMessage(", ");
		printdef(t->r.t);
		outputFormattedMessage("]");
		break;
	case TEXTBOX:
		outputFormattedMessage("[");
		printdef(t->l.t);
		outputFormattedMessage(", ");
		printdef(t->r.t->l.t);
		outputFormattedMessage(", ");
		printdef(t->r.t->r.t);
		outputFormattedMessage("]");
		break;
	case '(':
	case FORMFRAME:
		outputFormattedMessage("(");
		printdef(t->l.t);
		outputFormattedMessage(")");
		break;
	case FORMWIN:
		w = t->l.w;
		outputFormattedMessage("{\n    type: ");
		/* !@!@ This should really use the constants array [Ash] */
		switch (w.type) {
		case 0:
			outputFormattedMessage("TEXT");
			break;
		case 1:
			outputFormattedMessage("DONALD");
			break;
		case 2:
			outputFormattedMessage("ARCA");
			break;
		case 3:
			outputFormattedMessage("IMAGE");
			break;
		case 4:
			outputFormattedMessage("TEXTBOX");
			break;
		default:
			outputFormattedMessage("UNKNOWN");
			break;
		}
		;
		show(frame)
		;
		show(string)
		;
		show(box)
		;
		show(pict)
		;
		show(xmin)
		;
		show(ymin)
		;
		show(xmax)
		;
		show(ymax)
		;
		show(font)
		;
		show(bgcolor)
		;
		show(fgcolor)
		;
		show(bdcolor)
		;
		show(border)
		;
		if (w.bdtype) {
			outputFormattedMessage("\n    relief: ");
			printdef(w.bdtype);
		}
		if (w.align != -1)
			outputFormattedMessage("\n    alignment: %s", just_name(w.align));
		if (w.sensitive)
			outputFormattedMessage("\n    sensitive: ON");
		outputFormattedMessage("\n}");
		break;
	case FORMDISP:
		outputFormattedMessage("<");
		printdef(t->l.t);
		outputFormattedMessage(">");
		break;
	case CONCAT:
		printdef(t->l.t);
		outputFormattedMessage(" // ");
		printdef(t->r.t);
		break;
	case STRCAT:
		outputFormattedMessage("strcat(");
		printdef(t->l.t);
		outputFormattedMessage(", ");
		printdef(t->r.t);
		outputFormattedMessage(")");
		break;
	case STRLEN:
		outputFormattedMessage("strlen(");
		printdef(t->l.t);
		outputFormattedMessage(")");
		break;
	case SUBSTR:
		outputFormattedMessage("substr(");
		printdef(t->l.t);
		outputFormattedMessage(", ");
		printdef(t->r.t->l.t);
		outputFormattedMessage(", ");
		printdef(t->r.t->r.t);
		outputFormattedMessage(")");
		break;
	case TOSTRING:
		outputFormattedMessage("itos(");
		printdef(t->l.t);
		outputFormattedMessage(")");
		break;
	case BOXSHIFT:
		outputFormattedMessage("shift(");
		printdef(t->l.t);
		outputFormattedMessage(", ");
		printdef(t->r.t->l.t);
		outputFormattedMessage(", ");
		printdef(t->r.t->r.t);
		outputFormattedMessage(")");
		break;
	case BOXINTERSECT:
		outputFormattedMessage("intersect(");
		printdef(t->l.t);
		outputFormattedMessage(", ");
		printdef(t->r.t);
		outputFormattedMessage(")");
		break;
	case BOXCENTRE:
		outputFormattedMessage("centre(");
		printdef(t->l.t);
		outputFormattedMessage(", ");
		printdef(t->r.t);
		outputFormattedMessage(")");
		break;
	case BOXENCLOSING:
		outputFormattedMessage("enclose(");
		printdef(t->l.t);
		outputFormattedMessage(", ");
		printdef(t->r.t);
		outputFormattedMessage(")");
		break;
	case BOXREDUCE:
		outputFormattedMessage("reduce(");
		printdef(t->l.t);
		outputFormattedMessage(", ");
		printdef(t->r.t);
		outputFormattedMessage(")");
		break;
	case DOTNE:
		dot(ne)
		;
		break;
	case DOTNW:
		dot(nw)
		;
		break;
	case DOTSE:
		dot(se)
		;
		break;
	case DOTSW:
		dot(sw)
		;
		break;
	case DOTN:
		dot(n)
		;
		break;
	case DOTE:
		dot(e)
		;
		break;
	case DOTS:
		dot(s)
		;
		break;
	case DOTW:
		dot(w)
		;
		break;
	case '.':
		printdef(t->l.t);
		outputFormattedMessage(".");
		printdef(t->r.t);
		break;
	case DOTFRAME:
		dot(frame)
		;
		break;
	case DOTSTR:
		dot(string)
		;
		break;
	case DOTBOX:
		dot(box)
		;
		break;
	case DOTTYPE:
		dot(type)
		;
		break;
	case DOTPICT:
		dot(pict)
		;
		break;
	case DOTXMIN:
		dot(xmin)
		;
		break;
	case DOTYMIN:
		dot(ymin)
		;
		break;
	case DOTXMAX:
		dot(xmax)
		;
		break;
	case DOTYMAX:
		dot(ymax)
		;
		break;
	case DOTFONT:
		dot(font)
		;
		break;
	case DOTBG:
		dot(bgcolor)
		;
		break;
	case DOTFG:
		dot(fgcolor)
		;
		break;
	case DOTBDCOLOR:
		dot(bdcolor)
		;
		break;
	case DOTBORDER:
		dot(border)
		;
		break;
	case DOTBDTYPE:
		dot(relief)
		;
		break;
	case DOTALIGN:
		dot(alignment)
		;
		break;
	case DOTSENSITIVE:
		dot(sensitive)
		;
		break;
	case IMGFUNC:
		outputFormattedMessage("%s(", t->l.s);
		for (arg = t->r.t; arg->op; arg = arg->r.t) {
			printdef(arg->l.t);
			if (arg->r.t->op)
				outputFormattedMessage(", ");
		}
		outputFormattedMessage(")");
		break;
	case APPEND:
		outputFormattedMessage("append(");
		printdef(t->l.t);
		outputFormattedMessage(", ");
		printdef(t->r.t->l.t);
		outputFormattedMessage(", ");
		printdef(t->r.t->r.t);
		outputFormattedMessage(")");
		break;
	case DELETE:
		outputFormattedMessage("delete(");
		printdef(t->l.t);
		outputFormattedMessage(", ");
		printdef(t->r.t);
		outputFormattedMessage(")");
		break;
	case '&':
		printdef(t->l.t);
		outputFormattedMessage(" & ");
		printdef(t->r.t);
		break;
	case ',':
		printdef(t->l.t);
		outputFormattedMessage(", ");
		printdef(t->r.t);
		break;
	case '>':
		printdef(t->l.t);
		outputFormattedMessage(" / ");
		printdef(t->r.t);
		break;
	case UMINUS:
		outputFormattedMessage("-");
		printdef(t->l.t);
		break;
	case '+':
		printdef(t->l.t);
		outputFormattedMessage(" + ");
		printdef(t->r.t);
		break;
	case '-':
		printdef(t->l.t);
		outputFormattedMessage(" - ");
		printdef(t->r.t);
		break;
	case '*':
		printdef(t->l.t);
		outputFormattedMessage(" * ");
		printdef(t->r.t);
		break;
	case '/':
		printdef(t->l.t);
		outputFormattedMessage(" / ");
		printdef(t->r.t);
		break;
	case '%':
		printdef(t->l.t);
		outputFormattedMessage(" %% ");
		printdef(t->r.t);
		break;
	case EQ:
		printdef(t->l.t);
		outputFormattedMessage(" == ");
		printdef(t->r.t);
		break;
	case NE:
		printdef(t->l.t);
		outputFormattedMessage(" != ");
		printdef(t->r.t);
		break;
	case LT:
		printdef(t->l.t);
		outputFormattedMessage(" < ");
		printdef(t->r.t);
		break;
	case LE:
		printdef(t->l.t);
		outputFormattedMessage(" <= ");
		printdef(t->r.t);
		break;
	case GT:
		printdef(t->l.t);
		outputFormattedMessage(" > ");
		printdef(t->r.t);
		break;
	case GE:
		printdef(t->l.t);
		outputFormattedMessage(" >= ");
		printdef(t->r.t);
		break;
	case OR:
		printdef(t->l.t);
		outputFormattedMessage(" || ");
		printdef(t->r.t);
		break;
	case AND:
		printdef(t->l.t);
		outputFormattedMessage(" && ");
		printdef(t->r.t);
		break;
	case IF:
		outputFormattedMessage("if ");
		printdef(t->l.t);
		outputFormattedMessage(" then ");
		printdef(t->r.t->l.t);
		outputFormattedMessage(" else ");
		printdef(t->r.t->r.t);
		outputFormattedMessage(" endif");
		break;
	default:
		outputFormattedMessage("Unknown operator!");
		break;
	}
}

/*
 printsym - print the type, variable name and the definition
 of the variable pointed by v
 */
void printsym(symbol * v) {
	if (v->type != UNKNOWN) {
		if (v->def) {
			/* print the type and variable name */
			outputFormattedMessage("%s %s = ", scoutType(v->type), v->name);
			/* print the definition */
			printdef(v->def);
			outputFormattedMessage(";\n");
		} else {
			/* print the type and variable name */
			outputFormattedMessage("%s %s;\n", scoutType(v->type), v->name);
		}
	}
}
