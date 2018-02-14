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
#define worst_fit_label "--worst-fit"
#define best_fit_label "--best-fit"
#define first_fit_label "--first-fit"

index_S prim[500]; //index array
avail_S available_list[500]; //available list
int prim_index_end = 0;
int available_list_end = 0;
int available_list_size = -1;
char *which_order;
char *file_name;
FILE *fp;


int comparator_index(const void *a, const void *b);
int comparator_ascending(const void *a, const void *b);
int comparator_descending(const void *a, const void *b);
int binary_search_key(struct index_S *indexes[], int key, int low, int high);
int worst_fit(struct avail_S *available_list, int *available_list_size, int record_size);
int best_fit(struct avail_S *available_list, int *available_list_size, int record_size);
int first_fit(struct avail_S *available_list, int *available_list_size, int record_size);
void delete_record(int key);
void find_record(int key);
void add_key(int,char*);

int main(int argc,char* argv[]) {


    int available_list_size= -1;
    int index_list_size= -1;

    if (argc != 3) {
		printf("Illegal Number of Arguments");
		exit(0);
	}



}



int binary_search_key(int key, int low, int high) {
	int mid = (low + high)/2;
	if(low <= high) {
		if(prim[mid]->key == key)
			return mid;
		else if(prim[mid] > key)
			return binary_search_key(key, low, mid - 1);
		else
			return binary_search_key(key, mid + 1, high);
	}
	return -1;
}


void find_record(int key)
{
	long offset = binary_search_offset(key, 0, prim_index_end);
	if(offset == -1){
		printf("No record with SID=%d exists\n", key);
	}
	else
	{
		char *output;
		int record_size;
		//file name
		fp = fopen(file_name, "r+b" );
		fseek(fp, offset, SEEK_SET);
		fread(&record_size, sizeof(int), 1, fp);
		output = malloc(record_size + 1);
		fread(output, 1, record_size, fp);
		printf("%s\n", output);
		fclose(fp);
	}
}


//returns offset
long binary_search_offset(int key, int low, int high) {
	int mid = (low + high)/2;
	if(low <= high) {
		if(prim[mid]->key == key)
			return prim[mid]->off;
		else if(prim[mid] > key)
			return binary_search_key(prim, key, low, mid - 1);
		else
			return binary_search_key(prim, key, mid + 1, high);
	}
	return -1;
}


void delete_record(int key) {
	long offset = binary_search_offset(key, 0, prim_index_end);

	if (offset == -1) {
		printf("No record with SID=%d exists\n", key);
	} else {
		int record_size;
		fp = fopen(file_name, "r+b");
		fseek(fp, offset, SEEK_SET);
		fread(&record_size, 1, sizeof(int), fp);
		available_list[available_list_end]->off = offset;
		available_list[available_list_end]->size = record_size + sizeof(int);
		available_list_end = available_list_end + 1;

		if (which_order == worst_fit_label) {
			qsort(available_list, available_list_end, sizeof(avail_S), comparator_descending());
		}
		else if (which_order == best_fit_label) {
			qsort(available_list, available_list_end, sizeof(avail_S), comparator_ascending());
		}

		int j;
		int key_index = binary_search_key(key, 0, prim_index_end);
		for (j = key_index; j < prim_index_end; j++) {
			prim[j]->key = prim[j + 1]->key;
			prim[j]->off = prim[j + 1]->off;
		}

		prim_index_end = prim_index_end - 1;
		fclose(fp);
	}
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


int first_fit(struct avail_S *available_list, int *available_list_size, int record_size) {

}


int comparator_index(const void *a, const void *b)
{
	index_S *index_a=(index_S *) a;
	index_S *index_b=(index_S *) b;
	return index_a->key - index_b->key;

}


int comparator_ascending(const void *a, const void *b)
{
	avail_S *available_a=(avail_S *) a;
	avail_S *available_b=(avail_S *) b;

	int size = available_a->size - available_b->size;
    if(size != 0) {
    	return size;
    } else {
    	return (int)(available_a->off - available_b->off);
    }
}


int comparator_descending(const void *a, const void *b)
{
	avail_S *available_a=(avail_S *) a;
	avail_S *available_b=(avail_S *) b;
	int size = available_b->size - available_a->size;
	if(size != 0) {
		return size;
	} else {
		return (int)(available_b->off - available_a->off);
	}
}


void end_program()
{
	//write indexes to disk
	FILE *output_index = fopen("index.bin", "w+b");
	fwrite(prim, sizeof(index_S), prim_index_end, output_index);
	fclose(output_index);

	//write availability list to disk
	FILE *output_availability;
	output_availability = fopen("availability.bin", "w+b");
	fwrite(available_list, sizeof(avail_S), available_list_end, output_availability);
	fclose(output_availability);


	int i,j,hole_siz=0;

	printf("Index:\n");

	for(i = 0; i < prim_index_end; i++){
		printf( "key=%d: offset=%ld\n", primary[i].key, primary[i].off );
	}

	printf("Availability:\n");

	for(i=0;i<available_end;i++){
		printf( "size=%d: offset=%ld\n", available[i].siz, available[i].off );
		hole_siz+=available[i].siz;
	}

	printf( "Number of holes: %d\n", available_end);
	printf( "Hole space: %d\n", hole_siz );
}


void add_key(int key, char *record) {
	int index_already_exist = binary_search_key(key, 0, prim_index_end);
	if(index_already_exist != -1) {
		printf("Record with SID=%d exists\n",key);
		return;
	}

	else {
		if ((fp = fopen(file_name, "r+b")) == NULL) {
			fp = fopen(file_name, "w+b");
		} else {
			fp = fopen(file_name, "r+b");
		}


		int existing_offset;
		int record_size = strlen(record) - 1;
		if(which_order == best_fit_label) {
			existing_offset = best_fit(available_list, available_list_size, record_size);
		}
		else if(which_order == worst_fit_label) {
			existing_offset = worst_fit(available_list, available_list_size, record_size);
		}
		else if(which_order == first_fit_label) {
			existing_offset = first_fit(available_list, available_list_size, record_size);
		}

		int new_offset;
		if (existing_offset != -1) {
			fseek(fp, existing_offset, SEEK_SET);
			new_offset = existing_offset;
		}
		else {
			fseek(fp, 0, SEEK_END);
			new_offset = ftell(fp);
		}

		fwrite(&record_size, 1, sizeof(int), fp);
		fwrite(record, record_size, 1, fp);
		fclose(fp);

		prim[prim_index_end]->key = key;
		prim[prim_index_end]->off = new_offset;
		prim_index_end = prim_index_end + 1;
		qsort(prim, prim_index_end, sizeof(index_S), comparator_index());
	}

}
