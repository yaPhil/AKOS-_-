#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<ctype.h>

char*  scanString(FILE* file, int* err, int* size, int* numWords)
{
	size_t bufSize = (size_t)sysconf(_SC_PAGESIZE);
	size_t numSym = 0;
	size_t i = 0;
	int flag = 0;
	char* string = NULL;
	char* buffer = NULL;
	*numWords = 0;
	if(file == NULL)
	{
		*err = 4;
		*size = 0;
		return NULL;
	}
	string = malloc(bufSize);
	if(string == NULL)
	{
		*err = 3;
		*size = 0;
		return NULL;
	}	
	buffer = malloc(bufSize);
	if(buffer == NULL)
	{
		*err = 3;
		free(string);
		return NULL;
	}
	while(2*2 == 4)
	{
		int c = fgetc(file);
		if((c > 255 || c < 0) && c != EOF && c != '\0' && c != '\n')
		{
			*err = 5;
			*size = 0;
			free(string);
			free(buffer);
			return NULL;
		}
		if((char)c != '\n' && (char)c != EOF)
		{
			if(isspace((char)c) && flag == 1)
			{
				++*numWords;
				flag = 0;
			}
			else
			{
				if(flag == 0 && isspace((char)c) == 0)
				{
					flag = 1;
				}
			}
			buffer[i] = c; 
			++i;
			if(i == bufSize)
			{
				char* tmp = NULL;
				memmove(string + numSym, buffer, bufSize);
				i = 0;
				numSym += bufSize;
				tmp = realloc(string, numSym + bufSize);
				if(tmp == NULL)
				{
					*err = 3;
					break;
				}
				else
					string = tmp;
			}
		}
		else
		{
			memmove(string + numSym, buffer, i);
			numSym += i;
			string[numSym] = '\n';
			if(flag == 1)
			{
				++*numWords;
			}
			if((char)c == '\n')
			{
				*err = 0;
			}
			if((char)c == EOF)
			{
				*err = 2;
			}
			break;
		}
	}
	free(buffer);
	*size = numSym;
	return string;
}

char** scanFile(FILE* file, int* err, int* size, int* numWords, int* numSymb)
{
	char** result = NULL;
	int numStr = 0;
	if(file == NULL)
	{
		*size = 0;
		*err = 4;
		return NULL;
	}
	*numWords = 0;
	*numSymb = 0;
	while(2 * 2 == 4)
	{
		int errStr, sizeStr, wordsStr;
		char** tmp = NULL;
		char* str = scanString(file, &errStr, &sizeStr, &wordsStr);
		if(errStr == 3)
		{
			*err = errStr;
			*size = numStr;
			free(str);
			return result;
		}
		if(errStr == 5)
		{
			*err = errStr;
			*size = numStr;
			free(str);
			return result;
		}
		if(errStr == 2 && sizeStr == 0)
		{
			free(str);
			break;
		}
		tmp = realloc(result, (numStr + 1) * sizeof(char*));
		if(tmp == NULL)
		{
			*err = 3;
			*size = numStr;
			free(str);
			return result;
		}
		result = tmp;
		result[numStr] = str;
		++numStr;
		*numWords += wordsStr;
		*numSymb += sizeStr;
		if(errStr == 2)
			break;
	}
	*err = 0;
	*size = numStr;
	return result;
}

void freeFile(FILE* in, char** file, int size)
{
	int i = 0;
	for(i = 0; i < size; ++i)
		free(file[i]);
	free(file);
	if(in != NULL)
		fclose(in);
	return;
}

void swap(char** a, char** b)
{
	char* tmp = *a;
	*a = *b;
	*b = tmp;
	return;
}

int cmpString(char* a, char* b)
{
	int i = 0;
	while(a[i] != '\n' && b[i] != '\n')
	{
		if(a[i] < b[i])
		{
			return -1;
		}
		if(b[i] < a[i])
		{
			return 1;
		}
		++i;
	}
	if(a[i] == '\n' && b[i] == '\n')
	{
		return 0;
	}
	if(a[i] == '\n')
	{
		return -1;
	}
	else
	{
		return 1;
	}
}

void sort(char** file, int size)
{
	int i = 0;
	int j = 0;
	for(i = 0; i < size - 1; ++i)
	{
		for(j = 0; j < size - 1; ++j)
		{
			if(cmpString(file[j], file[j + 1]) == 1)
			{
				swap(&file[j], &file[j + 1]);
			}
		}
	}
}

int main(int argc, char** argv)
{
	int err, strSize, wordsSize, symSize;
	char** file;
	FILE* in;
	int i;
	int j = 0;
	if(argc != 2)
	{
		perror("Bad arguments\n");
		exit(1);
	}
	in = fopen(argv[1], "r");
	file = scanFile(in, &err, &strSize, &wordsSize, &symSize);
	if(err == 3)
	{
		perror("Bad alloc\n");
		freeFile(in, file, strSize);
		exit(3);
	}
	if(err == 4)
	{
		perror("Bad input file\n");
		freeFile(in, file, strSize);
		exit(4);
	}
	if(err == 5)
	{
		perror("Bad file format\n");
		freeFile(in, file, strSize);
		exit(5);
	}
	printf("Number of strings: %d\nNumber of words: %d\nNumber of symbols: %d\n", strSize, wordsSize, symSize + strSize);
	sort(file, strSize);
	for(i = 0; i < strSize; ++i)
	{
		j = 0;
		while(file[i][j] != '\n')
		{
			printf("%c", file[i][j]);
			++j;
		}
		printf("\n");
	}
	freeFile(in, file, strSize);
	return 0;
}
