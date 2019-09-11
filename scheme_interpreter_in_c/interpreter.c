#include<stdio.h>
#include<ctype.h>
#include<stdlib.h>
typedef enum{FIXNUM,BOOLEAN} object_type;

typedef struct object {
	object_type type;
	union {
		long fixnum;
		char boolean;
	} data;

} object;

object *false,*true;

void init(){
	false=malloc(sizeof(object));
	false->type=BOOLEAN;
	false->data.boolean=0;
	true=malloc(sizeof(object));
	true->type=BOOLEAN;
	true->data.boolean=1;

}

int peek(FILE *in){
	int c=getc(in);
	ungetc(c,in);
	return c;
}
void eat_whitespace(FILE *in){
	int c;
	while((c=getc(in))!=EOF){
		if(isspace(c))continue;
		ungetc(c,in);
		break;
	}

}

object* make_fixnum(long value){

	object * o=malloc(sizeof(object));
	if(o==NULL){
		return NULL;
	}
	o->type=FIXNUM;
	o->data.fixnum=value;
	return o;
}
object *read(FILE *in){
	eat_whitespace(in);
	int c=getc(in);
	long num=0;
	int sign=1;
	if(c=='#'){
		c=getc(in);
		switch (c) {
			case 't' :return true;
			case 'f' :return false;
			default :return NULL;
		}
	}
	if(isdigit(c)||c=='-'){
		if(c=='-'){
			c=getc(in);
			sign=-1;
		}


		do{
			num=num*10+c-'0';
			c=getc(in);
		}while(isdigit(c));
	}
	ungetc(c,in);
	num*=sign;

	return make_fixnum(num);
}
void print(object *o){
	switch (o->type){
		case FIXNUM:
		printf("%d\n",o->data.fixnum);break;
		case BOOLEAN:
		printf("#%c\n",o->data.boolean==0?'f':'t');break;
		default:break;

	}

}

int main(){

	printf("***********the interpreter start***************\n\n");
	init();
	while(1){
		printf(">");
		object *o =read(stdin);
		print(o);
	}

	return 0;
}