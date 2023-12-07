// R. Jesse Chaney and Chase Verbout

#include "vikalloc.h"

#define BLOCK_SIZE (sizeof(mem_block_t))
#define BLOCK_DATA(__curr) (((void *) __curr) + (BLOCK_SIZE))
#define DATA_BLOCK(__data) ((mem_block_t *) (__data - BLOCK_SIZE))

#define IS_FREE(__curr) ((__curr -> size) == 0)

#define PTR "0x%07lx"
#define PTR_T PTR "\t"

static mem_block_t *block_list_head = NULL;
static mem_block_t *block_list_tail = NULL;
static void *low_water_mark = NULL;
static void *high_water_mark = NULL;
// only used in next-fit algorithm
static mem_block_t *prev_fit = NULL;

static uint8_t isVerbose = FALSE;
static vikalloc_fit_algorithm_t fit_algorithm = FIRST_FIT;
static FILE *vikalloc_log_stream = NULL;

static void init_streams(void) __attribute__((constructor));

static size_t min_sbrk_size = MIN_SBRK_SIZE;

static void 
init_streams(void)
{
    vikalloc_log_stream = stderr;
}

size_t
vikalloc_set_min(size_t size)
{
    if (0 == size) {
        // just return the current value
        return min_sbrk_size;
    }
    if (size < (BLOCK_SIZE + BLOCK_SIZE)) {
        // In the event that it is set to something silly small.
        size = MAX(BLOCK_SIZE + BLOCK_SIZE, SILLY_SBRK_SIZE);
    }
    min_sbrk_size = size;

    return min_sbrk_size;
}

void 
vikalloc_set_algorithm(vikalloc_fit_algorithm_t algorithm)
{
    fit_algorithm = algorithm;
    if (isVerbose) {
        switch (algorithm) {
        case FIRST_FIT:
            fprintf(vikalloc_log_stream, "** First fit selected\n");
            break;
        case BEST_FIT:
            fprintf(vikalloc_log_stream, "** Best fit selected\n");
            break;
        case WORST_FIT:
            fprintf(vikalloc_log_stream, "** Worst fit selected\n");
            break;
        case NEXT_FIT:
            fprintf(vikalloc_log_stream, "** Next fit selected\n");
            break;
        default:
            fprintf(vikalloc_log_stream, "** Algorithm not recognized %d\n"
                    , algorithm);
            fit_algorithm = FIRST_FIT;
            break;
        }
    }
}

void
vikalloc_set_verbose(uint8_t verbosity)
{
    isVerbose = verbosity;
    if (isVerbose) {
        fprintf(vikalloc_log_stream, "Verbose enabled\n");
    }
}

void
vikalloc_set_log(FILE *stream)
{
    vikalloc_log_stream = stream;
}

void *
vikalloc(size_t size)
{
	// Calc_size will be used to figure out which multiple of min_sbrk_size will be used
	int calc_size = 0;
	// Curr will be used to traverse the list
	mem_block_t *curr = NULL;

	// If size is null or equal to 0 return null
	if (!size || size == 0)
		return NULL;

	// Calculate neccessary multiple of min_brk_size for requested size
	do
	{
			calc_size += min_sbrk_size;
	} while((int)size + (int)BLOCK_SIZE - calc_size > 0);

	// Set curr to head
	curr = block_list_head;	

	// If no head yet- make one. Tail becomes head. low_water_mark becomes pointer to head data. high_water_mark is the end of head.
	if (!block_list_head)
	{
		block_list_head = sbrk(calc_size);
		curr = block_list_head;
		curr->next = curr->prev = NULL;
		curr->size = size;
		curr->capacity = calc_size - BLOCK_SIZE; 
		block_list_tail = low_water_mark = curr;
		high_water_mark = low_water_mark + calc_size;
	}
	else // There is a head- so traverse and find the correct home.
	{
		do
		{
			// If curr is free and the capacity fits the requested size fill it and be done.
			if (IS_FREE(curr) && curr->capacity >= size)
			{
				curr->size = size;
				
    			return BLOCK_DATA(curr);
			} // If curr has the capactiy to fit size and a block then Split the node and wrangle the pointers
			else if (curr->capacity - curr->size >= size + BLOCK_SIZE)
			{
				mem_block_t * test = BLOCK_DATA(curr) + curr->size;
				test->capacity = curr->capacity - BLOCK_SIZE - curr->size;
				test->size = size;
				test->next = curr->next;
				test->prev = curr;
				// If curr has a next then it isnt the tail and we need to set the next's prev to curr
				if (curr->next)
					curr->next->prev = test;
				else 
					block_list_tail = test;
				curr->next = test;
				curr->capacity = curr->size;
				return BLOCK_DATA(test);	
			} 
		
		} while (curr->next && (curr = curr->next));
		// If the loop is exited we know that none of the spaces in previous nodes could fit the size. Make a new block.
		curr->next = sbrk(calc_size);
		curr = curr->next;
		curr->next = NULL;
		curr->prev = block_list_tail;
		block_list_tail = curr;
		curr->size = size;
		curr->capacity = calc_size - BLOCK_SIZE;
		high_water_mark += calc_size;
	}

    if (isVerbose) {
        fprintf(vikalloc_log_stream, ">> %d: %s entry: size = %lu\n"
                , __LINE__, __FUNCTION__, size);
    }

    return BLOCK_DATA(curr);
}

void 
vikfree(void *ptr)
{	
	// curr is going to point to the beginning of the given ptr mem block
	mem_block_t * curr = ptr - BLOCK_SIZE;	
	// If its null or free already- print a snarky message and return.
	if (!ptr || IS_FREE(curr))
	{
    	if (isVerbose) {
        	fprintf(vikalloc_log_stream, "Block is already free: ptr = " PTR "\n", (long) (ptr - low_water_mark));
    	}
    	return;
	}
	// If it passes above we know its not free yet so set size to 0	
	curr->size = 0;

	// Check if curr has a next pointer and if that next block is free as well- if so coalesce
	if (curr->next && IS_FREE(curr->next))
	{
		mem_block_t * temp = curr->next;
		curr->capacity += ((int)BLOCK_SIZE + temp->capacity);
		if (temp->next)
			temp->next->prev = curr;
		else 
			block_list_tail = curr;

		curr->next = temp->next;
		temp = NULL;
	}
	
	// Check if curr has a prev pointer and if that prev block is free as well- if so coalesce
	if (curr->prev && IS_FREE(curr->prev))
	{
		mem_block_t * temp = curr;
		curr = curr->prev;
		curr->capacity += ((int)BLOCK_SIZE + temp->capacity);
		if (temp->next)
			temp->next->prev = curr;
		else 
			block_list_tail = curr;

		curr->next = temp->next;
		temp = NULL;
	}
	/*
    if (isVerbose) {
        fprintf(vikalloc_log_stream, ">> %d: %s entry\n"
                , __LINE__, __FUNCTION__);
    }
*/
    return;
}

///////////////

void 
vikalloc_reset(void)
{
	// Reset to low_water_mark
	brk(low_water_mark);
	// Set pointers to NULL
	block_list_head = block_list_tail = NULL;
	high_water_mark = low_water_mark;	

    if (isVerbose) {
        fprintf(vikalloc_log_stream, ">> %d: %s entry\n"
                , __LINE__, __FUNCTION__);
    }

    if (low_water_mark != NULL) {
        if (isVerbose) {
            fprintf(vikalloc_log_stream, "*** Resetting all vikalloc space ***\n");
        }

    }
}

void *
vikcalloc(size_t nmemb, size_t size)
{
    int *ptr = NULL;
	// If the given size doesn't allocate return Null
	if (!(ptr = vikalloc(size * nmemb)))
		return NULL;
	// For the given nmemb size we want to set each entry to 0
	for (int i = 0; i < (int)nmemb; ++i)
		ptr[i] = 0;
    
    if (isVerbose) {
        fprintf(vikalloc_log_stream, ">> %d: %s entry\n"
                , __LINE__, __FUNCTION__);
    }

    return ptr;
}

void *
vikrealloc(void *ptr, size_t size)
{
	// temp is a ptr to the beginning of the ptr mem block 
	mem_block_t * temp = ptr - BLOCK_SIZE;
	// If ptr is null allocate a new block and return that
	if (!ptr)
		return vikalloc(size);
	// if ptr is a block and size is 0 then free ptr
	if (ptr && size == 0)
	{
		vikfree(ptr);
		return NULL;	
	}
	// if size is greater than the capacity in the given block then create a new block and copy the contents to it.
	if (size > temp->capacity)
	{
		int nmemb = (int)temp->size;
		char * curr = vikalloc(size);	
		char * temp2 = BLOCK_DATA(temp);
		
		for (int i = 0; i < nmemb; ++i)
			curr[i] = temp2[i];

		vikfree(BLOCK_DATA(temp));
		return curr;
	}
	else // if not then temp->size is set to given size because it fits
	{
		temp->size = size;
	}
	
    if (isVerbose) {
        fprintf(vikalloc_log_stream, ">> %d: %s entry\n"
                , __LINE__, __FUNCTION__);
    }

    return ptr;
}

void *
vikstrdup(const char *s)
{
	// length of given arr
	int len = strlen(s);
	// allocate memory for len + 1 for 0/
	mem_block_t * curr = vikalloc(len + 1);

	// copy each char in s into the allocated block	
	for (int i = 0; i < len; ++i)
		((char*)curr)[i] = s[i];
		
    if (isVerbose) {
        fprintf(vikalloc_log_stream, ">> %d: %s entry\n"
                , __LINE__, __FUNCTION__);
    }

    return curr;
}

#include "vikalloc_dump.c"
