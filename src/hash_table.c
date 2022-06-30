#include <stdlib.h>
#include <string.h>

#include "hash_table.h"

#define HASH_TABLE_MAX_LOAD_FACTOR 0.6

/**
 * 32 bit FNV-1A hashing algorithm (see http://isthe.com/chongo/tech/comp/fnv/)
 * Parameters:
 *     FNV_prime = 16777619 (standard 32-bit FNV_prime)
 *     offset_basis = 2166136261 (standard 32-bit offset_basis) 
 * */
static uint32_t fnv_hash_1a_32 ( void *key, int byte_count ) {
    uint8_t* octets = key;
    uint32_t hash = 2166136261; /* Initialize hash with offset_basis */

    /* For every octet of data, perform:
            hash := (hash xor octet)
            hash := hash * FNV_prime
    */
    for (int i = 0; i < byte_count; i++ )
        hash = ( hash ^ octets[i] ) * 16777619;

   return hash;
}

inline static uint32_t hash_key ( uint32_t key )
{
    return fnv_hash_1a_32(&key, 4);
}

inline static uint32_t probe ( int i )
{
    if (i == 0)
        return 0;

    /* Quardatic Probing: (i^2 + i) / 2 */
    return (i * (i + 1)) >> 1;
}

/**
 * Finds a record corresponding to the key `key`. Returns an empty slot (tombstone or truly empty) if the key 
 * was not found. 
 * */
static ht_record_t* ht_find_record ( ht_record_t* table_records, uint32_t capacity, uint32_t key )
{
    uint32_t home = hash_key(key) % capacity;
    uint32_t slot;

    ht_record_t* record;
    ht_record_t* last_tombstone = NULL;

    int i = 0;
    while (true)
    {
        slot = (home + probe(i++)) % capacity;
        record = &table_records[slot];

        if (record->status == HT_RECORD_STATUS_EMPTY)
        {
            // Write to tombstone (if any), or this record
            ht_record_t* target = last_tombstone != NULL ? last_tombstone : record;
            return target;
        }
        else if (record->status == HT_RECORD_STATUS_TOMBSTONE)
        {
            // note it for later write
            if (last_tombstone == NULL)
                last_tombstone = record;
        }
        else if (record->key == key) // record->status == HT_RECORD_STATUS_TOMBSTONE
        {
            return record;
        }
    }
}

/* Keep the table's size a power of 2 (for best results using quadratic probing) */
#define GROW_SIZE(x) \
    ((x) < 8 ? 8 : 2 * (x))

/**
 * Resizes the table's capacity to the next bigger size and copies the old records over
 * while also normalizing the hash table.
 * */
static void ht_grow ( hash_table_t* table )
{
    int old_capacity = table->capacity;
    int new_capacity = GROW_SIZE(old_capacity);
    ht_record_t* allocated = malloc(sizeof(ht_record_t) * new_capacity);

    for (int i = 0; i < new_capacity; i++)
    {
        ht_record_t* record = &allocated[i];
        record->key = 0;
        record->value = 0;
        record->status = HT_RECORD_STATUS_EMPTY;
    }

    table->count_filled = 0;
    for (int i = 0; i < old_capacity; i++)
    {
        ht_record_t* record = &table->records[i];
        if (record->status != HT_RECORD_STATUS_FILLED)
            continue;

        ht_record_t* dest = ht_find_record(allocated, new_capacity, record->key);

        memcpy(dest, record, sizeof(ht_record_t));
        // ...
        // or this below
        // ...
        // dest->key = record->key;
        // dest->value = record->value;
        // dest->status = HT_RECORD_STATUS_FILLED;

        table->count_filled++;
    }

    if (old_capacity > 0)
        free(table->records);

    table->capacity = new_capacity;
    table->count = table->count_filled;
    table->records = allocated;
}

/* --------------------------------------------- */
/* ------------- Library functions ------------- */
/* --------------------------------------------- */

void ht_init ( hash_table_t* table )
{
    table->count            = 0;
    table->count_filled     = 0;
    table->capacity         = 0;

    table->records = NULL;

    ht_grow(table);
}

void ht_free ( hash_table_t* table )
{
    if (table->capacity > 0 && table->records != NULL)
        free(table->records);
}

ht_insert_result_e ht_insert ( hash_table_t* table, uint32_t key, int value )
{
    if ( table->count + 1 >=
            HASH_TABLE_MAX_LOAD_FACTOR * table->capacity )
        ht_grow(table);

    ht_record_t* target = ht_find_record(table->records, table->capacity, key);
    
    ht_insert_result_e result;

    if (target->status == HT_RECORD_STATUS_TOMBSTONE) {
        result = HT_INSERT_WRITE_TOMB;
        table->count_filled++;
    }
    else if (target->key == key) {
        result = HT_INSERT_WRITE_OVER;
    }
    else {
        result = HT_INSERT_WRITE_NORMAL;
        table->count++;
        table->count_filled++;
    }

    target->key = key;
    target->value = value;
    target->status = HT_RECORD_STATUS_FILLED;

    return result;
}

bool ht_get ( hash_table_t* table, int key, int* value )
{
    if (table->count_filled == 0)
        return false;

    ht_record_t* target = ht_find_record(table->records, table->capacity, key);

    if (target->status != HT_RECORD_STATUS_FILLED)
        return false;

    if (value != NULL)
        *value = target->value;
    
    return true;
}

bool ht_delete ( hash_table_t* table, int key )
{
    if (table->count_filled == 0)
        return false;

    ht_record_t* target = ht_find_record(table->records, table->capacity, key);

    if (target->status != HT_RECORD_STATUS_FILLED)
        return false;

    target->status = HT_RECORD_STATUS_TOMBSTONE;
    table->count_filled--;
    
    return true;
}