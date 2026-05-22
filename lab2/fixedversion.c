#include <stdlib.h>
#include <stdio.h>

int main(void) {
    int *p = malloc(sizeof(int));

    if (p == NULL) {
        fprintf(stderr, "malloc failed\n");
        return 1;
    }

    *p = 42;

    printf("%d\n", *p);  // use before free

    free(p);
    p = NULL;

    return 0;
}
