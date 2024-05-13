#pragma once
//use #define COG_IMPLEMENTATION
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <assert.h>
#include <wchar.h>
#include <sys/time.h>
/*
	Initial Defines
*/
void * debug_alloc(size_t count, size_t size);
void debug_free(void * ptr);
void debug_alloc_and_free_counts();
#ifndef global_alloc
#define global_alloc(count,sz) debug_alloc(count, sz)
#endif 
#ifndef global_free
#define global_free(ptr) debug_free(ptr)
#endif
#ifndef str_type
#define str_type wchar_t
#endif
#define nil 0
typedef unsigned char Byte;

/*
Memory stuff
*/

void mem_shift(void * start, size_t size, size_t count, size_t distance);
void slice_cpy(void * target, void * source, size_t element_size, size_t count);


/*
Slice stuff
*/

#define arr * 
void * new_array(size_t object_size, size_t capacity);
void delete_array(void * array);
void * array_concat(void * array, void * target, size_t object_size, size_t addr_size);
size_t array_length(void * array);
size_t array_capacity(void * array);
void *array_resize(void * array, size_t new_size, size_t obj_size);
void *array_remove(void * array, size_t idx, size_t obj_size);
void *array_clone(void * array, size_t obj_size);

#define make(T, sz) new_array(sizeof(T), sz)
#define clone(array) array_clone(array, sizeof(*array))
#define destroy(target) delete_array(target)
#define resize(target, new_size) target = array_resize(target,new_size, sizeof(*target))
#define append(target, addr) \
    resize(target, len(target)+1);\
    target[len(target)-1] = addr

#define concat(target, addr)     target =array_concat(target, addr, sizeof(*target), sizeof(*addr))
#define remove(target, idx)  target = array_remove(target, idx, sizeof(*target))
#define insert(target, idx,value)\
target = resize(target, len(target)+1);\
memmove(&target[idx+1], &target[idx], (len(target)-idx-1)*sizeof(*target));\
target[idx] = value
#define len(target) array_length(target)
#define cap(target) array_capacity(target)

/*
String stuff
*/

typedef str_type arr String;
String new_string(const char* str);
String new_string_wide(const wchar_t* str);
void _strconcat(String * a, const char* b, size_t b_size);
String string_format(const char * fmt, ...);
bool StringEquals(String a, String b);
String RandomString(int minlen, int maxlen);
#define str_concat(a, b)\
	_strconcat(&a,(const char *)b, sizeof(b[0]));

#define str_append(a,b)\
	resize(a, len(a)+1);\
	a[len(a)-1] = b

/*
HashFunctions
*/
size_t HashBytes(Byte * byte, size_t size);
size_t HashInt(int in);
size_t HashFloat(float fl);
size_t HashLong(long lg);
size_t HashDouble(double db);
size_t HashString(String str);
/*
Hashtable
*/
//maps Ts to Us
// I love you so much - Anna/toast <3
#define enable_hash_type(T,U)\
typedef struct{\
	T key;\
	U value;\
}T##U##KeyValuePair;\
typedef struct{\
	T##U##KeyValuePair arr * Table;\
	size_t TableSize;\
	size_t (*hash_func)(T);\
	bool (*eq_func)(T,T);\
}T##U##HashTable;\
static T##U##HashTable * T##U##HashTable_create(size_t size,size_t (*hash_func)(T),bool (*eq_func)(T,T)){\
	T##U##HashTable * out = global_alloc(1, sizeof(T##U##HashTable));\
	out->Table= global_alloc(1,sizeof(T##U##KeyValuePair arr)*size);\
	out->TableSize = size;\
	out->hash_func = hash_func;\
	out->eq_func = eq_func;\
	for(int i =0; i<size; i++){\
		out->Table[i] = make(T##U##KeyValuePair,16);\
	}\
	return out;\
}\
static void T##U##HashTable_resize(T##U##HashTable * table, size_t new_size){\
	T##U##KeyValuePair arr * new_table = global_alloc(1,new_size);\
	for(int i =0; i<new_size; i++){\
		new_table[i] = make(T##U##KeyValuePair,8);\
	}\
	for(int i =0; i<table->TableSize; i++){\
		for(int j = 0; j<len(table->Table[i]); j++){\
			size_t hashval = table->hash_func(table->Table[i][j].key);\
			size_t hash = hashval%new_size;\
			T##U##KeyValuePair pair = {.key = table->Table[i][j].key, .value = table->Table[i][j].value};\
			append(new_table[hash], pair);\
		}\
	}\
	T##U##KeyValuePair arr * old = table->Table;\
	size_t old_len = table->TableSize;\
	table->Table = new_table;\
	table->TableSize = new_size;\
	for(int i =0; i<old_len; i++){\
		destroy(old[i]);\
	}\
    global_free(old);\
}\
static U* T##U##HashTable_find(T##U##HashTable* table, T key){\
	size_t hashval = table->hash_func(key);\
	size_t hash = hashval%table->TableSize;\
	for(int i =0 ; i<len(table->Table[hash]); i++){\
		T##U##KeyValuePair p = table->Table[hash][i];\
		if(table->eq_func(p.key, key)){\
			return &table->Table[hash][i].value;\
		}\
	}\
	return nil;\
}\
static T##U##KeyValuePair* T##U##HashTable_find_kv(T##U##HashTable* table, T key){\
	size_t hashval = table->hash_func(key);\
	size_t hash = hashval%table->TableSize;\
	for(int i =0 ; i<len(table->Table[hash]); i++){\
		T##U##KeyValuePair p = table->Table[hash][i];\
		if(table->eq_func(p.key, key)){\
			return &table->Table[hash][i];\
		}\
	}\
	return nil;\
}\
static void T##U##HashTable_insert(T##U##HashTable* table, T key, U value){\
	size_t hashval = table->hash_func(key);\
	size_t hash = hashval%table->TableSize;\
	T##U##KeyValuePair pair = (T##U##KeyValuePair){.key = key,.value = value};\
	T##U##KeyValuePair arr tmp = table->Table[hash];\
	int tl = len(tmp);\
	append(tmp, pair);\
	table->Table[hash] = tmp;\
}\
static void T##U##HashTable_destroy(T##U##HashTable * table){\
	for(int i =0; i<table->TableSize; i++){\
		destroy(table->Table[i]);\
	}\
	global_free(table->Table);\
	global_free(table);\
}
/*
Utils
*/
long get_time_microseconds();
void begin_profile();
long end_profile();
void end_profile_print(const char * message);
/*
Implementation
*/

//#ifdef COG_IMPLEMENTATION 
static int alloc_count = 0;
static int free_count =0;
void * debug_alloc(size_t count, size_t size){
	alloc_count++;
	return calloc(count, size);
}
void debug_free(void * ptr){
	free_count++;
	free(ptr);
}
void debug_alloc_and_free_counts(){
	printf("alloc count: %d, free_count: %d\n", alloc_count, free_count);
}
/*
Memory Stuff
*/

void mem_shift(void * start, size_t size, size_t count, size_t distance){
	char * data = (char *)start;
	for(int j = 0; j<size*distance; j++){
		for (int i = count*size; i>0; i--){
			data[i] = data[i-1];
		}
	}
}
void * mem_clone(void * start, size_t element_size, size_t count){
	char * out = (char *)global_alloc(count, element_size*count);
	for(int i =0; i<element_size*count; i++){
		out[i] = ((char *)(start))[i];
	}
	return (void*)out;
}



/*
Hashing
*/

size_t HashBytes(Byte * bytes, size_t size){
	size_t out = 0;
	const size_t pmlt = 31;
	size_t mlt = 31;
	for(int i =0; i<size;i++){
		out += bytes[i]*mlt;
		mlt*=pmlt;
	}
	return out;
}
size_t HashInt(int in){
	int tmp = in;
	return HashBytes((Byte *)&tmp, sizeof(tmp));
}
size_t HashFloat(float fl){
	float tmp = fl;
	return HashBytes((Byte *)&tmp, sizeof(tmp));
}
size_t HashLong(long lg){
	long tmp = lg;
	return HashBytes((Byte *)&tmp, sizeof(tmp));
}
size_t HashDouble(double db){
	double tmp = db;
	return HashBytes((Byte *)&tmp, sizeof(tmp));
}
size_t HashString(String str){
	size_t out = 0;
	const size_t pmlt = 31;
	size_t mlt = 1;
	for(int i =0; i<len(str);i++){
		out += str[i]*mlt;
		mlt*=pmlt;
	}
	return out;
}
/*
Utils
*/
long get_time_microseconds(){
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_usec+tv.tv_sec*1000000;
}
static long profile_time = 0;
void begin_profile(){
	if(profile_time == 0){
		profile_time = get_time_microseconds();
	}
}
long end_profile(){
	if(profile_time != 0){
		long out =  get_time_microseconds()-profile_time;
		profile_time = 0;
		return out;
	}
	return -1;
}
void end_profile_print(const char * message){
	printf("%s took %f seconds\n",message, ((double)end_profile())/1000000);
}

typedef struct{
    size_t length;
    size_t capacity;
}ArrayHeader_t;
size_t to_pow_2(size_t sz){
    int i = 1;
    while(i<sz){
        i*=2;
    }
    return i;
}
ArrayHeader_t* GetHeader(void* array){
    return (ArrayHeader_t*)(array-sizeof(ArrayHeader_t));
}
void * new_array(size_t object_size, size_t capacity){
    size_t space = to_pow_2(capacity);
    void * tmp = global_alloc(1,sizeof(ArrayHeader_t)+object_size*space);
    ArrayHeader_t * head = (ArrayHeader_t * )tmp;
    head->length = 0;
    head->capacity = space;
	assert(tmp != NULL);
    void * out = tmp+sizeof(ArrayHeader_t);
    return out;
}
void delete_array(void * array){
    ArrayHeader_t * head = GetHeader(array);
    global_free(head);
}
void * array_concat(void * array, void * target, size_t object_size, size_t addr_size){
    assert(addr_size == object_size);
    assert(array != target);
    ArrayHeader_t * array_head = GetHeader(array);
    int l = array_head->length;
    ArrayHeader_t * target_head = GetHeader(target);
    int v = target_head->length;
    void * out= array_resize(array, array_head->length+v,object_size);
    assert(out != NULL);
	memcpy(out+l*object_size, target, object_size*v);
    return out;
}
size_t array_length(void * array){
    ArrayHeader_t * head = GetHeader(array);
    return head->length;
}
size_t array_capacity(void * array){
    ArrayHeader_t * head = GetHeader(array);
    return head->capacity;
}
void * array_resize(void * array, size_t new_size, size_t obj_size){
    ArrayHeader_t * head = GetHeader(array);
    if(head->capacity>=new_size){
        head->length = new_size;
        return array;
    }
    ArrayHeader_t old = *head;
    size_t sz = to_pow_2(new_size);
    void * out = realloc(head,sz*obj_size+sizeof(ArrayHeader_t));
	assert(out != NULL);
    head =out;
    head->capacity = sz;
    head->length = new_size;
    return out+sizeof(ArrayHeader_t);
}
void * array_remove(void * array, size_t idx, size_t obj_size){
    if(len(array) == 1){
        global_free(array);
        return NULL;
    }
    size_t l = len(array);
    void * out = array_resize(array, len(array)-1, obj_size);
    void * start = out+idx*obj_size;
    void * end = out+(idx+1)*obj_size;
    size_t size = (l-idx-1)*obj_size;
    memmove(start, end, size);
    return out;
}
void *array_clone(void * array, size_t obj_size){
    size_t sz = GetHeader(array)->capacity+sizeof(ArrayHeader_t);
    void * block = global_alloc( sz, obj_size);
	assert(block != nil);
    char * tmp = block;
    char * old = (void *)GetHeader(array);
    for(int i =0; i<sz; i++){
        tmp[i] = old[i];
    }
    return block;
}
/*
String Stuff
*/

String new_string(const char* str){
	int l = strlen(str);
    String out = make(str_type,l);
	for(int i = 0; i<l; i++){
		append(out, (str_type)str[i]);
	}
	append(out, '\0');
	return out;
}
String new_string_wide(const wchar_t* str){
    int l = wcslen(str);
	String out = make(str_type, l);
	for(int i = 0; i<l; i++){
		append(out, (str_type)str[i]);
	}
	append(out, '\0');
	return out;
}
void _strconcat(String * a, const char* b, size_t b_size){
	if(b_size <4){
        int l = len(*a)-1;
		resize((*a), len((*a))+strlen(b));
		for(int i=0; i<strlen(b); i++){
			(*a)[l+i] = (str_type)(b[i]);
		}
	}
	else {
		resize((*a), len((*a))+wcslen((const wchar_t *)b));
		const wchar_t * v = (const wchar_t *)b;
		for(int i=0; i<wcslen(v); i++){
			(*a)[len(a)-1] = (str_type)(v[i]);
		}
	}
}
String string_format(const char * fmt, ...){
	String s =new_string("");
	va_list args;
	va_start(args, fmt);
	int l = strlen(fmt);
	for(int i = 0; i<l; i++){
		if(fmt[i] != '%'){
            {
				str_append(s, fmt[i]);
			}
			append(s, '\0');
		}
		else{
			if(fmt[i+1] == 'c'){
				char buff[2];
				buff[0] = (char)(va_arg(args,int));
				buff[1] = '\0';
				str_concat(s, buff);
				i++;
			}
			if(fmt[i+1] == 'l' && fmt[i+2] == 'u'){
				char buff[128];
				snprintf(buff,127, "%lu", va_arg(args,unsigned long));
				str_concat(s,buff);
				i+= 2;
			}
			if(fmt[i+1] == 'u'){		
				char buff[128];
				snprintf(buff,127, "%u", va_arg(args,unsigned int));
				str_concat(s,buff);
				i+= 1;
			}
			if(fmt[i+1] == 'l' && fmt[i+2] == 'd'){
				char buff[128];
				snprintf(buff,127, "%ld", va_arg(args,long));
				str_concat(s,buff);
				i+= 2;
			}
			if(fmt[i+1] == 'l' && fmt[i+2] == 's'){
				str_concat(s,va_arg(args, wchar_t *));
				i+= 2;
			}
			else if(fmt[i+1] == 's'){
				char * s2 =  va_arg(args,char *);
				str_concat(s, s2);
				i++;
			}
			else if(fmt[i+1] == 'f'){
				char buff[128];
				snprintf(buff,127,"%f", va_arg(args,double));
				str_concat(s,buff);
				i++;
			}
			else if(fmt[i+1] == 'd'){
				char buff[128];
				snprintf(buff,127,"%d", va_arg(args,int));
				str_concat(s,buff);
				i++;
			}
			else if(fmt[i+1] == '%'){
				append(s, '%');
				i++;
			}
		}
	}
	va_end(args);
	return s;
}
bool StringEquals(String a, String b){
	if(a == 0 || b == 0){
		return 0;
	}
	if(len(a) != len(b)){
		return 0;
	}
	for(int i= 0; i<len(a); i++){
		if(a[i] != b[i]){
			return 0;
		}
	}
	return 1;
}
String RandomString(int minlen, int maxlen){
	int length = rand()%(maxlen-minlen)+minlen;
	String out = make(str_type, length+1);
	for(int i= 0; i<length+1; i++){
		out[i] = 0;
	}
    resize(out, length+3);
	for(int i =0; i<length; i++){
		char c = rand()%(90-65)+65;
		if(rand()%2){
			c += 32;
		}
		out[i] = c;
	}
	out[length+1] = 0;
	return out;
}
//#endif
