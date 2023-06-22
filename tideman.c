#include <cs50.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Max number of candidates
#define MAX 9

// preferences[i][j] is number of voters who prefer i over j
int preferences[MAX][MAX];

// locked[i][j] means i is locked in over j
bool locked[MAX][MAX];

// Each pair has a winner, loser
typedef struct
{
    int winner;
    int loser;
}
pair;

// Array of candidates
string candidates[MAX];
pair pairs[MAX * (MAX - 1) / 2];

int pair_count;
int candidate_count;

// Function prototypes
bool vote(int rank, string name, int ranks[]);
void record_preferences(int ranks[]);
void add_pairs(void);
void sort_pairs(void);
void lock_pairs(void);
void print_winner(void);
bool check_loop(int current_node, int starting_node);

int main(int argc, string argv[])
{
    // Check for invalid usage
    if (argc < 2)
    {
        printf("Usage: tideman [candidate ...]\n");
        return 1;
    }

    // Populate array of candidates
    candidate_count = argc - 1;
    if (candidate_count > MAX)
    {
        printf("Maximum number of candidates is %i\n", MAX);
        return 2;
    }
    for (int i = 0; i < candidate_count; i++)
    {
        candidates[i] = argv[i + 1];
    }

    // Clear graph of locked in pairs
    for (int i = 0; i < candidate_count; i++)
    {
        for (int j = 0; j < candidate_count; j++)
        {
            locked[i][j] = false;
        }
    }

    pair_count = 0;
    int voter_count = get_int("Number of voters: ");

    // Clear preferences
    for (int i = 0; i < MAX; i++)
    {
        for (int j = 0; j < MAX; j++)
        {
            preferences[i][j] = 0;
        }
    }

    // Query for votes
    for (int i = 0; i < voter_count; i++)
    {
        // ranks[i] is voter's ith preference
        int ranks[candidate_count];

        // Query for each rank
        for (int j = 0; j < candidate_count; j++)
        {
            string name = get_string("Rank %i: ", j + 1);

            if (!vote(j, name, ranks))
            {
                printf("Invalid vote.\n");
                return 3;
            }
        }

        record_preferences(ranks);

        printf("\n");
    }

    add_pairs();
    sort_pairs();
    lock_pairs();
    print_winner();
    return 0;
}

// Update ranks given a new vote
bool vote(int rank, string name, int ranks[])
{
    // Look for candidate called name
    // If candidate found, put his name's ID in the corresponding ballot's rank
    for (int i = 0; i < candidate_count; i++)
    {
        if (!strcmp(name, candidates[i]))
        {
            ranks[rank] = i;
            return true;
        }
    }
    // If no candidate found don't update any rank and return false
    return false;
}

// Update preferences given one voter's ranks
void record_preferences(int ranks[])
{
    for (int i = 0; i <= candidate_count; i++)
    {
        for (int j = i + 1; j < candidate_count; j++)
        {
            preferences[ranks[i]][ranks[j]] += 1;
        }
    }
    return;
}

// Record pairs of candidates where one is preferred over the other
void add_pairs(void)
{
    // Initialize parsing half of preferences tab
    for (int i = 0; i < candidate_count; i++)
    {
        for (int j = i + 1; j < candidate_count; j++)
        {
            //Check if a pair has a winner and assign the IDs to winner and loser variable of the pair from the pairs array
            if (preferences[i][j] > preferences[j][i])
            {
                pairs[pair_count].winner = i;
                pairs[pair_count].loser = j;
                pair_count++;
            }
            else if (preferences[i][j] < preferences[j][i])
            {
                pairs[pair_count].winner = j;
                pairs[pair_count].loser = i;
                pair_count++;
            }
        }
    }
    return;
}

// Sort pairs in decreasing order by strength of victory
void sort_pairs(void)
{
    pair tmp;

    // Parse through the array of pairs to progress as the highest victory strength of the sub array is found and swapped
    for (int i = 0; i <= pair_count; i++)
    {
        int highest = i;
        // Check in the sub array if there's a higher winner than the current position's winner, in that case keep track of this highest position
        for (int j = i; j < pair_count; j++)
        {
            if (preferences[pairs[j].winner][pairs[j].loser] > preferences[pairs[highest].winner][pairs[highest].loser])
                highest = j;
        }
        // If the a higher winnner have been found compared to current position, swap it with the current position
        if (highest != i)
        {
            tmp = pairs[i];
            pairs[i] = pairs[highest];
            pairs[highest] = tmp;
        }
    }
    return;
}

// Lock pairs into the candidate graph in order, without creating cycles
void lock_pairs(void)
{
    // Starting from the highest winner, to to lowest, draw the edges of who wins over who
    for (int i = 0; i < pair_count; i++)
    {
        // Check if target node leads to a loop, if not draw the edge on locked array, otherwise don't draw it and moe ot next pair
        if(!check_loop(pairs[i].loser, pairs[i].winner))
        {
            locked[pairs[i].winner][pairs[i].loser] = true;
        }
    }
    return;
}

// Return true if locked array graph loops
bool check_loop(int current_node, int starting_node)
{
    // Check if current node is starting node, if yes there's a cycle
    if (current_node == starting_node)
        return true;
    // Check each edge of current node
    for (int i = 0; i < candidate_count; i++)
    {
        if (locked[current_node][i]) //
        {
            // If the current_node have an edge, do recursive call to explore it further and if this leads to no loop check other possible edges
            if (check_loop(i, starting_node))
                return true;
        }
    }

    // If after checking all possible edge no loop has been found resulting in a true return earlier then return false
    return false;
}

// Print the winner of the election
void print_winner(void)
{
    // Check line by line the locked array, a line means a candidate
    for (int i = 0; i < candidate_count; i++)
    {
        // Set the tested candidate winner by default, then check throught the line if an edge ever leads to him
        bool is_winner = true;
        for (int j = 0; j < candidate_count; j++)
        {
            // If an edge leading to the tested cadndidate is found, set as no longer winner
            if (locked[j][i])
                is_winner = false;
        }
        // If after checking the whole line, no edge has been found is winner is true as initialized and print his name
        if (is_winner)
            printf("%s\n", candidates[i]);
    }
    return;
}