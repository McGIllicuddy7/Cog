#include <stdio.h>
#include <unistd.h>
#define COG_IMPLEMENTATION
#include "cog.h"
#include <time.h>
enable_hash_type(String, int);
void test1(){
    const int mx = 1;
    const int lx =30;
    long tm = get_time_microseconds();
    srand(time(0));
    Arena * local = init_arena();
    String arr strs = make_destroyable(String, 32, local);
    for(int i = 0; i<mx; i++){
        String s = RandomString(local, 64,128);
        append(strs, s);
    }
    StringintHashTable* table = StringintHashTable_create(local, lx, HashString, StringEquals);
    printf("generation finished\n");
    for(int i =0; i<len(strs); i++){
        StringintHashTable_insert(table, strs[i], i);
    }
    for(int i =0; i<len(strs); i++){
        int *a = StringintHashTable_find(table, strs[i]);
        if(a == NULL){
            printf("failed to find\n");
            exit(1);
        }
        if(*a != i){
            StringintKeyValuePair * v = StringintHashTable_find_kv(table, strs[i]);
            printf("failed key = %ls value =%d i = %d\n", v->key, v->value, i);
            exit(1);
        }
    }
    printf("success\n");
    printf("took %f seconds\n",(double)(get_time_microseconds()-tm)/1000000);
    tm = get_time_microseconds();
    StringintHashTable_destroy(table);
    destroy(strs);
    free_arena(local);
    printf("freeing arena took %f seconds\n",(double)(get_time_microseconds()-tm)/1000000);
}
void test2(){
    Arena * local = init_arena();
    StringintHashTable *table = StringintHashTable_create(local, 1000, HashString, StringEquals);
    for(int i =0; i<1000; i++){
        StringintHashTable_insert(table, string_format(local, "%d", i), i);
    }
    StringintHashTable_destroy(table);
    free_arena(local);
}
void test3(){
    const int mx = 1;
    const int lx =10;
    long tm = get_time_microseconds();
    srand(time(0));
    Arena * local = init_arena();
    String arr strs = make_destroyable(String, 32, local);
    for(int i = 0; i<mx; i++){
        printf("%d\n",i);
        String s = RandomString(local, 64,128);
        append(strs, s);
    }
    StringintHashTable *table = StringintHashTable_create(local, lx, HashString, StringEquals);
    printf("generation finished\n");
    printf("success\n");
    printf("took %f seconds\n",(double)(get_time_microseconds()-tm)/1000000);
    tm = get_time_microseconds();
   // StringintHashTable_destroy(table);
    destroy(strs);
    free_arena(local);
    printf("freeing arena took %f seconds\n",(double)(get_time_microseconds()-tm)/1000000);
}
void test4(){
    Arena * local = init_arena();
    String arr strs = make(String, 32, local);
    for(int i = 0; i<1000; i++){
        printf("%d\n",i);
        String s = RandomString(local, 64,128);
        append(strs, s);
    }
    free_arena(local);
}
void test5(){
    int arr ints [10] = {};
}
int main(){
    //test1();
    //test2();
    //test3();
    //test4();
    debug_alloc_and_free_counts();
}