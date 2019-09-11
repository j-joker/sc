#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
typedef enum
{
	FIXNUM,
	BOOLEAN,
	CHAR,
	STRING
} object_type;

typedef struct object
{
	object_type type;
	union {
		long fixnum;
		char boolean;
		char character;
		char *str;
	} data;

} object;

object * false, *true;
#define BUFFER_MAX 1024

void init()
{
	false = malloc(sizeof(object));
	false->type = BOOLEAN;
	false->data.boolean = 0;
	true = malloc(sizeof(object));
	true->type = BOOLEAN;
	true->data.boolean = 1;
}

int peek(FILE *in)
{
	int c = getc(in);
	ungetc(c, in);
	return c;
}
int is_delimeter(char x)
{
	return (isspace(x) || x == '(' || x == ')' || x == '"');
}
void eat_whitespace(FILE *in)
{
	int c;
	while ((c = getc(in)) != EOF)
	{
		if (isspace(c))
			continue;
		ungetc(c, in);
		break;
	}
}

object *make_fixnum(long value)
{

	object *o = malloc(sizeof(object));
	if (o == NULL)
	{
		return NULL;
	}
	o->type = FIXNUM;
	o->data.fixnum = value;
	return o;
}
object *make_char(char c)
{
	object *o = malloc(sizeof(object));
	if (o == NULL)
	{
		return NULL;
	}
	o->type = CHAR;
	o->data.character = c;
	return o;
}
void eat_expect_str(FILE *in, char *str)
{
	int c = getc(in);
	while (*str != '\0')
	{
		if (c != *str)
		{
			printf("unexpected character\n");
			break;
		}
		str++;
		c = getc(in);
	}
}
object *make_string(char* str){
	char *dest=malloc(strlen(str)+1);
	strcpy(dest,str);
	object *o=malloc(sizeof(object));
	o->type=STRING;
	o->data.str=dest;
	return o;
}
object *read_char(FILE *in)
{
	int c = getc(in);
	switch (c)
	{
	case 's':
		if (peek(in) == 'p')
		{
			eat_expect_str(in, "pace");
			return make_char(' ');
		}
		break;
	case 'n':
		if (peek(in) == 'e')
		{
			eat_expect_str(in, "ewline");
			return make_char('\n');
		}
	default:
		break;
	}
	return make_char(c);
}
void peek_expect_delim(FILE *in)
{
	if (!is_delimeter(peek(in)))
	{
		printf("expect a delimeter\n");
	}
}
object *read(FILE *in)
{
	eat_whitespace(in);
	char buffer[BUFFER_MAX];
	int c = getc(in);
	long num = 0;
	int sign = 1;
	
	if(c=='"'){
		int i=0;
		while((c=getc(in))!='"'){
			if(c=='\\'&&peek(in)=='n'){
				getc(in);
				c='\n';
			}
			buffer[i++]=c;
		}
		buffer[i]='\0';
		return make_string(buffer);
	}
	
	if (c == '#')
	{
		c = getc(in);
		switch (c)
		{
		case 't':
			return true;
		case 'f':
			return false;
		case '\\':
			return read_char(in);
		default:
			return NULL;
		}
	}
	if (isdigit(c) || c == '-')
	{
		if (c == '-')
		{
			c = getc(in);
			sign = -1;
		}

		do
		{
			num = num * 10 + c - '0';
			c = getc(in);
		} while (isdigit(c));
		ungetc(c, in);
		num *= sign;
		return make_fixnum(num);
	}
	return NULL;
}
void print(object *o)
{
	switch (o->type)
	{
	case FIXNUM:
		printf("%d\n", o->data.fixnum);
		break;
	case BOOLEAN:
		printf("#%c\n", o->data.boolean == 0 ? 'f' : 't');
		break;
	case CHAR:
		printf("#\\");
		switch (o->data.character)
		{
		case '\n':
			printf("newline\n");
			break;
		case ' ':
			printf("space\n");
			break;
		default:
			printf("%c\n", o->data.character);
			break;
		}
		break;
	case STRING:
	printf("%s\n",o->data.str);
	break;
	default:
		break;
	}
}

int main()
{

	printf("***********the interpreter start***************\n\n");
	init();
	while (1)
	{
		printf(">");
		object *o = read(stdin);
		print(o);
	}

	return 0;
}