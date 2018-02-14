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
#include <string.h>

typedef struct avail_S {
	int size; /* Hole's size */
	long off; /* Hole's offset in file */
} avail_S;


typedef struct {
	int key; /* Record's key */
	long off; /* Record's offset in file */
} index_S;

#define max_key_offset_pairs 10000 //Availability list will never need to store more than 10,000 size–offset pairs
#define max_input_size 512
#define worst_fit_label "--worst-fit"
#define best_fit_label "--best-fit"
#define first_fit_label "--first-fit"
#define add_label "add"
#define find_label "find"
#define delete_label "del"
#define end_program_label "end"

index_S prim[max_key_offset_pairs]; //index array
avail_S available_list[max_key_offset_pairs]; //available list
int prim_index_end = 0;
int available_list_end = 0;
int available_list_size = -1;
char *which_order;
char *file_name;
FILE *fp;
FILE *output_index_file;
FILE *output_availability_file;

int comparator_index(const void *a, const void *b);
int comparator_ascending(const void *a, const void *b);
int comparator_descending(const void *a, const void *b);
int binary_search_key(int key, int low, int high);
long binary_search_offset(int key, int low, int high);
//int worst_fit(struct avail_S *available_list, int *available_list_size, int record_size);
//int best_fit(struct avail_S *available_list, int *available_list_size, int record_size);
//int first_fit(struct avail_S *available_list, int *available_list_size, int record_size);
long available_offset_for_any_order(int record_size);
void delete_record(int key);
void find_record(int key);
void add_record(int,char*);
void end_program();

int main(int argc,char* argv[]) {
    int available_list_size= -1;
    int index_list_size= -1;

    if (argc != 3) {
		printf("Illegal Number of Arguments");
		exit(0);
	}
    if (strcmp(argv[1], "--worst-fit") == 0) {
		which_order = worst_fit_label;
	}
    else if(strcmp(argv[1], "--best-fit") == 0) {
		which_order = best_fit_label;
	}
    else if(strcmp(argv[1], "--first-fit") == 0) {
		which_order = first_fit_label;
	}
    else {
		printf("Illegal Order");
		exit(0);
	}

    //student file file_name
    file_name = argv[2];

	if ((fp = fopen(file_name, "r+b")) != NULL) {
		int index_end;
		int availability_end;
		output_availability_file = fopen("availability.bin", "rb");
		if (output_availability_file) {
			fseek(output_availability_file, 0, SEEK_END);
			availability_end = ftell(output_availability_file);
			fseek(output_availability_file, 0, SEEK_SET);
			int i = 0;
			while (ftell(output_availability_file) < availability_end) {
				fread(&available_list[i], sizeof(struct avail_S), 1, output_availability_file);
				i++;
				available_list_size = available_list_size + 1;
			}
			fclose(output_availability_file);
		}
		output_index_file = fopen("index.bin", "rb");
		if (output_index_file) {
			fseek(output_index_file, 0, SEEK_END);
			index_end = ftell(output_index_file);
			fseek(output_index_file, 0, SEEK_SET);
			//for(k=0;k<=index_list_size;k++)
			int i = 0;
			while (ftell(output_index_file) < index_end) {
				fread(&prim[i], sizeof(index_S), 1, output_index_file);
				i++;
				index_list_size = index_list_size + 1;
			}
			fclose(output_index_file);
		}
	}

	char *operation;
	char *data;
	int key;
	char *tmp;
	char task[256];

	do {
		fgets(task, 150, stdin);
		tmp = strtok(task, " \n");
		operation = tmp;

		if (strcmp(operation, add_label) == 0) {
			tmp = strtok(NULL, " ");
			key = atoi(tmp);
			tmp = strtok(NULL, " ");
			data = tmp;
			add_record(key, data);
		} else if (strcmp(operation, delete_label) == 0) {
			tmp = strtok(NULL, " ");
			key = atoi(tmp);
			delete_record(key);
		} else if (strcmp(operation, find_label) == 0) {
			tmp = strtok(NULL, " ");
			key = atoi(tmp);
			find_record(key);
		} else if (strcmp(operation, end_program_label) == 0) {
			end_program();
			exit(0);
		} else {
			printf("Invalid Command Entered \n");
		}
	} while (1);
}



int binary_search_key(int key, int low, int high) {
	int mid = (low + high)/2;
	if(low <= high) {
		if(prim[mid].key == key)
			return mid;
		else if(prim[mid].key > key)
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
		if(prim[mid].key == key)
			return prim[mid].off;
		else if(prim[mid].key > key)
			return binary_search_offset(key, low, mid - 1);
		else
			return binary_search_offset(key, mid + 1, high);
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
		available_list[available_list_end].off = offset;
		available_list[available_list_end].size = record_size + sizeof(int);
		available_list_end = available_list_end + 1;

		if (which_order == worst_fit_label) {
			qsort(available_list, available_list_end, sizeof(avail_S), comparator_descending);
		}
		else if (which_order == best_fit_label) {
			qsort(available_list, available_list_end, sizeof(avail_S), comparator_ascending);
		}

		int j;
		int key_index = binary_search_key(key, 0, prim_index_end);
		for (j = key_index; j < prim_index_end; j++) {
			prim[j].key = prim[j + 1].key;
			prim[j].off = prim[j + 1].off;
		}

		prim_index_end = prim_index_end - 1;
		fclose(fp);
	}
}


//int worst_fit(struct avail_S *available_list, int *available_list_size, int record_size) {
//	int is_available = 0;
//	int i = 0;
//	for (i = 0; i <= *available_list_size; i++) {
//		if (available_list[i].size >= record_size) {
//			is_available = 1;
//			int hole_size = available_list[i].size - record_size;
//			if (hole_size > 0) {
//				(*available_list_size)++;
//				//Add hole size at the end
//				available_list[*available_list_size].size = hole_size;
//				available_list[*available_list_size].off = available_list[i].off + record_size;
//			}
//			//break from this loop as we found the space
//			break;
//		}
//	}
//
//	if (is_available == 0) {
//		return -1; //space not found
//	} else {
//		return i;
//	}
//}


//int best_fit(struct avail_S *available_list, int *available_list_size, int record_size) {
//	int is_available = 0;
//	int i = 0;
//	for (i = 0; i <= *available_list_size; i++) {
//		if (available_list[i].size >= record_size) {
//			is_available = 1;
//			int hole_size = available_list[i].size - record_size;
//			if (hole_size > 0) { //append it to available list only if there is any space left
//				(*available_list_size)++;
//				//Add hole size at the end
//				available_list[*available_list_size].size = hole_size;
//				available_list[*available_list_size].off = available_list[i].off + record_size;
//			}
//			//break from this loop as we found the space
//			break;
//		}
//	}
//
//	if (is_available == 0) {
//		return -1; //space not found
//	} else {
//		return i;
//	}
//}


//int first_fit(struct avail_S *available_list, int *available_list_size, int record_size) {
//	int is_available = 0;
//	int i = 0;
//	for (i = 0; i <= *available_list_size; i++) {
//		if (available_list[i].size >= record_size) {
//			is_available = 1;
//			int hole_size = available_list[i].size - record_size;
//			if (hole_size > 0) {
//				(*available_list_size)++;
//				//Add hole size at the end
//				available_list[*available_list_size].size = hole_size;
//				available_list[*available_list_size].off = available_list[i].off + record_size;
//			}
//			//break from this loop as we found the space
//			break;
//		}
//	}
//
//	if (is_available == 0) {
//		return -1; //space not found
//	} else {
//		return i;
//	}
//}


//int first_fit(struct avail_S *available_list, int *available_list_size, int record_size) {
//
//}


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

	int i; //print index values
	printf("Index:\n");
	for(i = 0; i < prim_index_end; i++){
		printf( "key=%d: offset=%ld\n", prim[i].key, prim[i].off );
	}

	printf("Availability:\n");
	int hole_size = 0;
	int j; //print availability list
	for(j = 0; j < available_list_end; j++){
		printf( "size=%d: offset=%ld\n", available_list[j].size, available_list[j].off );
		hole_size = hole_size + available_list[j].size;
	}

	printf( "Number of holes: %d\n", available_list_end);
	printf( "Hole space: %d\n", hole_size);
}


long available_offset_for_any_order(int record_size) {
	int i, j;
	for (i = 0; i < available_list_end; i++) {
		if (available_list[i].size >= record_size) {
			int size_difference = available_list[i].size - record_size - sizeof(int);
			long offset_difference = available_list[i].off + record_size + sizeof(int);
			long offset_location = available_list[i].off;

			if ((which_order == best_fit_label || which_order == worst_fit_label) && size_difference > 0) {
				available_list[i].size = size_difference;
				available_list[i].off = offset_difference;
				if (which_order == best_fit_label) {
					qsort(available_list, i + 1, sizeof(avail_S), comparator_ascending);
					return offset_location;
				} else if (which_order == worst_fit_label) {
					qsort(available_list, available_list_end, sizeof(avail_S), comparator_descending);
					return offset_location;
				}
			}

			else if (which_order == first_fit_label) {
				for (j = i; j < available_list_end; j++) {
					available_list[j].size = available_list[j + 1].size;
					available_list[j].off = available_list[j + 1].off;
				}

				if (size_difference > 0) {
					available_list[available_list_end - 1].size = size_difference;
					available_list[available_list_end - 1].off = offset_difference;
					return offset_location;
				} else {
					available_list_end = available_list_end - 1;
					return offset_location;
				}
			}

			if (size_difference == 0) {
				for (j = i; j < available_list_end; j++) {
					available_list[j].size = available_list[j + 1].size;
					available_list[j].off = available_list[j + 1].off;
				}
				available_list_end -= 1;
				return offset_location;
			}
		}
	}
	return -1;
}


void add_record(int key, char *record) {
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

		int record_size = strlen(record) - 1;
		long existing_offset = available_offset_for_any_order(record_size);
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

		prim[prim_index_end].key = key;
		prim[prim_index_end].off = new_offset;
		prim_index_end = prim_index_end + 1;
		qsort(prim, prim_index_end, sizeof(index_S), comparator_index);
	}

}
