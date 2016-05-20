#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<termios.h>
#include<unistd.h>
#include<signal.h>
#include<sys/ioctl.h>
#include<locale.h>
#include<wchar.h>

const int KEY_LEFT = 4479771;
const int KEY_UP = 4283163;
const int KEY_RIGHT = 4414235;
const int KEY_DOWN = 4348699;

volatile int numRows = 0;
volatile int numColumns = 0;
volatile int firstRow = 0;
volatile int lastRow = 0;
volatile int firstColumn = 0;
volatile int lastColumn = 0;
volatile int* strLen = NULL;
volatile int maxLength = 0;
volatile wchar_t** file = NULL;
volatile int sizeOfFile = 0;
int* overHead = NULL;
int f = 0;



struct termios oldTermParam;

int reDraw(wchar_t** from, int fR, int lR, int fC, int numColumns, int* len, int* mxLen, int* overHead, int f);

void handler(int sig)
{
	if(sig == SIGWINCH)
	{
		struct winsize newWin;
		int err = ioctl(0, TIOCGWINSZ, &newWin);
		numRows = newWin.ws_row;
		numColumns = newWin.ws_col;
		if(numRows < sizeOfFile - firstRow)
			lastRow = numRows - 1 + firstRow;
		else
		{
			lastRow = sizeOfFile;
			if(numRows > sizeOfFile)
				firstRow = 0;
			else
				firstRow = sizeOfFile - numRows;
		}
		lastColumn = reDraw(file, firstRow, lastRow, firstColumn, numColumns, strLen, &maxLength, overHead, f);
		signal(SIGWINCH, handler);
	}
}

wchar_t*  scanString(FILE* file, int* err, int* size)
{
	size_t bufSize = (size_t)sysconf(_SC_PAGESIZE);
	size_t numSym = 0;
	size_t i = 0;
	wchar_t* string = NULL;
	wchar_t* buffer = NULL;
	if(file == NULL)
	{
		*err = 4;
		*size = 0;
		return NULL;
	}
	string = malloc(bufSize * sizeof(wchar_t));
	if(string == NULL)
	{
		*err = 3;
		*size = 0;
		return NULL;
	}	
	buffer = malloc(bufSize * sizeof(wchar_t));
	if(buffer == NULL)
	{
		*err = 3;
		*size = 0;
		free(string);
		return NULL;
	}
	while(2*2 == 4)
	{
		wint_t c = fgetwc(file);

		if((wchar_t)c != L'\n' && (wchar_t)c != L'\0' && (wchar_t)c != WEOF)
		{
			if((wchar_t)c != L'\t')
			{
				buffer[i] = c; 
				++i;
			}
			else
			{
				int space = 4 - ((numSym + i) % 4);
				int j = 0;
				for(j = 0; j < space; ++j)
				{
					buffer[i] = L' ';
					++i;
					if(i == bufSize)
					{
						wchar_t* tmp = NULL;
						wmemmove(string + numSym, buffer, bufSize);
						i = 0;
						numSym += bufSize;
						tmp = realloc(string, numSym * sizeof(wchar_t) + bufSize * sizeof(wchar_t));
						if(tmp == NULL)
						{
							*err = 3;
							break;
						}
						else
							string = tmp;
					}
				}
			}
			if(i == bufSize)
			{
				wchar_t* tmp = NULL;
				wmemmove(string + numSym, buffer, bufSize);
				i = 0;
				numSym += bufSize;
				tmp = realloc(string, numSym * sizeof(wchar_t) + bufSize * sizeof(wchar_t));
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
			wmemmove(string + numSym, buffer, i);
			numSym += i;
			string[numSym] = L'\0';
			if((wchar_t)c == L'\n')
			{
				*err = 0;
			}
			if((wchar_t)c == L'\0')
			{
				*err = 1;
			}
			if((wchar_t)c == WEOF)
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

wchar_t** scanFile(FILE* file, int* err, int* size)
{
	wchar_t** result = NULL;
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
		wchar_t** tmp = NULL;
		wchar_t* str = scanString(file, &errStr, &sizeStr);
		if(errStr == 4)
		{
			if(str != NULL)
				free(str);
			*err = errStr;
			*size = numStr;
		}
		if(errStr == 3)
		{
			if(str != NULL)
				free(str);
			*err = errStr;
			*size = numStr;
			return result;
		}
		if(errStr == 5)
		{
			if(str != NULL)
				free(str);
			*err = errStr;
			*size = numStr;
			return result;
		}
		if(errStr == 2 && sizeStr == 0)
		{
			if(str != NULL)
				free(str);
			break;
		}
		tmp = realloc(result, (numStr + 1) * sizeof(wchar_t*));
		if(tmp == NULL)
		{
			if(str != NULL)
				free(str);
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

void setNoncononical()
{
	int err;
	struct termios newTermParam = oldTermParam;
	newTermParam.c_lflag &= ~(ICANON);
	newTermParam.c_lflag &= ~(ECHO);
	newTermParam.c_cc[VMIN] = 1;
	/*printf("noncocnonical%d\n", newTermParam.c_lflag);*/
	err = tcsetattr(0, TCSANOW, &newTermParam);
}

void setCononical()
{
	int err = tcsetattr(0, TCSANOW, &oldTermParam);
	/*printf("cononical%d\n", oldTermParam.c_lflag);*/
}

void printInt(int t)
{
	wchar_t str[overHead[t] - 2];
	int i = overHead[t] - 2;
	int j = i;
	str[0] = L'0';
	while(t != 0)
	{
		if(t % 10 == 0)
			str[i - 1] = L'0';
		if(t % 10 == 1)
			str[i - 1] = L'1';
		if(t % 10 == 2)
			str[i - 1] = L'2';
		if(t % 10 == 3)
			str[i - 1] = L'3';
		if(t % 10 == 4)
			str[i - 1] = L'4';
		if(t % 10 == 5)
			str[i - 1] = L'5';
		if(t % 10 == 6)
			str[i - 1] = L'6';
		if(t % 10 == 7)
			str[i - 1] = L'7';
		if(t % 10 == 8)
			str[i - 1] = L'8';
		if(t % 10 == 9)
			str[i - 1] = L'9';
		--i;
		t /= 10;
	}
	for(i = 0; i < j; ++i)
	{
		fputwc(str[i], stdout);
	}
}

void draw(wchar_t** from, int fR, int lR, int fC, int lC, int f)
{	
	int i = 0;
	int j = 0;
	for(i = 0; i < numRows; ++i)
	{
		fputwc(L'\n', stdout);
	}
	for(i = fR; i < lR; ++i)
	{
		if(f)
		{
			if(fC > 0)
				fputwc(L'<', stdout);
			else
				fputwc(L'|', stdout);
			for(j = overHead[i + 1]; j < overHead[lR]; ++j)
				fputwc(L' ', stdout);
			printInt(i + 1);
			fputwc(L':', stdout);
		}
		j = fC;
		while(j < wcslen(from[i]) && j < lC && from[i][j] != L'\0')
		{
			fputwc(from[i][j], stdout);
			++j;
		}
		fputwc(L'\n', stdout);
	}
}

int reDraw(wchar_t** from, int fR, int lR, int fC, int numColumns, int* len, int* mxLen, int* overHead, int f)
{
	int i = 0;
	int lC = 0;
	(*mxLen) = 0;
	for(i = fR; i < lR; ++i)
	{
		if(len[i] > (*mxLen))
			(*mxLen) = len[i];
	}
	if(f == 0)
		if(numColumns < (*mxLen) )
			lC = numColumns;
		else
			lC = (*mxLen);
	else
		if(numColumns < (*mxLen) + overHead[lR])
			lC = numColumns - overHead[lR];
		else
			lC = (*mxLen);
	lC += fC;
	draw(from, fR, lR, fC, lC, f);
	return lC;
}

int main(int argc, char** argv)
{
	FILE* in = NULL;
	struct winsize windowSize;
	int err = 0;
	int i;
	err = tcgetattr(0, &oldTermParam);
	if(argc == 1)
	{
		printf("No input file\n");
		return 0;
	}
	if(argv[1][0] == '-' && argv[1][1] == 'n')
	{
		f = 1;
		in = fopen(argv[2], "r");
	}
	else
	{
		in = fopen(argv[1], "r");
		if(argv[2] != NULL && argv[2][0] == '-' && argv[2][1] == 'n')
			f = 1;
	}
	err = ioctl(0, TIOCGWINSZ, &windowSize);
	numRows = windowSize.ws_row;
	numColumns = windowSize.ws_col;
	setlocale(LC_ALL, "");
	file = scanFile(in, &err, &sizeOfFile);
	if(err == 3)
	{
		perror("Bad alloc");
		for(i = 0; i < sizeOfFile; ++i)
			free(file[i]);
		free(file);
		if(in != NULL)
			fclose(in);
		return 3;
	}
	if(err == 4)
	{
		perror("Bad file");
		for(i = 0; i < sizeOfFile; ++i)
			free(file[i]);
		if(file != NULL)
			free(file);
		if(in != NULL)
			fclose(in);
		return 4;
	}
	strLen = malloc(sizeOfFile * sizeof(int));
	overHead = malloc((sizeOfFile + 1) * sizeof(int));
	for(i = 0; i < sizeOfFile; ++i)
	{
		strLen[i] = wcslen(file[i]);
	}
	overHead[0] = 2;
	for(i = 1; i <= sizeOfFile; ++i)
	{
		overHead[i] = overHead[i / 10] + 1;
	}
	++overHead[0];
	firstRow = 0;
	if(numRows < sizeOfFile)
		lastRow = numRows - 1;
	else
		lastRow = sizeOfFile;
	lastColumn = reDraw(file, firstRow, lastRow, firstColumn, numColumns, strLen, &maxLength, overHead, f);
	signal(SIGWINCH, handler);
	setNoncononical();
	int tmp = 0;
	while(2 * 2 == 4)
	{
		tmp = 0;
		read(0, &tmp, 4);
		if((char)tmp == 'q')
			break;
		if(tmp == KEY_UP)
		{
			if(firstRow == 0)
				continue;
			--firstRow;
			--lastRow;
			lastColumn = reDraw(file, firstRow, lastRow, firstColumn, numColumns, strLen, &maxLength, overHead, f);
		}
		if(tmp == KEY_DOWN)
		{
			if(lastRow == sizeOfFile)
				continue;
			++firstRow;
			++lastRow;
			lastColumn = reDraw(file,firstRow, lastRow, firstColumn, numColumns, strLen, &maxLength, overHead, f);
		}
		if(tmp == KEY_LEFT)
		{
			if(firstColumn == 0)
				continue;
			--firstColumn;
			--lastColumn;
			draw(file, firstRow, lastRow, firstColumn, lastColumn, f);
		}
		if(tmp == KEY_RIGHT)
		{
			if(lastColumn == maxLength)
				continue;
			++firstColumn;
			++lastColumn;
			draw(file, firstRow, lastRow, firstColumn, lastColumn, f);
		}
	}
	setCononical();
	for(i = 0; i < sizeOfFile; ++i)
		free(file[i]);
	free(file);
	free(strLen);
	free(overHead);
	fclose(in);
	return 0;
}
