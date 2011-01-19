#ifndef _GBOOK_
#define _GBOOK_

#define CH_BLOCK 10
//#define _DEBUG

struct gbook_choice {
	int link;
	char* text;

	char* require;
	char* gain;
	int life;
	int money;
};

struct gbook_act {
	int id;
	char* text;
	struct gbook_choice* choices;
	int n_choice;
};

struct gbook_engine {
	struct gbook_act* action;
	int n_action;
};

struct user_configs {
	char* name;

	int money;
	int lifep;

	char** objects;
	int    n_object;
};

struct gbook_object {
	char* name;
	int money;
};

struct gbook_merchant {
	struct gbook_object* objs;
	int n_obj;
};

void loadBook( const char* );
void saveConfigs( char*, int );
void loadProfile( const char* );

void loadMerchant( const char* );

#endif /* _GBOOK_ */
