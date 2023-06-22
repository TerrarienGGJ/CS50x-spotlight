// Implements a dictionary's functionality

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "dictionary.h"

// Represents a node in a hash table
typedef struct node
{
    char word[LENGTH + 1];
    struct node *next;
}
node;

void free_linkedlist(node *n);

// Choose number of buckets in hash table
const unsigned int N = 50;

// Hash table
node *table[N];

// Word count
unsigned int word_count = 0;

// Returns true if word is in dictionary, else false
bool check(const char *word)
{
    // Cover empty word cases
    if (!strlen(word))
        return false;
    // Initialize a node pointer, using hash as table index to explore
    node *cursor = table[hash(word)];
    // Parse the linked list by checking if the word is found, if not, move to the next node until the last, if none found, return false
    while (cursor != NULL)
    {
        if (strcasecmp(word, cursor->word) == 0)
            return true;
        else
            cursor = cursor->next;
    }
    return false;
}

// Hashes word to a number
unsigned int hash(const char *word)
{
    int sum = 0;
    for (int i = 0, len = strlen(word); i < len; i++)
        sum += (int)toupper(word[i]);
    return (sum % N);
}

// Loads dictionary into memory, returning true if successful, else false
bool load(const char *dictionary)
{
    // Set hash table to NULL pointers
    for (int i = 0; i < N; i++)
        table[i] = NULL;
    // Open the dictionary
    FILE *source = fopen(dictionary, "r");
    if (source == NULL)
        return false;
    char buff[LENGTH + 1];
    while (fscanf(source, "%s", buff) > 0)
    {
        int index = hash(buff);
        node *n = malloc(sizeof(node));
        if (n == NULL)
        {
            fclose(source);
            return false;
        }
        strcpy(n->word, buff);
        n->next = table[index];
        table[index] = n;
        word_count++;
    }
    fclose(source);
    return true;
}

// Returns number of words in dictionary if loaded, else 0 if not yet loaded
unsigned int size(void)
{
    return word_count;
}

// Unloads dictionary from memory, returning true if successful, else false
bool unload(void)
{
    //printf("unload");
    for (int i = 0; i < N; i++)
        free_linkedlist(table[i]);
    return true;
}

// Unload a linked list
void free_linkedlist(node *n)
{
    if (n != NULL)
    {
        free_linkedlist(n->next);
        free(n);
    }
    return;
}
