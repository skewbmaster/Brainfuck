#include <stdio.h>
#include <malloc.h>

#define INSIDE_LOOP_ALLOCATION 4

static char memory[30000] = { 0 };
static char code[30000];

struct loop
{
    int start;
    int end;
    int inside_loops_passed;
    int inside_loop_count;
    int inside_loops_allocated;

    struct loop* inside_loops;
    struct loop* outer_loop;
};

void cleanLoops(struct loop* main_loop)
{
    for (int i = 0; i < main_loop->inside_loop_count; ++i)
    {
        if (main_loop->inside_loops[i].inside_loops != NULL)
        {
            cleanLoops(&main_loop->inside_loops[i]);
        }
    }
    if (main_loop->inside_loops != NULL)
    {
        free(main_loop->inside_loops);
    }
}

int main(int argc, char *argv[]) {

    FILE* file;

    if (argc != 2) {
        file = fopen("program.b", "r");
    }
    else {
        file = fopen(argv[1], "r");
    }

    fgets(&code, 30000, file);
    fclose(file);

    char* memptr = memory;
    int codeptr = 0;


    struct loop main_loop_struct = {-1, -1, 0, 0, 4, NULL, NULL};
    main_loop_struct.inside_loops = (struct loop*) malloc(sizeof(struct loop) * main_loop_struct.inside_loops_allocated);

    struct loop* current_loop = &main_loop_struct;

    while (code[codeptr] != 0)
    {
        if (code[codeptr] == '[')
        {
            struct loop* next_loops = current_loop->inside_loops; // Gets us pointer to inside loops of current loop

            if (next_loops == NULL) { // If pointer uninitialised, create new pointer of size INSIDE_LOOP_ALLOCATION
                current_loop->inside_loops_allocated = INSIDE_LOOP_ALLOCATION;
                next_loops = (struct loop*) malloc(sizeof(struct loop) * INSIDE_LOOP_ALLOCATION);
                current_loop->inside_loops = next_loops;
            }
            else if (current_loop->inside_loop_count >= current_loop->inside_loops_allocated) // Check if current loop count has reached the allocated limit, only possible if initialised
            {
                current_loop->inside_loops_allocated <<= 1;
                struct loop* doubled_loops;
                // Reallocate space by doubling (basics of dynamic memory)
                doubled_loops = (struct loop*) realloc(next_loops, sizeof(struct loop) * current_loop->inside_loops_allocated);

                if (doubled_loops == NULL) // If reallocation fails, stop the program (memory issue)
                {
                    printf("Memory reallocation failed");
                    getchar();
                    return 1;
                }
                else if (doubled_loops != next_loops)
                {
                    free(next_loops);
                    current_loop->inside_loops = doubled_loops;
                    next_loops = doubled_loops;

                    for (int i = 0; i < current_loop->inside_loops->inside_loop_count; ++i)
                    {
                        current_loop->inside_loops->inside_loops[i].outer_loop = doubled_loops;
                    }
                }
            }

            next_loops[current_loop->inside_loop_count].outer_loop = current_loop; // Set next inside loop's outer loop to the current so that we can go back when it's done
            current_loop->inside_loop_count++; // Increment to keep track of which loop is being built within itself

            current_loop = &next_loops[current_loop->inside_loop_count-1]; // Finally, transition over to the next inside loop

            current_loop->start = codeptr; // Set the code pointer and initialise rest of struct
            current_loop->inside_loop_count = 0;
            current_loop->inside_loops_passed = 0;
            current_loop->inside_loops = NULL;
        }
        else if (code[codeptr] == ']')
        {
            if (current_loop->outer_loop == NULL)
            {
                printf("Invalid loop syntax, ended loop with no loop starter");
                getchar();
                return 1;
            }

            current_loop->end = codeptr; // Set the code pointer for the end of the loop
            current_loop = current_loop->outer_loop; // Go back to previous loop struct
        }


        codeptr++;
    }

    if (current_loop != &main_loop_struct)
    {
        printf("Invalid loop syntax, no matching end loop for n iterations");
        getchar();
    }

    codeptr = 0;

    while (code[codeptr] != 0)
    {
        switch (code[codeptr])
        {
            case '+':
            {
                ++*memptr;
                break;
            }
            case '-':
            {
                --*memptr;
                break;
            }
            case '>':
            {
                memptr++;
                break;
            }
            case '<':
            {
                memptr--;
                break;
            }
            case '.':
            {
                printf("%c", *memptr);
                break;
            }
            case ',':
            {
                char input;
                scanf_s("%c", &input);
                *memptr = input;
                break;
            }

            case '[':
            {
                current_loop->inside_loops_passed++;
                if (*memptr == 0)
                {
                    codeptr = current_loop->inside_loops[current_loop->inside_loops_passed - 1].end + 1;
                    continue;
                }
                current_loop = &current_loop->inside_loops[current_loop->inside_loops_passed - 1];
                break;
            }
            case ']':
            {
                current_loop->inside_loops_passed = 0;
                if (*memptr != 0)
                {
                    codeptr = current_loop->start + 1;
                    continue;
                }
                current_loop = current_loop->outer_loop;
                break;
            }
        }
        codeptr++;
    }

    cleanLoops(&main_loop_struct);

    getchar();

    return 0;
}

