/*
 ============================================================================
 Name        : assn_2.c
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

int comparator_index(const void *a, const void *b);
int comparator_ascending(const void *a, const void *b);
int comparator_descending(const void *a, const void *b);
int binary_search_key(struct index_S *indexes[], int key, int low, int high);
int worst_fit(struct avail_S *available_list, int *available_list_size, int record_size);
int best_fit(struct avail_S *available_list, int *available_list_size, int record_size);


int main(int argc,char* argv[]) {
	index_S prim[500]; //index array
    avail_S available[500]; //available list
    FILE *fp;
    int available_list_size= -1;
    int index_list_size= -1;

    if (argc != 3) {
		printf("Illegal Number of Arguments");
		exit(0);
	}



}



int binary_search_key(struct index_S *prim[], int key, int low, int high) {
	int mid = (low + high)/2;
	if(low <= high) {
		if(prim[mid] == key)
			return mid;
		else if(prim[mid] > key)
			return binary_search(prim, key, low, mid - 1);
		else
			return binary_search(prim, key, mid + 1, high);
	}
	return -1;
}

int worst_fit(struct avail_S *available_list, int *available_list_size, int record_size) {
	int is_available = 0;
	int i = 0;
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

	if (is_available == 0) {
		return -1; //space not found
	} else {
		return i;
	}
}


int best_fit(struct avail_S *available_list, int *available_list_size, int record_size) {
	int is_available = 0;
	int i = 0;
	for (i = 0; i <= *available_list_size; i++) {
		if (available_list[i].size >= record_size) {
			is_available = 1;
			int hole_size = available_list[i].size - record_size;
			if (hole_size > 0) { //append it to available list only if there is any space left
				(*available_list_size)++;
				//Add hole size at the end
				available_list[*available_list_size].size = hole_size;
				available_list[*available_list_size].off = available_list[i].off + record_size;
			}
			//break from this loop as we found the space
			break;
		}
	}

	if (is_available == 0) {
		return -1; //space not found
	} else {
		return i;
	}
}


int comparator_index(const void *a, const void *b)
{
	index_S *index_a=(index_S *) a;
	index_S *index_b=(index_S *) b;
	return index_a->key - index_b->key;

}


int comparator_ascending(const void *a, const void *b)
{
	avail_S *avail_a=(avail_S *) a;
	avail_S *avail_b=(avail_S *) b;

	int size = avail_a->size - avail_b->size;
    if(size != 0) {
    	return size;
    } else {
    	return (int)(avail_a->off - avail_b->off);
    }
}


int comparator_descending(const void *a, const void *b)
{
	avail_S *avail_a=(avail_S *) a;
	avail_S *avail_b=(avail_S *) b;

	int size = avail_b->size - avail_a->size;
	if(size != 0) {
		return size;
	} else {
		return (int)(avail_b->off - avail_a->off);
	}
}
