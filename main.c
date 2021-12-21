#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <wchar.h>
#include <string.h>
#include <locale.h>
#include <math.h>

void cls();

typedef struct Word Word;
typedef struct WordBase WordBase;
typedef struct WordGroup WordGroup;
typedef struct Cache Cache;

struct Word {
	int p;
	int score;
};

struct WordBase {
	Word* words;
	size_t size;
	wchar_t buffer[];
};


struct WordGroup {
	WordBase* base;
	Word* words;
	size_t size;
};

struct Cache {
	wchar_t* start;
	wchar_t* mission;
	Word* words;
	size_t size;
};

enum Mode{
    Mission,
    NoMission
};

struct CommandLine {
    wchar_t* buffer;
    wchar_t* start;
    wchar_t* mission;
};

enum Filter_opt {
    Startwith,
    Contain,
    Howmany,
};

void SwapWord(Word *A, Word *b);
void InsertionSort(Word A[], int n);
void InsertBlock(Word A[], int i, int k, Word T[]);
void FastInsertionSortRec(Word A[], int n, float c);

WordBase* MakeWordBase(const char* file_path);
void FreeWordBase(WordBase* base);

void FreeWordGroup(WordGroup* words);

struct CommandLine GetCommandLine();

WordGroup* Filter(const WordGroup* restrict in, enum Filter_opt option, const wchar_t* key, const int Max);

int main(void) {
    setlocale(LC_ALL, "");
    
    WordBase* base = MakeWordBase("words.db");

    if (!base) return 1;

    enum Mode flag = NoMission;

    WordGroup baseGroup;
    baseGroup.base = base;
    baseGroup.words = base->words;
    baseGroup.size = base->size;
    WordGroup* Out = NULL;

	struct CommandLine cmd;
	
	struct Cache* caches = malloc(sizeof(struct Cache));
	int cache_len = 0;
	int cache_cap = 1;

    const int Max = 44;
    while(1) {
		bool use_cache = false;
        cmd = GetCommandLine();
		cls();
        if (!cmd.start) continue;
        if (!cmd.mission) flag = NoMission;
        else flag = Mission;
		
		for (int i = 0;i < cache_len;i++) {
			if (caches[i].mission == NULL) continue;
			if (wcscmp(caches[i].start, cmd.start) == 0 && wcscmp(caches[i].mission, cmd.mission) == 0) {
				Out->base = base;
				Out->words = caches[i].words;
				Out->size = caches[i].size;
				use_cache = true;
				break;
			}
		}

		if (use_cache == false) {
			if (flag == NoMission) {
				Out = Filter(&baseGroup, Startwith, cmd.start, Max);
				
				if (Out == NULL) continue;
			} else if (flag == Mission) {
				if (wcscmp(cmd.start, L"") == 0) {
					wchar_t* p = wcschr(cmd.mission, L'\n');
					*p = L'\0';
					Out = Filter(&baseGroup, Howmany, cmd.mission, 0);
				} else {
					if (cmd.mission[0] == L'\0') continue;
					WordGroup* tmp = Filter(&baseGroup, Startwith, cmd.start, 0);
					if (tmp == NULL) continue;
					Out = Filter(tmp, Howmany, cmd.mission, 0);
					FreeWordGroup(tmp);
					if (Out == NULL) continue;
				}

				FastInsertionSortRec(Out->words, Out->size, 3);
			}
		}
        
		for (int i = 0;i < (Out->size < Max ? Out->size : Max);i++)
            wprintf(L"%S\n", Out->base->buffer + Out->words[i].p);

		if (use_cache == false) {
			if (cache_cap <= cache_len) {
				caches = realloc(caches, sizeof(struct Cache)*(cache_cap*=2));
				if (caches == NULL) {
					fprintf(stderr, "realloc failed\n");
					return 1;
				}
			}
	
			caches[cache_len].start = malloc(sizeof(wchar_t)*(wcslen(cmd.start) + 1));
			wcscpy(caches[cache_len].start, cmd.start);
			if (cmd.mission != NULL) {
				caches[cache_len].mission = malloc(sizeof(wchar_t)*(wcslen(cmd.mission) + 1));
				wcscpy(caches[cache_len].mission, cmd.mission);
			}
			caches[cache_len].words = Out->words;
			caches[cache_len].size = Out->size;
			cache_len++;
		}
	}

    FreeWordBase(base);
    return 0;
}

WordBase* MakeWordBase(const char* file_path) {
    FILE* db_f;
    db_f = fopen(file_path, "r");
    if (!db_f) {
        fprintf(stderr, "Failed to Read DB\n");
        return NULL;
    }

    fseek(db_f, 0, SEEK_END);
    long db_size = ftell(db_f);
    rewind(db_f);

    WordBase* base = malloc(sizeof(WordBase) + sizeof(wchar_t)*db_size);
    memset(base, 0, sizeof(WordBase) + sizeof(wchar_t)*db_size);
    for (wchar_t c = getwc(db_f); c!= EOF; c = getwc(db_f))
        if (c == L'\n')
            base->size++;
    rewind(db_f);

    base->size++;
    base->words = malloc(sizeof(Word)*base->size);
    base->size--;

    int p = 0;
    int length = 1;

    while(1) {
        size_t size;
        wchar_t* str = fgetwln(db_f, &size);
        if (size == 0) break;
        
        base->words[length].p = p;
        base->words[length].score = size*10;

        for (int i = 0;i < size - 1;i++)
            if ((str[i] - 0xAC00)%28 == 0) base->words[length].score++;

        wcscpy(base->buffer + p, str);
        base->buffer[p + size - 1] = L'\0';

        p += size;
        length++;
    }

	FastInsertionSortRec(base->words, base->size, 3);

    fclose(db_f);

    return base;
}

void FreeWordBase(WordBase* base) {
	free(base->words);
	free(base);
}

void FreeWordGroup(WordGroup* words) {
	free(words->words);
	free(words);
}

struct CommandLine GetCommandLine() {
	struct CommandLine cmd = {NULL, NULL, NULL};
	size_t size;
	cmd.buffer = fgetwln(stdin, &size);

	if (size == 0) {
		return cmd;
	}

	cmd.start = cmd.buffer;
	
	cmd.mission = wcsstr(cmd.buffer, L" ");
	if (cmd.mission) {
		cmd.mission[0] = L'\0';
		cmd.mission++;
		if (*cmd.mission == L'\0')
			return cmd;
	}

	if (*cmd.start == L'\0')
		return cmd;

	cmd.start[size - 1] = L'\0';
	return cmd;
}

WordGroup* Filter(const WordGroup* restrict in, enum Filter_opt option, const wchar_t* key, const int Max) {
    WordGroup* out = malloc(sizeof(WordGroup));
    out->base = in->base;
    out->words = malloc(sizeof(Word)*in->size);
    out->size = 0;
    int limit = Max ? Max : in->size;
    switch(option) {
        case Startwith:
			for (int i = 0;i < in->size;i++) {
                if (wcsncmp(key, in->base->buffer + in->words[i].p, wcslen(key)) == 0) {
                    out->words[out->size].p = in->words[i].p;
					out->words[out->size].score = in->words[i].score;
                    out->size++;

                    if (out->size == limit)
                        break;
                }
            }
        break;
        case Contain:
            for (int i = 0;i < in->size;i++) {
                if (wcsstr(in->base->buffer + in->words[i].p, key)) {
                    out->words[out->size].p = in->words[i].p;
					out->words[out->size].score = in->words[i].score;
                    out->size++;

                    if (out->size == limit)
                        break;
                }
            }
        break;
        case Howmany:
            for (int i = 0;i < in->size;i++) {
                int count = 0;
                wchar_t* p = in->base->buffer + in->words[i].p - 1;
                while((p = wcsstr(p + 1, key)))
                    count++;
                if (count) {
                    out->words[out->size].p = in->words[i].p;
                    out->words[out->size].score = count*1000 + in->words[i].score;
                    out->size++;

                    if (out->size == limit)
                        break;
                }
            }
        break;

    }

	if (out->size)
		out->words = realloc(out->words, sizeof(Word)*out->size);
	else {
		free(out->words);
		free(out);
		out = NULL;
	}

    return out;
}

void SwapWord(Word* a, Word *b) {
	Word temp;
	temp = *a;
	*a = *b;
	*b = temp;
}

void InsertionSort(Word A[], int n) {
	for (int i = 1;i < n;i++) {
		int j = i - 1;
		Word v = A[i];
		while (j >= 0 && A[j].score < v.score) {
			A[j + 1] = A[j];
			--j;
		}
		A[j + 1] = v;
	}
}

void InsertBlock(Word A[], int i, int k, Word T[]) {
	for (int j = 0;j < k;j++)
		T[j] = A[i + j];
	int l = k - 1;
	int j = i - 1;

	while (l >= 0) {
		while (j >= 0 && A[j].score < T[l].score) {
			A[j + l + 1] = A[j];
			--j;
		}
		A[j + l + 1] = T[l];
		--l;
	}
}

void FastInsertionSortRec(Word A[], int n, float c) {
	int h = (float) log(n)/log(c);
	if (h <= 1)
		return InsertionSort(A, n);
	float exp = (float) (h - 1)/h;
	int k = pow(n, exp);
	if (n <= k || k <= 5)
		return InsertionSort(A, n);
	Word* T = malloc(sizeof(Word)*k);
	T[0] = A[n - 1];
	for (int i = 0;i < n;i += k) {
		int b = k < n - 1 ? k : n - 1;
		FastInsertionSortRec(A + i, b, c);
		InsertBlock(A, i, b, T);
	}

	free(T);
}

void SortWordGroup(WordGroup* words) {
    int i, j;
	Word word;
    i = words->size - 1;
    while( i-- > 0) {
        j = i;
        word = words->words[i];
        while (++j < words->size && word.score < words->words[j].score);
        
        if (--j == i) continue;
        memcpy(words->words + i, words->words + i + 1, sizeof(Word)*(j - i));
        words->words[j] = word;
    }
}

void cls() {
	system("clear");
}
