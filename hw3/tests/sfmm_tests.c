#include <criterion/criterion.h>
#include <errno.h>
#include <signal.h>
#include "debug.h"
#include "sfmm.h"
#define TEST_TIMEOUT 15

/*
 * Assert the total number of free blocks of a specified size.
 * If size == 0, then assert the total number of all free blocks.
 */
void assert_free_block_count(size_t size, int count) {
    int cnt = 0;
    for(int i = 0; i < NUM_FREE_LISTS; i++) {
	sf_block *bp = sf_free_list_heads[i].body.links.next;
	while(bp != &sf_free_list_heads[i]) {
	    if(size == 0 || size == (bp->header & ~0x1f))
		cnt++;
	    bp = bp->body.links.next;
	}
    }
    if(size == 0) {
	cr_assert_eq(cnt, count, "Wrong number of free blocks (exp=%d, found=%d)",
		     count, cnt);
    } else {
	cr_assert_eq(cnt, count, "Wrong number of free blocks of size %ld (exp=%d, found=%d)",
		     size, count, cnt);
    }
}

/*
 * Assert that the free list with a specified index has the specified number of
 * blocks in it.
 */
void assert_free_list_size(int index, int size) {
    int cnt = 0;
    sf_block *bp = sf_free_list_heads[index].body.links.next;
    while(bp != &sf_free_list_heads[index]) {
	cnt++;
	bp = bp->body.links.next;
    }
    cr_assert_eq(cnt, size, "Free list %d has wrong number of free blocks (exp=%d, found=%d)",
		 index, size, cnt);
}

Test(sfmm_basecode_suite, malloc_an_int, .timeout = TEST_TIMEOUT) {
	sf_errno = 0;
	int *x = sf_malloc(sizeof(int));

	cr_assert_not_null(x, "x is NULL!");

	*x = 4;

	cr_assert(*x == 4, "sf_malloc failed to give proper space for an int!");

	assert_free_block_count(0, 1);
	assert_free_block_count(1952, 1);
	assert_free_list_size(7, 1);

	cr_assert(sf_errno == 0, "sf_errno is not zero!");
	cr_assert(sf_mem_start() + PAGE_SZ == sf_mem_end(), "Allocated more than necessary!");
}

Test(sfmm_basecode_suite, malloc_four_pages, .timeout = TEST_TIMEOUT) {
	sf_errno = 0;

	// We want to allocate up to exactly four pages.
	void *x = sf_malloc(16288);
	cr_assert_not_null(x, "x is NULL!");
	assert_free_block_count(0, 0);
	cr_assert(sf_errno == 0, "sf_errno is not 0!");
}

Test(sfmm_basecode_suite, malloc_too_large, .timeout = TEST_TIMEOUT) {
	sf_errno = 0;
        void *x = sf_malloc(PAGE_SZ * 100);

	cr_assert_null(x, "x is not NULL!");
	assert_free_block_count(0, 1);
	assert_free_block_count(36800, 1);
	cr_assert(sf_errno == ENOMEM, "sf_errno is not ENOMEM!");
}

Test(sfmm_basecode_suite, free_no_coalesce, .timeout = TEST_TIMEOUT) {
	sf_errno = 0;
	/* void *x = */ sf_malloc(8);
	void *y = sf_malloc(200);
	/* void *z = */ sf_malloc(1);

	sf_free(y);

	assert_free_block_count(0, 2);
	assert_free_block_count(224, 1);
	assert_free_block_count(1696, 1);

	cr_assert(sf_errno == 0, "sf_errno is not zero!");
}

Test(sfmm_basecode_suite, free_coalesce, .timeout = TEST_TIMEOUT) {
	sf_errno = 0;
	/* void *w = */ sf_malloc(8);
	void *x = sf_malloc(200);
	void *y = sf_malloc(300);
	/* void *z = */ sf_malloc(4);

	sf_free(y);
	sf_free(x);

	assert_free_block_count(0, 2);
	assert_free_block_count(544, 1);
	assert_free_block_count(1376, 1);

	cr_assert(sf_errno == 0, "sf_errno is not zero!");
}

Test(sfmm_basecode_suite, freelist, .timeout = TEST_TIMEOUT) {
	void *u = sf_malloc(200);
	/* void *v = */ sf_malloc(300);
	void *w = sf_malloc(200);
	/* void *x = */ sf_malloc(500);
	void *y = sf_malloc(200);
	/* void *z = */ sf_malloc(700);

	sf_free(u);
	sf_free(w);
	sf_free(y);

	assert_free_block_count(0, 4);
	assert_free_block_count(224, 3);
	assert_free_block_count(1760, 1);
	assert_free_list_size(4, 3);
	assert_free_list_size(7, 1);

	// First block in list should be the most recently freed block.
	int i = 4;
	sf_block *bp = sf_free_list_heads[i].body.links.next;
	cr_assert_eq(bp, (char *)y - sizeof(sf_header),
		     "Wrong first block in free list %d: (found=%p, exp=%p)",
                     i, bp, (char *)y - sizeof(sf_header));
}

Test(sfmm_basecode_suite, realloc_larger_block, .timeout = TEST_TIMEOUT) {
	void *x = sf_malloc(sizeof(int));
	/* void *y = */ sf_malloc(10);
	x = sf_realloc(x, sizeof(int) * 20);

	cr_assert_not_null(x, "x is NULL!");
	sf_block *bp = (sf_block *)((char *)x - sizeof(sf_header));
	cr_assert(bp->header & THIS_BLOCK_ALLOCATED, "Allocated bit is not set!");
	cr_assert((bp->header & ~0x1f) == 96, "Realloc'ed block size not what was expected!");

	assert_free_block_count(0, 2);
	assert_free_block_count(1824, 1);
}

Test(sfmm_basecode_suite, realloc_smaller_block_splinter, .timeout = TEST_TIMEOUT) {
	void *x = sf_malloc(sizeof(int) * 20);
	void *y = sf_realloc(x, sizeof(int) * 16);

	cr_assert_not_null(y, "y is NULL!");
	cr_assert(x == y, "Payload addresses are different!");

	sf_block *bp = (sf_block *)((char*)y - sizeof(sf_header));
	cr_assert(bp->header & THIS_BLOCK_ALLOCATED, "Allocated bit is not set!");
	cr_assert((bp->header & ~0x1f) == 96, "Block size not what was expected!");

	// There should be only one free block.
	assert_free_block_count(0, 1);
	assert_free_block_count(1888, 1);
}

Test(sfmm_basecode_suite, realloc_smaller_block_free_block, .timeout = TEST_TIMEOUT) {
	void *x = sf_malloc(sizeof(double) * 8);
	void *y = sf_realloc(x, sizeof(int));

	cr_assert_not_null(y, "y is NULL!");

	sf_block *bp = (sf_block *)((char*)y - sizeof(sf_header));
	cr_assert(bp->header & THIS_BLOCK_ALLOCATED, "Allocated bit is not set!");
	cr_assert((bp->header & ~0x1f) == 32, "Realloc'ed block size not what was expected!");

	// After realloc'ing x, we can return a block of size 32 to the freelist.
	// This block will be coalesced.
	assert_free_block_count(0, 1);
	assert_free_block_count(1952, 1);
}

//############################################
//STUDENT UNIT TESTS SHOULD BE WRITTEN BELOW
//DO NOT DELETE THESE COMMENTS
//############################################

//Test(sfmm_student_suite, student_test_1, .timeout = TEST_TIMEOUT) {
//}

//Count the number of free_block sizes equal to size.
//When size is 0, all free blocks are counted.
void free_block_count(size_t size, int count) {
	int count1=0;
	int i=0;
	sf_block *block1;
	while(i<8){
		block1 = (sf_free_list_heads+i)->body.links.next;
        while(block1 != (sf_free_list_heads+i)){
            if(size==0 || (block1->header>>5)*32 == size){
            	count1++;
            }
            block1 = block1->body.links.next;
        }
        i++;
	}

	if(size == 0) {
	cr_assert_eq(count1, count, "Wrong number of free blocks (exp=%d, found=%d)",
		     count, count1);
    } else {
	cr_assert_eq(count1, count, "Wrong number of free blocks of size %ld (exp=%d, found=%d)",
		     size, count, count1);
    }
}


//Count the number of blocks in free_list [index].
void free_list_size(int index, int size) {
	int size1=0;
	sf_block *block1 = (sf_free_list_heads+index)->body.links.next;
    while(block1 != (sf_free_list_heads+index)){
        size1++;
        block1 = block1->body.links.next;
    }

    cr_assert_eq(size1, size, "Free list %d has wrong number of free blocks (exp=%d, found=%d)",
		 index, size, size1);
}


Test(sfmm_student_suite, malloc_allocates_a_full_page_multiple_times, .timeout = TEST_TIMEOUT) {
	sf_errno = 0;
    sf_malloc(1);
    sf_malloc(100);
    sf_malloc(200);
    sf_malloc(300);
    sf_malloc(500);
    sf_malloc(700);

    //All free blocks == 0
	free_block_count(0, 0);
	//Check sf_errno == 0
	cr_assert(sf_errno == 0, "sf_errno is not zero!");
}

Test(sfmm_student_suite, malloc_allocates_two_full_pages_a_time, .timeout = TEST_TIMEOUT) {
	sf_errno = 0;
	void *x = sf_malloc(4016);

	//All free blocks == 0
	free_block_count(0, 0);
	//Check sf_errno == 0
	cr_assert(sf_errno == 0, "sf_errno is not zero!");
	//Check block size
	cr_assert((((sf_block*)(x-8))->header & ~0x1f) == 4032, "Realloc'ed block size not what was expected!");
	//Check header == footer
	cr_assert(((sf_block*)(x-8))->header == *(sf_footer *)(x-16+(((sf_block*)(x-8))->header>>5)*32), "Header not equal to footer!");
	//Check payload is aligned 32.
	cr_assert(((long)((sf_block*)(x-8))->body.payload%32) == 0, "Payload is not aligned 32!");
}

Test(sfmm_student_suite, free_coalesce_above, .timeout = TEST_TIMEOUT) {
	sf_errno = 0;
	void *x = sf_malloc(1);
	void *y = sf_malloc(1);
	/*void *z =*/sf_malloc(1);

	sf_free(x);
    sf_free(y);

    //free_list[1]=1
    free_list_size(1, 1);
    //free_list[7]=1
    free_list_size(7, 1);
    //1 free block size is 64.
	free_block_count(64, 1);
	//1 free block size is 1920.
	free_block_count(1888, 1);
	//All free blocks == 2
	free_block_count(0, 2);
	//Check sf_errno == 0
	cr_assert(sf_errno == 0, "sf_errno is not zero!");

}

Test(sfmm_student_suite, free_coalesce_below, .timeout = TEST_TIMEOUT) {
	sf_errno = 0;
	void *x = sf_malloc(1);
	void *y = sf_malloc(1);
	/*void *z =*/sf_malloc(1);

	sf_free(y);
    sf_free(x);

    //free_list[1]=1
    free_list_size(1, 1);
    //free_list[7]=1
    free_list_size(7, 1);
    //1 free block size is 64.
	free_block_count(64, 1);
	//1 free block size is 1920.
	free_block_count(1888, 1);
	//All free blocks == 2
	free_block_count(0, 2);
	//Check sf_errno == 0
	cr_assert(sf_errno == 0, "sf_errno is not zero!");

}

Test(sfmm_student_suite, free_coalesce_above_and_below, .timeout = TEST_TIMEOUT) {
	sf_errno = 0;
	void *x = sf_malloc(1);
	void *y = sf_malloc(1);
	void *z = sf_malloc(1);
	/*void *h =*/sf_malloc(1);

	sf_free(x);
	sf_free(z);
    sf_free(y);

    //free_list[2]=1
    free_list_size(2, 1);
    //free_list[7]=1
    free_list_size(7, 1);
    //1 free block size is 96.
	free_block_count(96, 1);
	//1 free block size is 1856.
	free_block_count(1856, 1);
	//All free blocks == 1
	free_block_count(0, 2);
	//Check sf_errno == 0
	cr_assert(sf_errno == 0, "sf_errno is not zero!");
}

Test(sfmm_student_suite, realloc_large_and_check_payload, .timeout = TEST_TIMEOUT) {
	sf_errno = 0;
	void *x = sf_malloc(1);
	*(int *)(x) = 123456;
	void *y = sf_realloc(x, 100);


	//free_list[0]=1
    free_list_size(0, 1);
    //free_list[7]=1
    free_list_size(7, 1);
    //1 free block size is 32.
	free_block_count(32, 1);
	//1 free block size is 1824.
	free_block_count(1824, 1);
	//All free blocks == 1
	free_block_count(0, 2);
	//check payload
	cr_assert(*(int *)(y) == 123456, "payload not copy!");
	//Check sf_errno == 0
	cr_assert(sf_errno == 0, "sf_errno is not zero!");
	//Check return is not NULL;
	cr_assert(y != NULL, "y is NULL!");
	//Check block size
	cr_assert((((sf_block*)(y-8))->header & ~0x1f) == 128, "Realloc'ed block size not what was expected!");
	//Check header == footer
	cr_assert(((sf_block*)(y-8))->header == *(sf_footer *)(y-16+(((sf_block*)(y-8))->header>>5)*32), "Header not equal to footer!");
}

Test(sfmm_student_suite, memalign_not_power_of_2, .timeout = TEST_TIMEOUT) {
	sf_errno = 0;
	void *x = sf_memalign(40, 48);

	//Check sf_errno == 0
	cr_assert(sf_errno == EINVAL, "sf_errno is not EINVAL!");
	//Check return is NULL
	cr_assert(x == NULL, "x is not NULL!");
}

Test(sfmm_student_suite, memalign_not_split_out, .timeout = TEST_TIMEOUT) {
	sf_errno = 0;
	void *x = sf_memalign(40, 32);

    //free_list[7]=1
    free_list_size(7, 1);
    //1 free block size is 1920.
	free_block_count(1920, 1);
	//All free blocks == 1
	free_block_count(0, 1);
	//Check sf_errno == 0
	cr_assert(sf_errno == 0, "sf_errno is not zero!");
	//Check return is not NULL;
	cr_assert(x != NULL, "x is NULL!");
	//Check block size
	cr_assert((((sf_block*)(x-8))->header & ~0x1f) == 64, "Realloc'ed block size not what was expected!");
	//Check header == footer
	cr_assert(((sf_block*)(x-8))->header == *(sf_footer *)(x-16+(((sf_block*)(x-8))->header>>5)*32), "Header not equal to footer!");
	//Check payload is aligned 32.
	cr_assert(((long)((sf_block*)(x-8))->body.payload%32) == 0, "Payload is not aligned 32!");
}

Test(sfmm_student_suite, memalign_64, .timeout = TEST_TIMEOUT) {
	sf_errno = 0;
	void *x = sf_memalign(40, 64);

	//Check if the split is clean.
	cr_assert(((((sf_block*)(x-8))->header>>5)*32-56)/32==0,"split is not clean!");
	//Check sf_errno == 0
	cr_assert(sf_errno == 0, "sf_errno is not zero!");
	//Check return is not NULL;
	cr_assert(x != NULL, "x is NULL!");
	//Check block size
	cr_assert((((sf_block*)(x-8))->header & ~0x1f) == 64, "Realloc'ed block size not what was expected!");
	//Check header == footer
	cr_assert(((sf_block*)(x-8))->header == *(sf_footer *)(x-16+(((sf_block*)(x-8))->header>>5)*32), "Header not equal to footer!");
	//Check payload is aligned 64.
	cr_assert(((long)((sf_block*)(x-8))->body.payload%64) == 0, "Payload is not aligned 64!");
}

Test(sfmm_student_suite, memalign_128, .timeout = TEST_TIMEOUT) {
	sf_errno = 0;
	void *x = sf_memalign(40, 128);

	//Check if the split is clean.
	cr_assert(((((sf_block*)(x-8))->header>>5)*32-56)/32==0,"split is not clean!");
	//Check sf_errno == 0
	cr_assert(sf_errno == 0, "sf_errno is not zero!");
	//Check return is not NULL
	cr_assert(x != NULL, "x is NULL!");
	//Check block size
	cr_assert((((sf_block*)(x-8))->header & ~0x1f) == 64, "Realloc'ed block size not what was expected!");
	//Check header == footer
	cr_assert(((sf_block*)(x-8))->header == *(sf_footer *)(x-16+(((sf_block*)(x-8))->header>>5)*32), "Header not equal to footer!");
	//Check payload is aligned 128.
	cr_assert(((long)((sf_block*)(x-8))->body.payload%128) == 0, "Payload is not aligned 128!");
}

Test(sfmm_student_suite, memalign_512, .timeout = TEST_TIMEOUT) {
	sf_errno = 0;
	void *x = sf_memalign(40, 512);

	//Check if the split is clean.
	cr_assert(((((sf_block*)(x-8))->header>>5)*32-56)/32==0,"split is not clean!");
	//Check sf_errno == 0
	cr_assert(sf_errno == 0, "sf_errno is not zero!");
	//Check return is not NULL;
	cr_assert(x != NULL, "x is NULL!");
	//Check block size
	cr_assert((((sf_block*)(x-8))->header & ~0x1f) == 64, "Realloc'ed block size not what was expected!");
	//Check header == footer
	cr_assert(((sf_block*)(x-8))->header == *(sf_footer *)(x-16+(((sf_block*)(x-8))->header>>5)*32), "Header not equal to footer!");
	//Check payload is aligned 512.
	cr_assert(((long)((sf_block*)(x-8))->body.payload%512) == 0, "Payload is not aligned 512!");
}
