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
	T_TRUE,
	T_CParen

};
struct Obj;
typedef struct Obj *Primitive(struct Obj *env ,struct Obj *args);

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
		Primitive *fn;
		struct
		{

			struct Obj *vars;
			struct Obj *up;
		};
		struct {
			struct Obj *params;
			struct Obj *body;
			struct Obj *env;
		};
	};
} Obj;

Obj *read(FILE *in);
Obj *intern(char *);
Obj *make_prim(Primitive *);
Obj *make_int(int);
Obj *eval_list(Obj *,Obj *);
Obj *progn(Obj *,Obj *);
Obj *eval(Obj *,Obj *);
Obj *find(Obj *,Obj *);
Obj *make_env(Obj *,Obj *);
Obj *make_function(Obj *,Obj *,Obj *);
void add_var(Obj *,Obj *,Obj *);

static Obj *Nil;
static Obj *Dot;
static Obj *Par;
static Obj *True;
static Obj *CParen;

//环境
static Obj *env;
//符号表
static Obj *symbols;


static void error(char *err){
	fprintf(stderr,err);
	exit(1);
}
static void show_env(Obj *env){
	Obj *vars=env->vars;
	
}

int list_length(Obj *list){
	int len=0;
	while(1){
	if(list==Nil){
		return len;
	}
	list=list->cdr;
	len++;
	 }
}

Obj *prim_quote(Obj *env,Obj *args){
	return args->car;
}

Obj *prim_plus(Obj *env,Obj *args){
	int sum=0;
	args=eval_list(env,args);
	while(args!=Nil){
		sum+=args->car->value;
		args=args->cdr;
	}	
	return make_int(sum);
}

Obj *prim_list(Obj *env,Obj *args){
	return eval_list(env,args);
}

Obj *prim_define(Obj *env,Obj *args){

	Obj *sym=args->car;
	Obj *val=args->cdr->car;
	Obj *new_val=eval(env,val);
	add_var(env,sym,new_val);
	return new_val;
}

Obj *prim_lambda(Obj *env,Obj *args){
	Obj *params=args->car;
	Obj *body=args->cdr;
	return make_function(params,body,env);
}

Obj *prim_setq(Obj *env,Obj *args){
	Obj *sym=args->car;
	Obj *val=args->cdr->car;
	Obj *o=find(env,sym);
	if(o==Nil){
		printf("unknown symbol: %s",sym->str);
		exit(1);
	}
	Obj *new_arg=eval(env,val);
	o->cdr=new_arg;
	return new_arg;
}


Obj *make_sym(char *);
Obj *cons(Obj *car,Obj *cdr){
	Obj *o =malloc(sizeof(Obj));
	o->car=car;
	o->cdr=cdr;
	o->type=T_CELL;
	return o;
}

void add_var(Obj *env,Obj *sym ,Obj *val){
	env->vars=cons(cons(sym,val),env->vars);
}
void add_prim(Obj *env,char *name ,Primitive *fn){
	Obj *sym=intern(name);
	Obj *fun = make_prim(fn);
	add_var(env,sym,fun);
}
Obj *find(Obj *env,Obj* sym){
	for(Obj *p=env ;p;p=p->up){
		for(Obj *x=p->vars;x!=Nil;x=x->cdr){
			Obj *bind=x->car;
			if(bind->car==sym){
				return bind;
			}
		}
	}
	return Nil;
	
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
	o->str =malloc(strlen(name)+1);
	strcpy(o->str,name);
	o->type = T_SYMBOL;
	return o;
}

Obj *make_function(Obj *params,Obj *body,Obj *env){
	Obj *o=malloc(sizeof(Obj));
	o->params=params;
	o->body=body;
	o->env=env;
	o->type=T_FUNCTION;
	return o;
}

Obj *make_special(int type)
{
	Obj *o = malloc(sizeof(Obj));
	o->type = type;
	return o;
}

Obj *push_env(Obj *env,Obj *vars,Obj *vals){
	Obj *p=vars,*q=vals;
	Obj *frame=Nil;
	while(p!=Nil&&q!=Nil){
		Obj *bind=cons(p->car,q->car);
		frame=cons(bind,frame);
		p=p->cdr;
		q=q->cdr;
	}
	return make_env(frame,env);
	
}

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
Obj *make_prim(Primitive *fn){
	Obj *o =malloc(sizeof(Obj));
	o->type=T_PRIMITIVE;
	o->fn=fn;
	return o;
}

Obj *apply(Obj *env,Obj *fn ,Obj  *args){
	if(fn->type==T_PRIMITIVE){
		return fn->fn(env,args);
	}else if(fn->type==T_FUNCTION){
		Obj *params=fn->params;
		Obj *body=fn->body;
		Obj *env=fn->env;
		Obj *new_args=eval_list(env,args);
		Obj *new_env=push_env(env,params,new_args);
		return progn(new_env,body);
	}
}
Obj *progn(Obj *env,Obj *list){
	Obj *re=Nil;
	for(Obj *p=list;p!=Nil;p=p->cdr){
		re=eval(env,p->car);
	}
	return re;
}


Obj *eval_list(Obj *env,Obj *list){
	Obj *head=NULL,*tail=NULL;
	for(Obj *p=list;p!=Nil;p=p->cdr){
		Obj * o=eval(env,p->car);
		if(head==NULL){
			head=tail=cons(o,Nil);
		}else{
			tail->cdr=cons(o,Nil);
			tail=tail->cdr;
		}
		
	}
	return head;
}

Obj *eval(Obj *env, Obj *obj)
{

	switch (obj->type)
	{
	case T_INT:
	case T_NIL:
	case T_PAREN:
		return obj;
	case T_SYMBOL:{
		Obj *bind=find(env,obj);
		if(bind==Nil){
			printf("symbol %s is not defined",obj->str);
			exit(1);
		}
		return bind->cdr;
		return obj;
	}

		
	case T_CELL:{
		Obj *fn =eval(env,obj->car);
		Obj *args=obj->cdr;
		return apply(env,fn,args);
	}
	default:
		return NULL;
	}
}

void print(Obj *o)
{

	switch (o->type)
	{
	case T_FUNCTION:
		printf("<function>");
		return;
	case T_INT:
		printf("%d", o->value);
		return;
	case T_SYMBOL:
		printf("%s",o->str);
		break;
	case T_TRUE:
		printf("#t");
		break;
	case T_CELL:
		printf("(");
		while(1){
			if(o->type==T_CELL){
				print(o->car);
			}else{
				print(o);
			}
			if(o->type!=T_CELL||o->cdr==Nil){
				  break;
			}
			if(o->cdr->type!=T_CELL){
				printf(" . ");
			}else {
				printf(" ");
			}
			o=o->cdr;
			
		}
		printf(")");
		break;
	}
	
}
int peek(FILE *in){
	int c=getc(in);
	ungetc(c,in);
	return c;
}

Obj *read_quote(FILE *in){
	Obj *sym=intern("quote");
	Obj *o =read(in);
	return cons(sym,cons(o,Nil));

}

Obj * read_sym(FILE *in){
	char buf[1024];
	int c=getc(in);
	int i=0;
	while(isalpha(c)||c=='+'){
		buf[i++]=c;
		c=getc(in);
	}
	ungetc(c,in);
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

Obj *read_list(FILE *in){
	int c=getc(in);
	if(c==')')
		return Nil;
	ungetc(c,in);

	Obj *head,*tail;
	Obj *o=read(in);
	head=tail=cons(o,Nil);
	while(1){
		o=read(in);
		if(o==CParen){
			return head;
		}
		if(o==Dot){
			o=read(in);
			tail->cdr=o;
			if(read(in)!=CParen){
				printf("parenthfesis not closed\n");
			}
			return head;
		}
		tail->cdr=cons(o,Nil);
		tail=tail->cdr;
	}
}

Obj *read(FILE *in)
{
	int c;
	while (1)
	{
		c = getc(in);
		if (isspace(c))
			continue;
		if (c ==EOF)
			return NULL;
		if(c=='\''){
			return read_quote(in);
		}
		if(c=='.'){
			return Dot;
		}
		if(c==')'){
			return CParen;
		}
		if(isdigit(c)||c=='-'){
			ungetc(c,in);
			Obj *num=make_int(read_num(in));
			return num;
		}
		if(isalpha(c)||c=='+'){
			ungetc(c,in);
			Obj * sym=read_sym(in);
			return sym;
		}
		if(c=='('){
			Obj *o =read_list(in);
			return o;
		}
	}
}

void define_constant(Obj *env){
	Obj *sym=intern("t");
	add_var(env,sym,True);
}

void define_prim_function(Obj *env){
	add_prim(env,"quote",prim_quote);
	add_prim(env,"+",prim_plus);
	add_prim(env,"list",prim_list);
	add_prim(env,"setq",prim_setq);
	add_prim(env,"define",prim_define);
	add_prim(env,"lambda",prim_lambda);
}
int main()
{
	//给一些单例的常量赋值
	Nil = make_special(T_NIL);
	Dot = make_special(T_DOT);
	Par = make_special(T_PAREN);
	True = make_special(T_TRUE);
	CParen=make_special(T_PAREN);
	//符号池
	symbols=Nil;
	//环境
	env = make_env(Nil, NULL);		
	//内置的value
	define_constant(env);
	//内置的函数
	define_prim_function(env);

	printf("******Welcom to Lisp World******\n\n");
	while(1){
		printf(">");
		Obj *exp=read(stdin);
		print(eval(env,exp));
		printf("\n");
	}

	return 0;
}
