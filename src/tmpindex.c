#include "hash.h"
#include "tmpindex.h"
#include "vector.h"

#include <string.h>
#include <assert.h>
#include <stdlib.h>

struct tmpindex* tmpindex_new(unsigned int size)
{
	struct tmpindex* idx = malloc(sizeof(struct tmpindex));
	if(idx == NULL)
		goto exit;
	idx->slots = size;
	idx->count = 0;
	idx->largest = 0;
	idx->list = NULL;
	idx->hash = (struct posting**)malloc(idx->slots * sizeof(struct posting*));
	if(idx->hash == NULL)
		goto free;
	memset(idx->hash,0,idx->slots * sizeof(struct posting*));
	return idx;
free:
	free(idx);
exit:
	return NULL;
}

struct posting* posting_alloc(const char *c,unsigned int d,float w)
{
	struct posting *p = malloc(sizeof(struct posting));
	if(p == NULL)
		goto exit;
	p->c = malloc(strlen(c)+1);
	if(p->c == NULL)
		goto free_posting;
	strcpy(p->c,c);
	p->invertedlist = vector_new(16);
	if(p->invertedlist == NULL)
		goto free_string;

	if(!vector_put_vbyte(p->invertedlist,d))
		goto free_invertedlist;
	if(!vector_put_float(p->invertedlist,w))
		goto free_invertedlist;

	p->count = 1;
	p->lastdoc = d;
	p->hashnext = NULL;
	p->next = NULL;

	return p;
free_invertedlist:
	vector_free(p->invertedlist);
free_string:
	free(p->c);
free_posting:
	free(p);
exit:
	return NULL;
}

void posting_free(struct posting *p)
{
	free(p->c);
	vector_free(p->invertedlist);
	free(p);
}

int posting_insert(struct posting *p,unsigned int d,float w)
{
	assert(d > p->lastdoc);

	if(!vector_put_vbyte(p->invertedlist,d-p->lastdoc))
		return -1;
	p->lastdoc = d;
	if(!vector_put_float(p->invertedlist,w))
		return -1;
	p->count++;
	return 0;
}

int tmpindex_insert(struct tmpindex *idx,const char *c,unsigned int d,float w)
{
	unsigned int entry;
	struct posting *p;

	if(strcmp(c,"") == 0)
		return -1;
	entry = jshash(c,idx->slots);
	if(idx->hash[entry] == NULL) /* hash list is empty,insert to it */
	{
		p = posting_alloc(c,d,w);
		if(p == NULL)
			return -1;

		idx->hash[entry] = p; /* add to hashlist */
		p->next = idx->list;  /* add to list */
		idx->list = p;
		idx->count++;
		if(idx->largest < p->invertedlist->len)
			idx->largest = p->invertedlist->len;
	}
	else			/* find c in the hash list */
	{
		for(p = idx->hash[entry]; p; p = p->hashnext)
		{
			if(strcmp(p->c,c) == 0)
			{
				if(posting_insert(p,d,w) != 0)
					return -1;
				if(idx->largest < p->invertedlist->len)
					idx->largest = p->invertedlist->len;
				return 0;
			}
		}
		if(p == NULL)	/* not in hashlist */
		{
			p = posting_alloc(c,d,w);
			if(p == NULL)
				return -1;
			p->hashnext = idx->hash[entry]; /* add to hashlist */
			idx->hash[entry] = p; 
			p->next = idx->list; /* add to list */
			idx->list = p;
			idx->count++;
			if(idx->largest < p->invertedlist->len)
				idx->largest = p->invertedlist->len;
		}
	}
	return 0;
}

struct posting* tmpindex_find(struct tmpindex *idx,const char *c)
{
	struct posting *ret;
	unsigned int entry = jshash(c,idx->slots);

	ret = idx->hash[entry];
	while(ret)
	{
		if(strcmp(c,ret->c) == 0)
			break;
		ret = ret->hashnext;
	}
	return ret;
}

void tmpindex_clear(struct tmpindex *idx)
{
	struct posting *p,*save;

	p = idx->list;
	while(p)		/* free postings_node in tmpindex */
	{
		save = p->next;
		posting_free(p);
		p = save;
	}
	idx->list = NULL;
	idx->count = 0;
	idx->largest = 0;
	memset(idx->hash,0,idx->slots * sizeof(struct posting*));
}

void tmpindex_free(struct tmpindex *idx)
{
	tmpindex_clear(idx);
	free(idx->hash);
	free(idx);
}

void tmpindex_dump()
{
//the most importent function in tmpindex...
}


#ifdef TMPINDEX_TEST

/* this is written for unit test */
#include <stdio.h>

void posting_print(struct posting *p)
{
	unsigned int i;
	unsigned int val;
	unsigned int lastval;
	float w;

	printf("%s,%d ",p->c,p->count);
	vector_reset_pos(p->invertedlist);

	if(!vector_get_uint(p->invertedlist,&lastval))
	{
		printf("a error happened\n");
		return;
	}
	if(!vector_get_float(p->invertedlist,&w))
	{
		printf("a error happen");
		return;
	}
	printf("<%d,%f> ",lastval,w);


	for(i=1; i<p->count; i++)
	{
		if(!vector_get_vbyte(p->invertedlist,&val))
		{
			printf("a error happen");
			return;
		}
		if(!vector_get_float(p->invertedlist,&w))
		{
			printf("a error happen");
			return;
		}

		lastval += val;
		printf("<%d,%f> ",lastval,w);
	}
	printf("\n");
}
void tmpindex_print(struct tmpindex *idx)
{
	unsigned int i;
	struct posting *p;

	for(i=0; i < idx->slots; i++)
	{
		if(idx->hash[i])
		{
			printf("slot:%d\n",i);
			for(p=idx->hash[i]; p!=NULL; p=p->hashnext)
			{
				printf("\t");
				posting_print(p);
			}
		}
		else
		{
			printf("slot %d is empty\n",i);
		}
	}
}
int main()
{
	struct tmpindex *idx = tmpindex_new(10);
	struct posting *p;

	tmpindex_insert(idx,"abc",0,1);
	tmpindex_insert(idx,"abm",0,1);
	tmpindex_insert(idx,"bbc",0,1);
	tmpindex_insert(idx,"adfg",0,1);
	tmpindex_insert(idx,"ohgi",0,1);
	tmpindex_insert(idx,"aglp",0,1);
	tmpindex_print(idx);
/*	tmpindex_insert(idx,"hkpg",0,1);
	tmpindex_insert(idx,"ado",0,1);
	tmpindex_insert(idx,"abc",1,1);
	tmpindex_insert(idx,"bbc",1,1);
	tmpindex_insert(idx,"bbc",2,1);
*/
	tmpindex_insert(idx,"adfg",3,1);
	tmpindex_insert(idx,"bbc",6,3);
	p = tmpindex_find(idx,"bbc");
	posting_print(p);

	tmpindex_print(idx);

	tmpindex_clear(idx);	
	tmpindex_print(idx);
	tmpindex_free(idx);
	return 0;
}
#endif
