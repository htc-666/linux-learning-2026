#include <stdio.h>
#include <unistd.h>

int main(void)
{
    printf("No newline\n");
    while(1)
    {
        sleep(1);
    }
    return 0;
}