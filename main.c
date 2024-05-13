#include <stdio.h>
#include <unistd.h>
#define COG_IMPLEMENTATION
#include "utils.h"
enable_hash_type(String, int);

void test1(){
    const int mx =1000000;
    const int lx =1000000;
    long tm = get_time_microseconds();
    srand(time(0));
    String arr strs = make(String, 32);
    for(int i = 0; i<mx; i++){
        String s = RandomString(32,33);
        assert(s>(int*)64);
        append(strs, s);
    }
    StringintHashTable* table = StringintHashTable_create(lx, HashString, StringEquals);
    printf("%zu\n", len(strs));
    for(int i =0; i<len(strs); i++){
        assert(strs[i]>(int*)64);
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
    for(int i =0; i<len(strs); i++){
        destroy(strs[i]);
    }
    destroy(strs);
    printf("freeing arena took %f seconds\n",(double)(get_time_microseconds()-tm)/1000000);
}
/*

void test1(){
    Arena * local = init_arena();
    const int mx =1000000;
    const int lx =1000000;
    long tm = get_time_microseconds();
    srand(time(0));
    String arr strs = make(String, 32,local);
    for(int i = 0; i<mx; i++){
        String s = RandomString(local,32,33);
        assert(s>(int*)64);
        append(strs, s);
    }
    StringintHashTable* table = StringintHashTable_create(local,lx, HashString, StringEquals);
    printf("%zu\n", len(strs));
    for(int i =0; i<len(strs); i++){
        assert(strs[i]>(int*)64);
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
    free_arena(local);
    printf("freeing arena took %f seconds\n",(double)(get_time_microseconds()-tm)/1000000);
}
*/
int main(){
    test1();
   // test2();
   // test3();
   // test4();
   // test5();
   // test6();
    debug_alloc_and_free_counts();
}