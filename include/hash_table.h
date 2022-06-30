#include <stdint.h>
#include <stdbool.h>

typedef enum {
    HT_RECORD_STATUS_EMPTY,
    HT_RECORD_STATUS_FILLED,
    HT_RECORD_STATUS_TOMBSTONE,
} ht_record_status_e;

typedef struct {
    uint32_t key;
    int value;
    ht_record_status_e status;
} ht_record_t;

/**
 * A close-hashed hash table the maps (unsignend) integers keys to integer values
 * */
typedef struct {
    uint32_t count; /* Count of all used entries (including those marked as tombstones) */
    uint32_t count_filled; /* Count of only filled entries */
    uint32_t capacity; /* Number of total allocated slots */

    ht_record_t* records;
} hash_table_t;


typedef enum {
    /** 
     * Lowest bit represents if the insertion added a new record in the table (0) or it overwrote an existing record (1)
     * 
     * The next bits ( currently next 2 bits: (result_enum_value & 0x110) >> 1) ) are actual enumeration values
     * for the various insertion result types
     * */
    HT_INSERT_WRITE_NORMAL  = /* binary 0010 */ 2,
    HT_INSERT_WRITE_TOMB    = /* binary 0100 */ 4,
    HT_INSERT_WRITE_OVER    = /* binary 0111 */ 7,
} ht_insert_result_e;


/**
 * Initializes a hash table for usage
 * */
void ht_init ( hash_table_t* table );


/**
 * Destroy the hash table
 * */
void ht_free ( hash_table_t* table );


/**
 * Inserts an a record into the table with key `key` and value `value`
 * 
 * Returns `HT_INSERT_WRITE_NORMAL` if record was inserted normally.
 *
 * Returns `HT_INSERT_WRITE_OVER` if record with the given key already existed and is now overwritten by this new value.

 * Returns `HT_INSERT_WRITE_TOMB` if record was written over a tombstone (internal)
 * */
ht_insert_result_e ht_insert ( hash_table_t* table, uint32_t key, int value );


/**
 * Retrieves the value associated with key `key` and writes it to the address `value`.
 * 
 * Returns true on success, and false otherwise.
 * 
 * Pass `NULL` as `value` to only check the existence of the key using the return value
 * */
bool ht_get ( hash_table_t* table, int key, int* value );


/**
 * Deletes the record corresponding to key `key`.
 * 
 * Returns true if the key was found (hence now deleted), and false otherwise.
 * */
bool ht_delete ( hash_table_t* table, int key );
