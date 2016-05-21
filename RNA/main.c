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
			if(c == EOF)
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

int code(char c)
{
	if(c == '\0')
		return 0;
	if(c == 'a' || c == 'A')
		return 1;
	if(c == 't' || c == 'T')
		return 2;
	if(c == 'g' || c == 'G')
		return 3;
	if(c == 'c' || c == 'C')
		return 4;
	if(c == 'u' || c == 'U')
		return 5;
	return -1;
}

char decode(int p)
{
	if(p == 1)
		return 'a';
	if(p == 2)
		return 't';
	if(p == 3)
		return 'g';
	if(p == 4)
		return 'c';
	if(p == 5)
		return 'u';
	return '\0';
}

char* getDNA(FILE* file, int* err, int* size)
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
		if(c == '\n' || c == '\0' || (c > 0 && c <= 127 && c != 'a' && c != 'A' && c != 't' && c != 'T' && c != 'g' && c != 'G'  && c != 'c'&& c != 'C' && c != 'u' && c != 'U' && c != '>'))
				continue;
		if((char)c != '>' && (char)c != EOF)
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
			if((char)c == '>')
			{
				*err = 0;
			}
			if(c == EOF)
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

int pack(FILE* in, FILE* out)
{
	int c = 0;
	if(in == NULL || out == NULL)
	{
		perror("Bad file");
		return 4;
	}
	c = fgetc(in);
	while((char)c != '>' && (char)c != EOF)
	{
		c = fgetc(in);
	}
	while(2 * 2 == 4)
	{
		char* name = NULL;
		char* dna = NULL;
		int err = 0;
		int size = 0;
		int errDna = 0;
		int sizeDna = 0;
		int numInt = 0;
		int i = 0;
		int j = 0;
		name = scanString(in, &err, &size);
		if(err == 2)
		{
			if(name != NULL)
				free(name);
			perror("bad file format");
			return 2;
		}
		if(err == 3)
		{
			if(name != NULL)
				free(name);
			perror("not enough memory");
			return 3;
		}
		if(err == 4)
		{
			if(name != NULL)
				free(name);
			perror("file error");
			return 4;
		}
		if(err == 5)
		{
			if(name != NULL)
				free(name);
			perror("Bad file");
			return 5;
		}
		fprintf(out, ">");
		fwrite(name, 1, size, out);
		fprintf(out, "\n");
		dna = getDNA(in, &errDna, &sizeDna);
		if(errDna == 3)
		{
			if(dna != NULL)
				free(dna);
			perror("not enough memory");
			return 3;
		}
		if(errDna == 4)
		{
			if(dna != NULL)
				free(dna);
			perror("file error");
			return 4;
		}
		numInt = sizeDna / 10 + 1;
		fwrite(&numInt, 4, 1, out);
		for(i = 0; i < numInt; ++i)
		{
			unsigned int ans = 0;
			for(j = 0; j < 10 && i * 10 + j <= sizeDna; ++j)
			{
                char nuc = dna[i * 10 + j];
                int cd = code(nuc);
                cd = (cd << (29 - j * 3));
                ans = ans | cd;
			}
			fwrite(&ans, 4, 1, out);
		}
		fprintf(out, "\n");
		if(name != NULL)
			free(name);
		if(dna != NULL)
			free(dna);
		if(errDna == 2)
			break;
	}
	return 0;
}

int unpack(FILE* in, FILE* out)
{
	while(2 * 2 == 4)
	{
		char* name = NULL;
		int err = 0;
		int size = 0;
		int numInt = 0;
		int i = 0;
		int j = 0;
		char flag = 1;
		name = scanString(in, &err, &size);
		if(size == 0 && err == 2)
		{
			if(name != NULL)
				free(name);
			break;
		}
		if(err == 2)
		{
			if(name != NULL)
				free(name);
			perror("bad file format");
			return 2;
		}
		if(err == 3)
		{
			if(name != NULL)
				free(name);
			perror("not enough memory");
			return 3;
		}
		if(err == 4)
		{
			if(name != NULL)
				free(name);
			perror("file error");
			return 4;
		}
		if(err == 5)
		{
			if(name != NULL)
				free(name);
			perror("bad file");
			return 5;
		}
		fwrite(name, 1, size, out);
		fprintf(out, "\n");
		fread(&numInt, 4, 1, in);
		for(i = 0; i < numInt; ++i)
		{
			unsigned int block = 0;
			fread(&block, 4, 1, in);
			if(i % 6 == 0 && i != 0)
				fprintf(out, "\n");
			for(j = 0; j < 10; ++j)
			{
				int code = (block & (7<<(29 - j * 3)))>>(29 - j * 3);
				char nuc = decode(code);
				if(nuc == '\0')
				{
					flag = 0;
					break;
				}
				else
				{
					fprintf(out, "%c", nuc);
				}
			}
			if(flag == 1)
				fprintf(out, " ");
			if(flag == 0)
				break;
		}
		free(scanString(in, &err, &size));
		fprintf(out, "\n");
		free(name);
	}
	return 0;
}

int main(int argc, char** argv)
{
	FILE* in;
	FILE* out;
	int i = 1;
	char* direc = NULL;
	if(argc < 7)
	{
		perror("Too few parametrs");
		return 0;
	}
	while(i < argc)
	{
		if(argv[i][0] == '-' && argv[i][1] == 'i' && argv[i][2] == '\0')
		{
			++i;
			in = fopen(argv[i], "r");
		}
		else
			if(argv[i][0] == '-' && argv[i][1] == 'o' && argv[i][2] == '\0')
			{
				++i;
				out = fopen(argv[i], "w");
			}
			else
				if(argv[i][0] == '-' && argv[i][1] == 'd' && argv[i][2] == '\0')
				{
					++i;
					direc = argv[i];
				}
				else
				{
					perror("Bad parametrs format, see README file");
					return 0;
				}
		++i;
	}
	if(direc[0] == 'p' && direc[1] == 'a' && direc[2] == 'c' && direc[3] == 'k' && direc[4] == '\0')
	{
		pack(in, out);
	}
	else
		if(direc[0] == 'u' && direc[1] == 'n' && direc[2] == 'p' && direc[3] == 'a' && direc[4] == 'c' && direc[5] == 'k' && direc[6] == '\0')
		{
			unpack(in, out);
		}
		else
			perror("Bad direction\n");
	if(in != NULL)
		fclose(in);
	if(out != NULL)
		fclose(out);
	return 0;
}
