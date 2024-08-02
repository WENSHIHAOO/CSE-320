/**
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "sfmm.h"

void *find_fit(size_t size){
    int i=0;
    sf_block *block1;
    //Find the suitable block.
    while(i<8){
        block1 = (sf_free_list_heads+i)->body.links.next;
        while(block1 != (sf_free_list_heads+i)){
            if((block1->header>>5) >= size){
                block1->body.links.next->body.links.prev=block1->body.links.prev;
                block1->body.links.prev->body.links.next=block1->body.links.next;
                block1->body.links.next=NULL;
                block1->body.links.prev=NULL;
                block1->header |= 16;
                break;
            }
            block1 = block1->body.links.next;
        }
        i++;
    }
    //There is no suitable block.
    if((block1->header>>5) < size){
        return NULL;
    }

    size_t size0;
    sf_footer *floot1 = (sf_footer *)((sf_footer *)block1+(size*32-8)/8);
    if((size0=(block1->header>>5)-size) >= 1){
        //When the found block is too large, split the block. and set their header and link.
        sf_footer *floot0 = (sf_footer *)((sf_footer *)block1+((block1->header>>5)*32-8)/8);
        sf_block *block0 = (sf_block *)((sf_footer *)block1+size*32/8);
        size0<<=5;
        size0+=16;
        block0->header=size0;
        *floot0 = size0;
        if(i==8){
            block0->header-=16;
            *floot0-=16;
            (sf_free_list_heads+7)->body.links.next=block0;
            block0->body.links.next=(sf_free_list_heads+7);
            block0->body.links.prev=(sf_free_list_heads+7);
            (sf_free_list_heads+7)->body.links.prev=block0;
        }
        else{
            sf_free(block0->body.payload);
        }
    }
    size<<=5;
    size+=16;
    block1->header = size;
    *floot1 = size;
    return block1;
}

void *coalesce(void *pp){
    void *block = pp-8;
    size_t size = ((sf_block *)block)->header>>5;
    sf_footer *foot = (sf_footer *)(block+size*32-8);
    //Check adjacent free block
    if((*(sf_footer *)(block-8)>>4)%2==1 && (*(sf_header *)(foot+1)>>4)%2==1){
        return block;
    }
    else if((*(sf_footer *)(block-8)>>4)%2==0 && (*(sf_header *)(foot+1)>>4)%2==1){
        sf_block *prev_block=(sf_block *)(block-(*(sf_footer *)(block-8)>>5)*32);
        size+=(prev_block->header>>5);
        //Set and remove link
        prev_block->body.links.prev->body.links.next = prev_block->body.links.next;
        prev_block->body.links.next->body.links.prev = prev_block->body.links.prev;
        prev_block->body.links.next=NULL;
        prev_block->body.links.prev=NULL;
        //Set block
        *(sf_footer *)(block-8)=0;
        ((sf_block *)block)->header=0;
        prev_block->header = (size<<5);
        *foot = (size<<5);
        block = prev_block;
    }
    else if((*(sf_footer *)(block-8)>>4)%2==1 && (*(sf_header *)(foot+1)>>4)%2==0){
        sf_block *next_block=(sf_block *)(foot+1);
        size+=(next_block->header>>5);
        //Set and remove link
        next_block->body.links.prev->body.links.next = next_block->body.links.next;
        next_block->body.links.next->body.links.prev = next_block->body.links.prev;
        next_block->body.links.next=NULL;
        next_block->body.links.prev=NULL;
        //Set block
        *foot=0;
        next_block->header=0;
        ((sf_block *)block)->header = (size<<5);
        *(sf_footer *)(block+size*32-8) = (size<<5);
    }
    else{
        sf_block *prev_block=(sf_block *)(block-(*(sf_footer *)(block-8)>>5)*32);
        sf_block *next_block=(sf_block *)(foot+1);
        size+=((next_block->header>>5)+(prev_block->header>>5));
        //Set and remove link
        prev_block->body.links.prev->body.links.next = prev_block->body.links.next;
        prev_block->body.links.next->body.links.prev = prev_block->body.links.prev;
        prev_block->body.links.next=NULL;
        prev_block->body.links.prev=NULL;
        next_block->body.links.prev->body.links.next = next_block->body.links.next;
        next_block->body.links.next->body.links.prev = next_block->body.links.prev;
        next_block->body.links.next=NULL;
        next_block->body.links.prev=NULL;
        //Set block
         *(sf_footer *)(block-8)=0;
        ((sf_block *)block)->header=0;
        *foot=0;
        next_block->header=0;
        prev_block->header = (size<<5);
        block = prev_block;
        *(sf_footer *)(block+size*32-8) = (size<<5);
    }

    return block;
}

void *sf_malloc(size_t size) {
    if(sf_free_list_heads->body.links.prev==NULL){
        //Initialize sf_free_list_heads
        int i=0;
        while(i<8){
            (sf_free_list_heads+i)->body.links.prev=(sf_free_list_heads+i);
            (sf_free_list_heads+i)->body.links.next=(sf_free_list_heads+i);
            i++;
        }
    }

    size_t asize;
    void *block;
    //Determine the size of the block size.
    if(size == 0){
        return NULL;
    }
    if(size <= 16){
        asize=32;
    }
    else{
        float fsize = ((size+16)/32.0);
        int isize=(int)fsize;
        fsize=fsize-isize;
        if(fsize>0){
            isize++;
        }
        asize=32*isize;
    }

    while((block=find_fit(asize/32))==NULL){
        void *page=sf_mem_grow();
        if(page == NULL){
            sf_errno = 12;
            return NULL;
        }
        sf_block *block;
        if(page == sf_mem_start()){
            //initialize heap
            //Set prologue
            sf_header *heapStart0 = (sf_header *)page;
            sf_header *heapStart1 = (sf_header *)(page+8);
            sf_header *heapStart2 = (sf_header *)(page+16);
            sf_block *prologue0 = (sf_block *)(page+24);
            sf_footer *prologue1 = (sf_footer *)(page+48);
            *heapStart0 = 0;
            *heapStart1 = 0;
            *heapStart2 = 0;
            prologue0->header = (1<<5) + 16;
            *prologue1 = (1<<5) + 16;
            //Set block
            block = page+56;
            sf_footer *foot = (sf_footer *)(sf_mem_end()-16);
            block->header = ((sf_mem_end()-sf_mem_start()-64)/32)<<5;
            *foot = ((sf_mem_end()-sf_mem_start()-64)/32)<<5;
            //Set list
            (sf_free_list_heads+7)->body.links.next=block;
            block->body.links.next=(sf_free_list_heads+7);
            block->body.links.prev=(sf_free_list_heads+7);
            (sf_free_list_heads+7)->body.links.prev=block;
        }
        else{
            //uninitialized heap
            if((*(sf_footer *)(page-16)>>4)%2 == 1){
                //Set block
                block = page-8;
                sf_footer *foot = (sf_footer *)(sf_mem_end()-16);
                block->header = (64)<<5;
                *foot = (64)<<5;
            }
            else{
                //Set block
                block=page-8-((*(sf_footer *)(page-16))>>5)*32;
                sf_footer *foot = (sf_footer *)(sf_mem_end()-16);
                block->header = ((64+(*(sf_footer *)(page-16)>>5)))<<5;
                *foot = ((64+(*(sf_footer *)(page-16)>>5)))<<5;
                (*(sf_footer *)(page-16))=0;
                (*(sf_footer *)(page-8))=0;
            }
            //Set list
            if((sf_free_list_heads+7)->body.links.next == block){
                (sf_free_list_heads+7)->body.links.next=block;
                block->body.links.next=(sf_free_list_heads+7);
                block->body.links.prev=(sf_free_list_heads+7);
                (sf_free_list_heads+7)->body.links.prev=block;
            }
            else{
                block->body.links.next=(sf_free_list_heads+7);
                block->body.links.prev=(sf_free_list_heads+7)->body.links.prev;
                (sf_free_list_heads+7)->body.links.prev->body.links.next=block;
                (sf_free_list_heads+7)->body.links.prev=block;
            }
        }
        //Set epilogue
        sf_footer *epilogue = (sf_footer *)(sf_mem_end()-8);
        *epilogue = 16;
    }
    return ((sf_block *)block)->body.payload;
}

void sf_free(void *pp) {
    if(pp==NULL){
        sf_errno = 22;
        abort();
    }
    sf_block* block = (sf_block *)(pp-8);
    sf_footer* foot = (sf_footer*)(pp-16+(block->header>>5)*32);
    //Check valid pointer
    if(((long)pp%32!=0) || block->header>>5==0 || (block->header>>4)%2==0 ||
        (long)block<=(long)sf_mem_start() || (long)foot>=(long)sf_mem_end() || block->header!=*foot){
        sf_errno = 22;
        abort();
    }
    block = coalesce(pp);
    size_t size=(block->header>>5);
    if((((void *)block)+size*32+8) == sf_mem_end()){
        size=7;
    }
    else if(size<4){
        size--;
    }
    else if(size>3 && size<6){
        size=3;
    }
    else if(size>5 && size<9){
        size=4;
    }
    else if(size>8 && size<14){
        size=5;
    }
    else {
        size=6;
    }

    //Set list
    sf_block *block1 = (sf_free_list_heads+size)->body.links.next;
    if(block1 == (sf_free_list_heads+size)){
        block->body.links.next = block1;
        block->body.links.prev = block1;
        block1->body.links.prev=block;
        block1->body.links.next=block;
        if((block->header>>4)%2==1){
            block->header-=16;
        }
        foot = (sf_footer *)((void *)block+(block->header>>5)*32-8);
        if((*foot>>4)%2==1){
            *foot-=16;
        }
    }
    else{
        block1 = (sf_free_list_heads+size);//None order
        //while(block1 != (sf_free_list_heads+size)){
            //if((block1->header>>5) <= (block->header>>5)){
                block->body.links.next = block1->body.links.next;
                block->body.links.prev = block1;
                block1->body.links.next->body.links.prev=block;
                block1->body.links.next=block;
                if((block->header>>4)%2==1){
                    block->header-=16;
                }
                foot = (sf_footer *)((void *)block+(block->header>>5)*32-8);
                if((*foot>>4)%2==1){
                    *foot-=16;
                }
                //break;
            //}
            //block1 = block1->body.links.next;
        //}
    }

    return;
}

void *sf_realloc(void *pp, size_t rsize) {
    if(pp==NULL){
        sf_errno = 22;
        abort();
    }
    sf_block* block = (sf_block *)(pp-8);
    sf_footer* foot = (sf_footer*)(pp-16+(block->header>>5)*32);
    if(((long)pp%32!=0) || block->header>>5==0 || (block->header>>4)%2==0 ||
        (long)block<=(long)sf_mem_start() || (long)foot>=(long)sf_mem_end() || block->header!=*foot){
        sf_errno = 22;
        abort();
    }

    if(rsize==0){
        sf_free(pp);
        return NULL;
    }
    else if((block->header>>5)*32 < rsize+16){
        sf_block* new_block = sf_malloc(rsize)-8;
        if(new_block == NULL){
            return NULL;
        }
        memcpy(new_block->body.payload, pp, (block->header>>5)*32-16);
        sf_free(pp);
        return new_block->body.payload;
    }
    else if((block->header>>5)*32-32 >= rsize+16){
        //When the found block is too large, split the block. and set their header and link.
        size_t asize;
        if(rsize <= 16){
            asize=32;
        }
        else{
            float fsize = ((rsize+16)/32.0);
            int isize=(int)fsize;
            fsize=fsize-isize;
            if(fsize>0){
                isize++;
            }
            asize=32*isize;
        }

        size_t size=asize/32;
        sf_block *block1 = block;
        sf_footer *foot0 = (sf_footer *)((sf_footer *)block1+((block1->header>>5)*32-8)/8);
        sf_footer *foot1 = (sf_footer *)((sf_footer *)block1+(size*32-8)/8);
        sf_block *block0 = (sf_block *)((sf_footer *)block1+size*32/8);
        size_t size0=(block1->header>>5)-size;
        size<<=5;
        size+=16;
        size0<<=5;
        size0+=16;
        block1->header = size;
        *foot1 = size;
        block0->header=size0;
        *foot0 = size0;
        sf_free(block0->body.payload);
        return block1->body.payload;
    }
    else{
        return block->body.payload;
    }
}

void *sf_memalign(size_t size, size_t align) {
    //Check align is large than 32, And power of 2
    size_t a = align;
    if(a<32){
        sf_errno = 22;
        return NULL;
    }
    if(a==32){
        return sf_malloc(size);
    }
    while(a>32){
        if(a%2 == 1){
            sf_errno = 22;
            return NULL;
        }
        a/=2;
        if(a==32){
            break;
        }
        else if(a<32){
            sf_errno = 22;
            return NULL;
        }
    }
    //Find a pointer is align byte aligned
    a=align+size;
    void *free1=sf_malloc(a)-8;
    sf_footer *foot2 = free1-8+(((sf_block*)free1)->header>>5)*32;
    void *block = free1;
    while(((long)block+8)%align != 0){
        block+=32;
    }

    size_t asize=0;
    if(size <= 16){
        asize=1;
    }
    else{
        float fsize = ((size+16)/32.0);
        int isize=(int)fsize;
        fsize=fsize-isize;
        if(fsize>0){
            isize++;
        }
        asize=isize;
    }
    sf_footer *block_foot=block+asize*32-8;
    asize<<=5;
    asize+=16;
    ((sf_block *)block)->header=asize;
    *block_foot=asize;
    //Find free2
    if((long)(((void *)block_foot)+32) <= (long)(foot2)){
        void *free2 = ((void *)block_foot)+8;
        size_t size2=((((long)foot2-(long)free2)+8)/32);
        size2<<=5;
        size2+=16;
        ((sf_block*)free2)->header = size2;
        *(sf_footer *)foot2=size2;
        sf_free(((sf_block*)free2)->body.payload);
    }
    else{
        *block_foot=0;
        block_foot=foot2;
        *block_foot=asize*32;
    }
    //Find free1
    if(block-free1>=32){
        void *foot1=block-8;
        size_t size1=((((long)foot1-(long)free1)+8)/32);
        size1<<=5;
        size1+=16;
        ((sf_block*)free1)->header = size1;
        *(sf_footer *)foot1=size1;
        sf_free(((sf_block*)free1)->body.payload);
    }
    return ((sf_block *)block)->body.payload;
}
