// Compile with:
// gcc -Wall -I./include/ -std=c99 -O3 -o example src/*.c example.c

#include <stdio.h>

#include "hash_table.h"

void debug_hash_table(hash_table_t* table)
{
    printf("Capacity = %d, CountAll = %d, CountFill = %d\n", table->capacity, table->count, table->count_filled);
    for (int i = 0; i < table->capacity; i++)
    {
        ht_record_t* record = &table->records[i];

        printf("HT[%d] = ", i);

        if (record->status == HT_RECORD_STATUS_EMPTY)
            printf("<EMPTY>");
        else if (record->status == HT_RECORD_STATUS_TOMBSTONE)
            printf("<TOMBSTONE>");
        else // record->status == HT_RECORD_STATUS_FILLED
            printf("key=%d, value=%d", record->key, record->value);

        printf("\n");
    }
    printf("-----------\n");
}

int main()
{
    hash_table_t table;
    ht_init(&table);

    int output;

    ht_insert(&table, 101,  25);
    ht_insert(&table, 700, 303);
    ht_insert(&table,  5, 6571);

    if ( ! ht_get(&table, 700, &output) )
    {
        fprintf(stderr, "Failed to get value of %d \n", 700);
        return -1;
    }

    printf("table[700] = %d\n", output); // output should be 303

    /* ------------ */

    ht_get(&table, 101, &output);
    printf("table[101] = %d\n", output); // output should be 25

    /* ------------ */

    printf("table[444] found = %s\n", ht_get(&table, 444, NULL) ? "true" : "false"); // should be false

    /* ------------ */

    ht_delete(&table, 700);
    ht_delete(&table, 5);

    printf("(after deleting) table[700] found = %s\n", ht_get(&table, 700, NULL) ? "true" : "false"); // should be false
    printf("(after deleting) table[5] found = %s\n", ht_get(&table, 5, NULL) ? "true" : "false"); // should be false

    /* ------------ */

    ht_insert(&table,  5, 8330);
    ht_get(&table, 5, &output);
    printf("table[5] = %d\n", output); // output should be 8330

    /* ------------ */

    printf("\n");
    debug_hash_table(&table);

    ht_free(&table);

    return 0;
}
