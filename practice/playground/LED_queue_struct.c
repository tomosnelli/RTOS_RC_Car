#include <stdio.h>

typedef struct LED_Operation
{
    int toggle;
    int duration;
} LED_Operation_t;

static LED_Operation_t pairing[] = {
    { 1, 100 },
    { 0, 100 },
    { 1, 100 },
    { 0, 100 },
    { 1, 100 },
    { 0, 500 }
};

typedef struct LED_Command
{
    LED_Operation_t * command;
    size_t uSize;
} LED_Command_t;

int main()
{
	size_t arraySize = sizeof( pairing )/sizeof( LED_Operation_t );
	LED_Command_t buff = { pairing, arraySize };
	for( size_t i = 0; ; ++i)
	{
		printf( "toggle:   %d\n", (buff.command[ i % arraySize ]).toggle );
		printf( "duration: %d\n", (buff.command[ i % arraySize ]).duration );
	}

	return 0;
}