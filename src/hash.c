#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "uthash.h"

typedef struct student {
    int id;
    char name[10];
    UT_hash_handle hh;
} STU;

STU *students=NULL;

void add_stu(STU *s){
    HASH_ADD_INT(students, id, s);
}

void del_stu(STU* s){
    HASH_DEL(students, s);
    free(s);
}

STU* find_stu(int key){
    STU *s;
    HASH_FIND_INT(students, &key, s);
    return s;
}

int main(int argc, char**argv) {
    STU* tom;
    tom = (STU*)malloc(sizeof(*tom));
    tom->id = 1;
    strcpy(tom->name, "tom");
    add_stu(tom);
    STU* s;
    s = find_stu(1);
    printf("id %d stu is %s\n", 1, s->name);
	STU* jack;
	jack = (STU*)malloc(sizeof(*jack));
	jack->id = 2;
	strcpy(jack->name, "jack");
	add_stu(jack);
	s = find_stu(2);
    printf("id %d stu is %s\n", 2, s->name);
    printf("there are %d stu \n", HASH_COUNT(students));
    STU *cur, *tmp;
	HASH_ITER(hh, students, cur, tmp){
        printf("del stu %s\n", cur->name);
        del_stu(cur); 
    }
    return 0;
}
