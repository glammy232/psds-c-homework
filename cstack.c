#include "cstack.h"

#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 10

typedef struct node
{
    struct node* next;
    unsigned int size;
    void* data;
} node_t;

typedef struct
{
    node_t* top;
    unsigned int count;
} stack_t;

static stack_t** stack_pool = NULL;
static size_t stack_pool_capacity = 0;

static void init_stack_pool(void)
{
    size_t capacity = INITIAL_CAPACITY;
    stack_t** pool = calloc(capacity, sizeof(stack_t*));
    if (pool == NULL)
    {
        return;
    }
    stack_pool = pool;
    stack_pool_capacity = capacity;
}

static int push_impl(stack_t* s, const void* data_in, unsigned int size)
{
    if (s == NULL || data_in == NULL || size == 0)
    {
        return 0;
    }

    node_t* new_node = malloc(sizeof(node_t));
    if (new_node == NULL)
    {
        return 0;
    }

    new_node->data = malloc(size);
    if (new_node->data == NULL)
    {
        free(new_node);
        return 0;
    }

    new_node->size = size;
    memcpy(new_node->data, data_in, size);

    new_node->next = s->top;
    s->top = new_node;
    s->count++;

    return 1;
}

hstack_t stack_new(void)
{
    if (stack_pool == NULL)
    {
        init_stack_pool();
        if (stack_pool == NULL)
        {
            return -1;
        }
    }

    size_t free_id = 0;
    int found_free = 0;

    for (size_t i = 0; i < stack_pool_capacity; i++)
    {
        if (stack_pool[i] == NULL)
        {
            free_id = i;
            found_free = 1;
            break;
        }
    }

    if (!found_free)
    {
        size_t new_capacity = stack_pool_capacity + stack_pool_capacity / 2;
        if (new_capacity <= stack_pool_capacity)
        {
            return -1;
        }

        stack_t** new_pool =
            realloc(stack_pool, new_capacity * sizeof(stack_t*));
        if (new_pool == NULL)
        {
            return -1;
        }

        for (size_t i = stack_pool_capacity; i < new_capacity; i++)
        {
            new_pool[i] = NULL;
        }

        free_id = stack_pool_capacity;
        stack_pool = new_pool;
        stack_pool_capacity = new_capacity;
    }

    stack_t* stack = malloc(sizeof(stack_t));
    if (stack == NULL)
    {
        return -1;
    }

    stack->top = NULL;
    stack->count = 0;
    stack_pool[free_id] = stack;

    return (hstack_t)free_id;
}

int stack_valid_handler(const hstack_t stack)
{
    if (stack_pool == NULL || stack < 0 || (size_t)stack >= stack_pool_capacity)
    {
        return 0;
    }
    return stack_pool[stack] != NULL ? 1 : 0;
}

void stack_free(const hstack_t stack)
{
    if (!stack_valid_handler(stack))
    {
        return;
    }

    stack_t* s = stack_pool[stack];
    node_t* current = s->top;

    while (current != NULL)
    {
        node_t* to_free = current;
        current = current->next;
        free(to_free->data);
        free(to_free);
    }

    free(s);
    stack_pool[stack] = NULL;
}

void stack_free_all(void)
{
    if (stack_pool == NULL)
    {
        return;
    }
    for (size_t i = 0; i < stack_pool_capacity; i++)
    {
        if (stack_pool[i] != NULL)
        {
            stack_free((hstack_t)i);
        }
    }
    free(stack_pool);
    stack_pool = NULL;
    stack_pool_capacity = 0;
}

unsigned int stack_size(const hstack_t stack)
{
    if (!stack_valid_handler(stack))
    {
        return 0;
    }
    return stack_pool[stack]->count;
}

unsigned int stack_top_size(const hstack_t stack)
{
    if (!stack_valid_handler(stack) || stack_pool[stack]->top == NULL)
    {
        return 0;
    }
    return stack_pool[stack]->top->size;
}

void
stack_push(const hstack_t stack, const void* data_in, const unsigned int size)
{
    if (!stack_valid_handler(stack))
    {
        return;
    }
    (void)push_impl(stack_pool[stack], data_in, size);
}

unsigned int
stack_pop(const hstack_t stack, void* data_out, const unsigned int size)
{
    if (!stack_valid_handler(stack) || stack_pool[stack]->top == NULL)
    {
        return 0;
    }

    stack_t* s = stack_pool[stack];
    node_t* top_node = s->top;

    if (data_out == NULL || size < top_node->size)
    {
        return 0;
    }

    unsigned int actual_size = top_node->size;
    memcpy(data_out, top_node->data, actual_size);

    s->top = top_node->next;
    s->count--;

    free(top_node->data);
    free(top_node);

    return actual_size;
}

hstack_t stack_get_all(void)
{
    hstack_t all_stack_id = stack_new();
    if (all_stack_id == -1)
    {
        return -1;
    }

    for (size_t i = 0; i < stack_pool_capacity; i++)
    {
        if (stack_pool[i] != NULL && (hstack_t)i != all_stack_id)
        {
            hstack_t valid_id = (hstack_t)i;

            if (!push_impl(
                    stack_pool[all_stack_id], &valid_id, sizeof(hstack_t)
                ))
            {
                stack_free(all_stack_id);
                return -1;
            }
        }
    }

    return all_stack_id;
}
