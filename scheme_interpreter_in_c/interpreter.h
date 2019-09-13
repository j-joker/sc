#ifndef INTERPETER_H
#define INTERPRETER_H
typedef enum
{
	FIXNUM,
	BOOLEAN,
	CHAR,
	STRING,
	EMPTY_LIST,
	PAIR
} object_type;

typedef struct object
{
	object_type type;
	union {
		long fixnum;
		char boolean;
		char character;
		char *str;
		struct
		{
			struct object *car;
			struct object *cdr;
		} pair;
	} data;

} object;

object *read(FILE *);
void print(object *);
#endif
