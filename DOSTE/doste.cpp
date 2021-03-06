#include <doste/dvm/oid.h>
#include <doste/file.h>
#include <doste/doste.h>
#include <doste/dasm.h>
#include <doste/dvm/buffer.h>

#define DOSTE
extern "C" {
#include "../Eden/eden.h"
#include "../Eden/hash.h"
extern symptr hashtable[HASHSIZE+1];
extern void schedule_parents_of(symptr);
void doste_trigger(char*,int);
};

#include <unistd.h>
#include <sys/time.h>

#define EDEN_STYPE_NORMAL 264
#define EDEN_STYPE_DEFINITION 265
#define EDEN_STYPE_FUNC 272
#define EDEN_STYPE_PROC 273
#define EDEN_STYPE_HIER 274

#define EDEN_TYPE_INT 259
#define EDEN_TYPE_FLOAT 258
#define EDEN_TYPE_STRING 261
#define EDEN_TYPE_CHAR 260
#define EDEN_TYPE_LIST 262

using namespace doste;
using namespace doste::dvm;

struct EdenOID {
	unsigned int a; unsigned int b; unsigned int c; unsigned int d;
};

class OIDPair {
	public:
	friend inline bool operator==(const OIDPair &p1, const OIDPair &p2);

	doste::dvm::OID obj;
	doste::dvm::OID key;
};

bool operator==(const OIDPair &p1, const OIDPair &p2) {
	return (p1.obj == p2.obj && p1.key == p2.key);
}

struct EdenEntry {
	OID name;
	OID def;
	char *dstr_c;
	OID dstr;
	List<OIDPair> deps;
	EdenEntry *next;
};

class Eden : public Handler {
	public:
	Eden();
	~Eden();
	
	bool handle(Event &evt);
	bool baseEden(Event &evt);

	//private:
	static const unsigned int EDEN_HASHSIZE = 100;
	static EdenEntry *edenhash[EDEN_HASHSIZE];
	static int hashOID(const OID &o) { return o.d() % EDEN_HASHSIZE; };

	static EdenEntry *find(char *name);
};

EdenEntry *Eden::edenhash[EDEN_HASHSIZE] = {0};

Eden::Eden() : Handler(OID(0,100,0,0),OID(0,100,0,1000)) {}

Eden::~Eden() {}

EdenEntry *Eden::find(char *name) {
	OID temp = OID(name);
	int hash = hashOID(temp);
	EdenEntry *res = edenhash[hash];
	while (res != 0) {
		if (res->name == temp) return res;
		res = res->next;
	}
	
	res = new EdenEntry;
	res->dstr_c = 0;
	res->name = temp;
	res->next = edenhash[hash];
	edenhash[hash] = res;
	return res;
}

bool Eden::baseEden(Event &evt) {
	int context = evt.dest().d();

	//std::cout << "HANDLE\n";

	if (evt.type() == Event::GET) {
		char buf[100];
		evt.param<0>().toString(buf,100);
		symptr sym = lookup(buf, context);

		if (sym == 0) {
			evt.result(Null);
			return true;
		}

		//std::cout << "Symbol: stype=" << sym->stype << " type=" << sym->d.type << "\n"
		EdenEntry *e;

		// if (sym->changed) {
		//	std::cout << "SYMBOL NOT UPDATED\n";
		// }

		switch(sym->stype) {
		case EDEN_STYPE_DEFINITION:
		case EDEN_STYPE_NORMAL:
			switch(sym->d.type) {
			case EDEN_TYPE_INT: evt.result(sym->d.u.i); break;
			case EDEN_TYPE_FLOAT: evt.result(sym->d.u.r); break;
			case EDEN_TYPE_STRING:
				e = find(buf);
				if (e->dstr_c != sym->d.u.s) {
					e->dstr = dstring(sym->d.u.s);
					e->dstr_c = sym->d.u.s;
				}
				evt.result(e->dstr);
				break;
			default: evt.result(Null);
			}
			break;
		
		default:
			evt.result(Null);
		}
		return true;
	} else if (evt.type() == Event::GET_KEYS) {
		
		symptr      sp;
		int count = 0;

		for (int i=0; i<HASHSIZE; i++) {
			for (sp = hashtable[i]; sp != (symptr) 0; sp = sp->next) {
				if (sp->context == context)
					count++;
			}
		}

		OID boid = Buffer::create(B_OID, count);
		OID temp;
		Buffer *keys = Buffer::lookup(boid);

		int j=0;
		for (int i=0; i<HASHSIZE; i++) {
			for (sp = hashtable[i]; sp != (symptr) 0; sp = sp->next) {
				if (sp->context == context) {
					temp = OID(sp->name);
					keys->seti(j++,temp);
				}
				//std::cout << " KEY " << sp->name << "\n";
			}
		}

		evt.result(boid);
		return true;

	} else if (evt.type() == Event::ADDDEP) {
		char buf[100];
		evt.param<0>().toString(buf,100);
		symptr sym = lookup(buf, context);

		if (sym == 0) {
			return true;
		}

		sym->doste = 1;

		EdenEntry *e = find(buf);
		OIDPair pair;
		pair.key = evt.param<2>();
		pair.obj = evt.param<1>();
	
		e->deps.addFront(pair);
		e->deps.unique(pair);

		return true;
	} else if (evt.type() == Event::NOTIFY) {
		char buf[100];
		evt.param<0>().toString(buf,100);
		EdenEntry *e = find(buf);

		//mark_changed(sym);

		if (e->def != Null /*&& (e->flags & OID::FLAG_FUNCTION) == 0*/) {
			Event *evt2 = new Event(Event::SET, evt.dest());
			evt2->param<0>(evt.param<0>());
			evt2->param<1>(Definition(e->def).evaluate(evt.dest(), evt.param<0>(), false));
			evt2->send(Event::FLAG_FREE);
		}
		return true;

	} else if (evt.type() == Event::DEFINE) {
		char buf[100];
		evt.param<0>().toString(buf,100);
		EdenEntry *e = find(buf);

		e->def = evt.param<1>();
		//e->flags = OID::FLAG_

		Event *evt2 = new Event(Event::NOTIFY, evt.dest());
		evt2->param<0>(evt.param<0>());
		evt2->param<1>(Null);
		evt2->param<2>(Null);
		evt2->send(Event::FLAG_FREE);

		return true;
	} else if (evt.type() == Event::SET) {
		char buf[100];
		evt.param<0>().toString(buf,100);
		symptr sym = lookup(buf, context);

		if (sym == 0) {
			sym = install(buf, context, EDEN_STYPE_NORMAL, EDEN_TYPE_INT, (int)evt.param<1>());
			return true;
		}


		symbol sym2;
		sym2.d = sym->d;

		//Check if already exists...
		if (evt.param<1>().isFloat()) {
			sym->d.type = 258;
			sym->d.u.r = (float)evt.param<1>();
		} else if (evt.param<1>().isInt()) {
			sym->d.type = 259;
			sym->d.u.i = (int)evt.param<1>();
		} else {
			if (((OID)evt.param<1>()[0]).isChar() && (evt.param<1>()[Size] != Null)) {
				dstring temp(evt.param<1>());
				sym->d.u.s = new char[temp.size()+1];
				temp.toString(sym->d.u.s,temp.size()+1);
				sym->d.type = 261;
			}
		}

		if (sym2.d.type != sym->d.type || sym2.d.u.v.x != sym->d.u.v.x || sym2.d.u.v.y != sym->d.u.v.y) {
			doste_trigger(buf, context);
			schedule_parents_of(sym);
		}
		return true;
	} else if (evt.type() == Event::GETDEF) {
		evt.result(Null);
		return true;
	}

	return false;
}

bool Eden::handle(Event &evt) {
	//if (evt.dest().b() == 101) {
		return baseEden(evt);
	//} else {
	//	//return contextEden(evt);
	//}
	return false;
}


//HACK
char parser_data[3000];
int parser_i = 0;

extern "C" {

void doste_parse(char c) {
	parser_data[parser_i++] = c;
	if (c == ';') {
		parser_data[parser_i] = 0;
		DASM *dasm = (DASM*)((OID)dvm::root["notations"]["dasm"]);
		
		int j=0;
		while (parser_data[j] == ' ' || parser_data[j] == '\n' || parser_data[j] == '\t')
			j++;
		
		if (parser_data[j] == '?') {
			parser_data[j] = ' ';
			OID result = dasm->execute(parser_data,dvm::root);
			char buf[200];
			result.toString(buf,200);
			std::cout << buf << "\n";
		} else {
			dasm->execute(parser_data,dvm::root);
		}
		//std::cout << "DASM: " << parser_data << "\n";
		parser_i = 0;
	}
}

int doste_init() {
	doste::initialise(0,0);
	
	new Eden();
	
	dvm::root["notations"]["dasm"]["run"] = new LocalFile("/dcs/emp/empublic/doste32/share/doste/scripts/linux.dasm");
	dvm::root["notations"]["dasm"]["run"] = new LocalFile("config.dasm");

	dvm::root["eden"] = OID::create();
	dvm::root["eden"]["base"] = OID(0,100,0,OID("base").d());
}

void doste_update() {
	doste::update();
}

void doste_trigger(char *name, int context) {
	// std::cout << "CHANGE: " << name << "\n";
	EdenEntry *e = Eden::find(name);
	Event *evt;

	//Notify things TODO
	for (List<OIDPair>::iterator i=e->deps.begin(); i!=e->deps.end(); i++) {
		//The list must be unique to prevent excess notifies?
		//Now dealt with by notify so this does not matter.
		//m_lock.wait();
		//m_deps.unique(*i);
		//m_lock.free();

		//Generate the notify event for this object.
		evt = new Event(Event::NOTIFY, (*i).obj);
		evt->param<0>((*i).key);
		
		//Agents rely on the following but they really should not.
		evt->param<1>(OID(0,100,0,context));
		evt->param<2>(OID(name));
		//evt->param<3>(m_value); //NOTE: TO BE REMOVED

		evt->send(Event::FLAG_FREE);
	}

	e->deps.clear();

	/*Event *evt = new Event(Event::NOTIFY, e->obj);
	evt->param<0>(e->attr);
	evt->param<1>(OID(0,100,0,0));
	evt->param<2>(OID(name));
	evt->send(Event::FLAG_FREE);*/
}

EdenOID *doste_lookup (EdenOID *a, EdenOID *b) {
	EdenOID *result = new EdenOID;
	OID temp=OID(a->a,a->b,a->c,a->d)[OID(b->a,b->b,b->c,b->d)];
	result->a = temp.a();
	result->b = temp.b();
	result->c = temp.c();
	result->d = temp.d();
	return result;
}

int doste_a(EdenOID *e) {
	return e->a;
}

int doste_b(EdenOID *e) {
	return e->b;
}

int doste_c(EdenOID *e) {
	return e->c;
}

int doste_d(EdenOID *e) {
	return e->d;
}

EdenOID *doste_edenoid(int a, int b, int c, int d) {
	EdenOID *result = new EdenOID;
	result->a = a;
	result->b = b;
	result->c = c;
	result->d = d;
	return result;
}

EdenOID *doste_oid (char *name) {
	EdenOID *result = new EdenOID;
	OID temp=OID(name);
	result->a = temp.a();
	result->b = temp.b();
	result->c = temp.c();
	result->d = temp.d();
	return result;
}

char *doste_name (EdenOID *o) {
	char *buff = new char[100];
	OID temp=OID(o->a, o->b, o->c, o->d);
	temp.toString(buff,100);
	return buff;
}

void doste_set(EdenOID *p1, EdenOID *p2, EdenOID *p3) {
	OID temp1=OID(p1->a, p1->b, p1->c, p1->d);
	OID temp2=OID(p2->a, p2->b, p2->c, p2->d);
	OID temp3=OID(p3->a, p3->b, p3->c, p3->d);
	temp1.set(temp2,temp3);
}

}



