/*
 ============================================================================
 Name        : In-Memory.c
 Author      : Arjun Sharma
 Version     :
 Copyright   : Your copyright notice
 Description : In-Memory Indexing with Availability Lists
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

typedef struct avail_S {
	int size; /* Hole's size */
	long off; /* Hole's offset in file */
} avail_S;


typedef struct {
	int key; /* Record's key */
	long off; /* Record's offset in file */
} index_S;

#define max_key_offset_pairs 10000 //Availability list will never need to store more than 10,000 size–offset pairs

int worst_fit(struct avail_S *available_list, int available_list_size, int record_size) {

	int i = 0;
	int is_available = 0;

	for (i = 0; i <= *available_list_size; i++) {
		if (available_list[i].size >= record_size) {
			is_available = 1;
			int hole_size = available_list[i].size - record_size;
			if (hole_size > 0) {
				(*available_list_size)++;
				//Add hole size at the end
				available_list[*available_list_size].size = hole_size;
				available_list[*available_list_size].off = available_list[i].off + record_size;

			}
			//break from this loop as we found the space
			break;
		}
	}

	if (is_available == 1) {
		return i;
	} else {
		return -1;
	}
}

int main(void) {
	puts("!!!Hello World!!!"); /* prints !!!Hello World!!! */
	return EXIT_SUCCESS;
}
