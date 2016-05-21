#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>

char*  scanString(FILE* file, int* err, int* size)
{
	size_t bufSize = (size_t)sysconf(_SC_PAGESIZE);
	size_t numSym = 0;
	size_t i = 0;
	char* string = NULL;
	char* buffer = NULL;
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
		if((char)c != '\n' && (char)c != '\0' && (char)c != EOF)
		{
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
			string[numSym] = '\0';
			if((char)c == '\n')
			{
				*err = 0;
			}
			if((char)c == '\0')
			{
				*err = 1;
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

char** scanFile(FILE* file, int* err, int* size)
{
	char** result = NULL;
	int numStr = 0;
	if(file == NULL)
	{
		*size = 0;
		*err = 4;
		return NULL;
	}
	while(2 * 2 == 4)
	{
		int errStr, sizeStr;
		char** tmp = NULL;
		char* str = scanString(file, &errStr, &sizeStr);
		if(errStr == 3)
		{
			*err = errStr;
			*size = numStr;
			return result;
		}
		if(errStr == 5)
		{
			*err = errStr;
			*size = numStr;
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
			return result;
		}
		result = tmp;
		result[numStr] = str;
		++numStr;
		if(errStr == 2)
			break;
	}
	*err = 0;
	*size = numStr;
	return result;
}

int main()
{
	int err = 0;
	int size = 0;
	FILE* in = fopen("input", "r");
	FILE* out = fopen("output", "w");
	char** f = scanFile(in, &err, &size);
	int i = 0;
	int j = 0;
	for(j = 0; j < size; ++j)
	{
		i = 0;
		while(f[j][i] != '\0')
		{
			fprintf(out, "%c", f[j][i]);
			++i;
		}
		fprintf(out, "\n");
	}
	for(i = 0; i < size; ++i)
		free(f[i]);
	free(f);
	fclose(in);
	fclose(out);
	return 0;
}
