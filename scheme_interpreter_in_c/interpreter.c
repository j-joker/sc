#include <stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<string.h>
enum
{
	T_INT,
	T_CELL,
	T_SYMBOL,
	T_PRIMITIVE,
	T_FUNCTION,
	T_MACRO,
	T_ENV,
	T_NIL,
	T_PAREN,
	T_DOT,
	T_TRUE

};

typedef struct Obj
{
	int type;
	union {
		int value;
		struct
		{
			struct Obj *car;
			struct Obj *cdr;
		};
		char *str;
		struct
		{

			struct Obj *vars;
			struct Obj *up;
		};
	};
} Obj;

static Obj *Nil;
static Obj *Dot;
static Obj *Par;
static Obj *True;

//环境
static Obj *env;
//符号表
static Obj *symbols;

Obj *make_sym(char *);
Obj *cons(Obj *car,Obj *cdr){
	Obj *o =malloc(sizeof(Obj));
	o->car=car;
	o->cdr=cdr;
	return o;
}

void add_var(Obj *env,Obj *sym ,Obj *val){
	env->vars=cons(cons(sym,val),env->vars);
}
Obj *find(Obj *env,Obj* sym){
	for(Obj *p=env ;p;p=p->up){
		for(Obj *x=p->vars;x!=Nil;x=x->cdr){
			Obj *bind=x->car;
			if(bind->car==sym){
				return bind->cdr;
			}
		}
	}
	
}

Obj *intern(char *name)
{
	for (Obj *p = symbols; p != Nil; p = p->cdr)
	{
			if(strcmp(p->car->str,name)==0){
				return p->car;
			}
	}
	Obj *sym=make_sym(name);
	symbols=cons(sym,symbols);
	return sym;
}

Obj *make_sym(char *name)
{
	Obj *o = malloc(sizeof(Obj));
	o->str = name;
	o->type = T_SYMBOL;
	return o;
}

Obj *make_special(int type)
{
	Obj *o = malloc(sizeof(Obj));
	o->type = type;
	return o;
}
//vars是用来干嘛的
Obj *make_env(Obj *vars, Obj *up)
{
	Obj *env = malloc(sizeof(Obj));
	env->vars = vars;
	env->up = up;
	return env;
}

Obj *make_int(int num){
	Obj *o=malloc(sizeof(Obj));
	o->type=T_INT;
	o->value=num;
	return o;
}

Obj *eval(Obj *env, Obj *obj)
{
	switch (obj->type)
	{
	case T_INT:
	case T_NIL:
	case T_PAREN:
		return obj;
	case T_SYMBOL:
		return find(env,obj);
	default:
		return NULL;
	}
}

void print(Obj *o)
{
	switch (o->type)
	{
	case T_INT:
		printf("%d", o->value);
		return;
	case T_SYMBOL:
		printf("%s",o->str);
	case T_TRUE:
		printf("#t");
	
	}
	
}
int peek(FILE *in){
	int c=getc(in);
	ungetc(c,in);
	return c;
}

Obj * read_sym(FILE *in){
	char buf[1024];
	int c=getc(in);
	int i=0;
	while(isalpha(c)){
		buf[i++]=c;
		c=getc(in);
	}
	buf[i]='\0';
	Obj* o=intern(buf);
	return o;
}

int read_num(FILE *in){
	int c;
	int sum=0;
	int sign=1;
	c=getc(in);
	while(isdigit(c)||c=='-'){
		if(c=='-'){
			sign=-1;
			c=getc(in);
			continue;
		}
		sum=sum*10+c-'0';
		c=getc(in);
	}
	sum*=sign;
	ungetc(c,in);
	return sum;
}

Obj *read(FILE *in)
{
	int c;
	while (1)
	{
		c = getc(in);
		if (isspace(c))
			continue;
		if (c == NULL)
			return NULL;
		if(isdigit(c)||c=='-'){
			ungetc(c,in);
			Obj *num=make_int(read_num(in));
			return num;
		}
		if(isalpha(c)){
			ungetc(c,in);
			Obj * sym=read_sym(in);
			return sym;
		}
	}
}

void define_constant(Obj *env){
	Obj *sym=intern("t");
	add_var(env,sym,True);
}

int main()
{
	//给一些单例的常量赋值
	Nil = make_special(T_NIL);
	Dot = make_special(T_DOT);
	Par = make_special(T_PAREN);
	True = make_special(T_TRUE);
	symbols=Nil;
	//环境
	env = make_env(Nil, NULL);
	//内置的value
	define_constant(env);
	//内置的函数
	
	printf("******Welcom to Lisp World******\n\n");
	while(1){
		printf(">");
		Obj *exp=read(stdin);
		print(eval(env,exp));
		printf("\n");
	}

	return 0;
}