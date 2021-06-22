#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>

int has_initialized = 0;

// our memory area we can allocate from, here 64 kB
#define MEM_SIZE (64 * 1024)
uint8_t heap[MEM_SIZE];

// start and end of our own heap memory area
void *managed_memory_start;

// this block is stored at the start of each free and used block
struct mem_control_block
{
    int size;
    struct mem_control_block *next;
};

// pointer to start of our free list
struct mem_control_block *free_list_start;

void mymalloc_init()
{

    // our memory starts at the start of the heap array
    managed_memory_start = &heap;

    // allocate and initialize our memory control block
    // for the first (and at the moment only) free block
    struct mem_control_block *m = (struct mem_control_block *)managed_memory_start;
    m->size = MEM_SIZE - sizeof(struct mem_control_block);

    // no next free block
    m->next = (struct mem_control_block *)0;

    // initialize the start of the free list
    free_list_start = m;

    // We're initialized and ready to go
    has_initialized = 1;
}

void *mymalloc(long numbytes)
{
    if (numbytes <= 0)
    { // Same as malloc in manpages
        return NULL;
    }

    if (has_initialized == 0)
    {
        mymalloc_init();
    }

    int round = 8 - numbytes % 8;
    if (round != 8)
    {
        numbytes = round + numbytes;
    }
    struct mem_control_block *checking = free_list_start;
    struct mem_control_block *prevChecking = (struct mem_control_block *)0;

    while (checking->size < numbytes)
    {
        if (checking->next == (struct mem_control_block *)0)
        {
            printf("No space in memory for element with size: %ld \n", numbytes);
            return NULL;
        };
        prevChecking = checking;
        checking = checking->next;
    }
    void *returnAddress = checking + 1; //adds the sizeof(struct mem_control_block) by adding 1
    int leftovers = checking->size - numbytes;
    if (leftovers >= sizeof(struct mem_control_block))
    {
        struct mem_control_block *freeblock = (struct mem_control_block *)((char *)checking + numbytes + sizeof(struct mem_control_block));
        freeblock->size = leftovers - sizeof(struct mem_control_block);
        freeblock->next = checking->next;
        checking->size = numbytes;
        checking->next = freeblock;

        if (prevChecking != 0)
        {
            prevChecking->next = freeblock;
            return returnAddress;
        }
        else
        {
            free_list_start = freeblock;
            return returnAddress;
        }
    }
    else
    {
        if (prevChecking != 0)
        {
            prevChecking->next = checking->next;
            return returnAddress;
        }
        else
        {
            free_list_start = checking->next; //This might be NULL
            return returnAddress;
        }
    }
}

void myfree(void *firstbyte)
{
    if (firstbyte != NULL) //If ptr is NULL, no operation is performed.
    {

        if ((char *)firstbyte < (char *)managed_memory_start || (char *)firstbyte >= (char *)managed_memory_start + MEM_SIZE)
        {
            fprintf(stderr, "The memory address is out of scope! \n");
            exit(EXIT_FAILURE);
        }

        void *blockpointer = firstbyte - sizeof(struct mem_control_block); //removes the sizeof(struct mem_control_block) by remvoing 1
        struct mem_control_block *block = blockpointer;

        if (!block->size)
        {
            // If the block doesn't exists block->size == NULL
            // The block can be undefined and have size != NULL if one uses myfree two times on same location
            // Then undefined behavior occurs. (Same as free in the manpages)
            fprintf(stderr, "The pointer points to an undefined block \n");
            exit(EXIT_FAILURE);
        }

        if (!free_list_start) //If the list was full
        {
            block->next = (struct mem_control_block *)0; //Size is set from myalloc
            free_list_start = block;
        }
        else if (free_list_start > block)
        {
            if ((char *)block + block->size + sizeof(struct mem_control_block) == (char *)free_list_start)
            { // Join the two blocks
                block->size += free_list_start->size + sizeof(struct mem_control_block);
                block->next = free_list_start->next;
                free_list_start = block;
            }
            else
            { // A block between the two
                block->next = free_list_start;
                free_list_start = block;
            }
        }
        else if (free_list_start < block)
        {
            struct mem_control_block *checking = free_list_start;
            struct mem_control_block *prevChecking = (struct mem_control_block *)0;

            while (checking < block)
            {
                prevChecking = checking;
                checking = checking->next;
            }
            if (checking == (struct mem_control_block *)0)
            {
                //block is the last free memory location in the heap, set prevchecking->next to block
                prevChecking->next = block;
                block->next = (struct mem_control_block *)0;
            }
            else if ((char *)block + block->size + sizeof(struct mem_control_block) == (char *)checking)
            {
                //block is next to checking, set prevchecking->next to block, block->next to checking->next
                //add block and checking size together
                block->size += sizeof(struct mem_control_block) + checking->size;
                block->next = checking->next;
                prevChecking->next = block;
            }
            else
            {
                //Can't join the blocks, block is inbetween
                prevChecking->next = block;
                if (prevChecking == checking)
                {
                    block->next = (struct mem_control_block *)0;
                }
                block->next = checking;
            }
            if ((char *)prevChecking + prevChecking->size + sizeof(struct mem_control_block) == (char *)block)
            {
                prevChecking->size += sizeof(struct mem_control_block) + block->size;
                prevChecking->next = block->next;
            }
        }
        else
        {
            fprintf(stderr, "Something bad happend! \n");
            exit(EXIT_FAILURE);
        }
    }
}

// functions used to generalize tests
void test_is_null(void *pointer, char *name)
{
    if (pointer)
    {
        fprintf(stderr, "%s does not point to null \n", name);
        exit(EXIT_FAILURE);
    }
}

void test_is_not_null(void *pointer, char *name)
{
    if (!pointer)
    {
        fprintf(stderr, "the pointer for %s was not returned \n", name);
        exit(EXIT_FAILURE);
    }
}

void test_pointer_size(struct mem_control_block *pointer, int checksum, char *name)
{
    if (checksum != pointer->size)
    {
        fprintf(stderr, "The size of the memory in %s did not match the expected value. \n", name);
        exit(EXIT_FAILURE);
    }
}

void test_pointer_address(void *pointer, void *reference, int size, char *name)
{
    if ((char *)pointer != (char *)reference + size)
    {
        fprintf(stderr, "The address of %s was not correct. \n", name);
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv)
{
    int input;
    while (input != -1)
    {
        printf("Which case do you want? (Use number matching case. -1 for exit): ");
        scanf("%d", &input);
        switch (input)
        {
        case 1:
        { // Case to try to remove out of scope value. Should exit
            printf("running test 1 \n");
            if (has_initialized == 0)
            {
                mymalloc_init();
            }
            myfree((void *)(managed_memory_start - 64));
            break;
        }

        case 2:
        { // Case to try to remove out of scope value. Should exit
            printf("running test 2 \n");
            if (has_initialized == 0)
            {
                mymalloc_init();
            }
            myfree((void *)(managed_memory_start + MEM_SIZE));
            break;
        }
        case 3:
        { //Fill whole heap with one block
            printf("running test 3 \n");
            void *e;
            e = mymalloc(MEM_SIZE - 16); // allocate max sizes bytes
            test_is_null(free_list_start, "free_list_start");
            struct mem_control_block *assigned_block = (struct mem_control_block *)((char *)e - sizeof(struct mem_control_block));
            int d = assigned_block->size;
            myfree(e);
            test_pointer_size(free_list_start, d, "free_list_start");
            printf("Code successful! \n");
            break;
        }
        case 4:
        {
            //Check if one can add and remove multiple elements of different size
            printf("running test 4 \n");
            void *a;
            void *b;
            void *c;
            void *d;
            void *e;
            a = mymalloc(1); // allocate 1 byte, rounded to 8
            test_is_not_null(a, "a");
            test_pointer_address(free_list_start, a, 8, "free_list_start");
            b = mymalloc(10); // allocate 10 bytes, rounded to 16
            test_is_not_null(b, "b");
            test_pointer_address(b, a, 8 + sizeof(struct mem_control_block), "b");
            c = mymalloc(16); // allocate 16 bytes
            test_is_not_null(c, "c");
            test_pointer_address(c, b, 16 + sizeof(struct mem_control_block), "c");
            d = mymalloc(3100); // allocate 3100 bytes, rounded to 3104
            test_is_not_null(d, "d");
            test_pointer_address(d, c, 16 + sizeof(struct mem_control_block), "d");
            e = mymalloc(8); // allocate 8 bytes
            test_is_not_null(e, "e");
            test_pointer_address(e, d, 3104 + sizeof(struct mem_control_block), "e");
            myfree(a);
            myfree(b);
            myfree(c);
            myfree(d);
            myfree(e);
            printf("Code successful! \n");
            break;
        }
        case 5:
        { //Try to insert too big value
            printf("running test 5 \n");
            void *e;
            e = mymalloc(MEM_SIZE); // allocate max sizes bytes
            test_is_null(e, "too large byte pointer");
            printf("Code successful! \n");
            break;
        }
        case 6:
        { //Try to allocate zero bytes
            printf("running test 6 \n");
            void *e;
            e = mymalloc(0); // allocate zero bytes
            test_is_null(e, "zero byte pointer");
            printf("Code successful! \n");
            break;
        }
        case 7:
        { //Try to allocate negative bytes
            printf("running test 7 \n");
            void *e;
            e = mymalloc(-1); // allocate negative bytes
            test_is_null(e, "negative byte pointer");
            printf("Code successful! \n");
            break;
        }
        case 8:
        {
            // Test of merging logic:
            // 1. After a freed block
            // 2. Free_list_start jumps over blocks
            // 3. Inbetween two freed blocks
            printf("running test 8 \n");
            void *a;
            void *b;
            void *c;
            int a_size = 8;
            int b_size = 16;
            int c_size = 32;
            a = mymalloc(a_size);
            b = mymalloc(b_size);
            c = mymalloc(c_size);
            int oldSize = free_list_start->size;
            //Check if freed block merges with the one after
            myfree(c);
            test_pointer_address(free_list_start, b, b_size, "free_list_start");
            test_pointer_size(free_list_start, oldSize + sizeof(struct mem_control_block) + c_size, "free_list_start");
            //check if free_list_start changes blocks
            myfree(a);
            test_pointer_size(free_list_start, a_size, "free_list_start");
            //check if freed block inbetween two free blocks merges into one
            myfree(b);
            test_pointer_size(free_list_start, MEM_SIZE - sizeof(struct mem_control_block), "free_list_start");
            printf("Code successful! \n");
            break;
        }
        case 9:
        {
            // Test of merging logic:
            // 1. Free_list_start jumps over blocks
            // 2. Before a freed block
            printf("running test 9 \n");
            void *a;
            void *b;
            void *c;
            int a_size = 8;
            int b_size = 16;
            a = mymalloc(a_size);
            b = mymalloc(b_size);
            c = mymalloc(32);
            //check if free_list_start changes blocks
            myfree(a);
            test_pointer_size(free_list_start, a_size, "free_list_start");
            // Checks if freed block merges with the one block before
            myfree(b);
            test_pointer_size(free_list_start, a_size + sizeof(struct mem_control_block) + b_size, "free_list_start");
            myfree(c);
            test_pointer_size(free_list_start, MEM_SIZE - sizeof(struct mem_control_block), "free_list_start");
            printf("Code successful! \n");
            break;
        }
        case -1:
        { //Exit tests
            break;
        }
        default:
        {
            printf("Out of test range");
            break;
        }
        }
    }
}