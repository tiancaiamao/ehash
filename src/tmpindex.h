#ifndef TMP_INDEX_H
#define TMP_INDEX_H

struct posting
{
	struct posting *next;	/* link postings_node together*/
	struct posting *hashnext; /* overflow list in hash */
	char *c;			/* concept */
	unsigned int count;
	unsigned int lastdoc;
	struct vector *invertedlist;
};
struct tmpindex
{
	struct posting **hash;
	struct posting *list;
	unsigned int slots;
	unsigned int count;	/* count of postings node */
	unsigned int largest;
};

struct tmpindex* tmpindex_new(unsigned int);
int tmpindex_insert(struct tmpindex* idx,const char *c,unsigned int d,float w);
struct posting* tmpindex_find(struct tmpindex *idx,const char *c);
void tmpindex_delete(struct tmpindex *idx);
void tmpindex_clear(struct tmpindex *idx);
void tmpindex_free(struct tmpindex *idx);
int tmpindex_delete_postings(struct tmpindex *idx,char *c,unsigned int d);
void posting_free(struct posting *p);
#endif
