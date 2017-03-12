#define _CRT_SECURE_NO_WARNINGS

#include "diff.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_LENGTH 0x400


bool hash_cmp(StrHash_t *shash1, StrHash_t *shash2)
{
	return shash1->hash < shash2->hash;
}


bool line_cmp(StrHash_t *shash1, StrHash_t *shash2)
{
	return shash1->line < shash2->line;
}


int str_cmp(StrHash_t *shash1, StrHash_t *shash2, FILE* file)
{
	char str1[MAX_LENGTH + 1], str2[MAX_LENGTH + 1];

	fseek(file, shash1->offset, SEEK_SET);
	fread(str1, sizeof(str1[0]), shash1->len, file);

	fseek(file, shash2->offset, SEEK_SET);
	fread(str2, sizeof(str2[0]), shash2->len, file);

	str1[shash1->len] = 0;
	str2[shash2->len] = 0;

	return strcmp(str1, str2);
}


void merge(StrHash_t *arr, StrHash_t *lArr, size_t leftCount, StrHash_t *rArr, size_t rightCount, bool(*cmp)(StrHash_t*, StrHash_t*))
{
	size_t i = 0, j = 0, k = 0;

	while (i < leftCount && j < rightCount)
	{
		if (cmp(lArr + i, rArr + j))
			arr[k++] = lArr[i++];
		else
			arr[k++] = rArr[j++];
	}

	while (i < leftCount)
		arr[k++] = lArr[i++];

	while (j < rightCount)
		arr[k++] = rArr[j++];
}


void merge_sort(StrHash_t *arr, size_t count, bool(*cmp)(StrHash_t*, StrHash_t*))
{
	size_t mid, i;
	StrHash_t *lArr, *rArr;

	if (count < 2) return;

	mid = count / 2;

	lArr = (StrHash_t*)malloc(mid*sizeof(StrHash_t));
	rArr = (StrHash_t*)malloc((count - mid)*sizeof(StrHash_t));

	for (i = 0; i < mid; i++)
		lArr[i] = arr[i];

	for (i = mid; i < count; i++)
		rArr[i - mid] = arr[i];

	merge_sort(lArr, mid, cmp);
	merge_sort(rArr, count - mid, cmp);
	merge(arr, lArr, mid, rArr, count - mid, cmp);

	free(lArr);
	free(rArr);
}


size_t count_lines(const char* fileName)
{
	FILE *file;
	size_t count = 0;
	char ch;

	file = fopen(fileName, "rb");
	if (!file)
		return -1;

	do {
		ch = fgetc(file);
		if (ch == 0xA) count++;
	} while (ch != EOF);

	fclose(file);

	return count;
}


unsigned long hash(const char* data, size_t len)
{
	unsigned long rez = 0;
	unsigned long *pData = (unsigned long*)data;
	size_t i = len / 4;

	while (i--)
		rez += pData[i];
	for (i = (len / 4) * 4; i < len; i++)
		rez += data[i] << ((i % 4) * 8);

	return rez ^ 0xDEADBEEF;
}


__inline long read_line(char* str, unsigned long maxCount, FILE* file, long offset)
{
	unsigned long count = 0;

	fseek(file, offset, SEEK_SET);
	*str = 0;

	fgets(str, maxCount, file);
	while (str[count++] != 0xA);
	str[count] = 0;

	return count;
}


StrHash_t* gen_strhash(const char* fileName, size_t *count)
{
	FILE *file;
	StrHash_t *hashes;
	size_t i, len;
	unsigned long offset = 0;
	char str[MAX_LENGTH + 1];

	*count = count_lines(fileName);
	if (*count == -1)
		return NULL;

	hashes = (StrHash_t*)malloc(*count*sizeof(StrHash_t));

	file = fopen(fileName, "rb");

	if (!file)
		return NULL;

	for (i = 0; i < *count; i++)
	{
		len = read_line(str, MAX_LENGTH, file, offset);
		hashes[i].file = fileName;
		hashes[i].offset = offset;
		hashes[i].len = len;
		hashes[i].line = i;
		hashes[i].hash = hash(str, len);
		offset += len;
	}

	fclose(file);

	return hashes;
}


StrHash_t* difference(StrHash_t* hashes1, size_t count1, StrHash_t* hashes2, size_t count2, size_t* count)
{
	StrHash_t *outHashes, *cur1, *cur2, *end1, *end2, *cur;

	*count = 0;
	outHashes = (StrHash_t*)malloc((count1 + count2)*sizeof(StrHash_t));
	if (!outHashes)
		return NULL;

	cur1 = hashes1; end1 = hashes1 + count1;
	cur2 = hashes2; end2 = hashes2 + count2;
	cur = outHashes;

	while ((cur1 != end1) && (cur2 != end2))
	{
		if (cur1->hash < cur2->hash)
		{
			memcpy(cur, cur1, sizeof(StrHash_t));
			cur1++;
			cur++;
			(*count)++;
		}
		else if (cur2->hash < cur1->hash)
		{
			memcpy(cur, cur2, sizeof(StrHash_t));
			cur2++;
			cur++;
			(*count)++;
		}
		else if (cur1->hash == cur2->hash)
		{
			cur1++;
			cur2++;
		}
	}

	while (cur1 != end1)
	{
		memcpy(cur, cur1, sizeof(StrHash_t));
		cur1++;
		cur++;
		(*count)++;
	}

	while (cur2 != end2)
	{
		memcpy(cur, cur2, sizeof(StrHash_t));
		cur2++;
		cur++;
		(*count)++;
	}

	outHashes = (StrHash_t*)realloc(outHashes, *count*sizeof(StrHash_t));
	merge_sort(outHashes, *count, line_cmp);

	return outHashes;
}


int output(const char* fileName, StrHash_t* outHashes, size_t count)
{
	FILE *outFile, *inFile;
	StrHash_t *cur, *end;
	char str[MAX_LENGTH + 1];

	outFile = fopen(fileName, "wb");

	if (!outFile)
		return -1;

	if (count == 0)
	{
		fprintf(outFile, "Files have equal content");
		fclose(outFile);
		return 0;
	}

	cur = outHashes; end = outHashes + count;

	while (cur != end)
	{
		if (!(cur->line & IS_DUPLICATE))
		{
			inFile = fopen(cur->file, "rb");
			fseek(inFile, cur->offset, SEEK_CUR);
			fread(str, sizeof(str[0]), cur->len, inFile);
			str[cur->len] = 0;
			fclose(inFile);

			fprintf(outFile, "%s %d %s", cur->file, cur->line + 1, str);
		}
		cur++;
	}

	fclose(outFile);

	printf("Result is in \"%s\" file", fileName);

	return 0;
}


int diff(const char* file1, const char* file2, const char* outFile)
{
	size_t count1 = 0, count2 = 0, count = 0;
	StrHash_t *hashes1, *hashes2, *outHashes;

	hashes1 = gen_strhash(file1, &count1);
	hashes2 = gen_strhash(file2, &count2);

	if (!(hashes1 && hashes2))
		return -1;

	merge_sort(hashes1, count1, &hash_cmp);
	merge_sort(hashes2, count2, &hash_cmp);

	outHashes = difference(hashes1, count1, hashes2, count2, &count);
	if (!outHashes)
		return -1;

	output(outFile, outHashes, count);

	free(hashes1);
	free(hashes2);
	free(outHashes);

	return 0;
}
