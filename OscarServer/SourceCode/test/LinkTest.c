#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct Node
{
	char* content;
	struct Node* next;
};



void TestLink()
{
	struct Node* cache_mem = (struct Node*)malloc(10 * sizeof(struct Node));
	memset(cache_mem, 0, 10 * sizeof(struct Node));

	struct Node* free_list = NULL;

	for (size_t i = 0; i < 10; i++)
	{
		cache_mem[i].next = free_list;
		cache_mem[i].content = "oscar=";
		free_list = &cache_mem[i];
	}


	struct Node* walk = free_list;

	while (walk)
	{
		printf("Node content = %s\n", walk->content);
		walk = walk->next;
	}

	printf("sdf");
}