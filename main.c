#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include <time.h>

void cls() {
	system("clear");
}

#define MAXWCHARLEN 30
#define MAXTREELEN 1000

struct sTrie {
    wchar_t p[MAXWCHARLEN];
    size_t p_len;
    struct sTrie** kids;
    size_t len;
    bool isEnd;
};

struct sTries {
	struct sTrie* tries;
	size_t len;
	bool check[MAXTREELEN];
};

struct sTrie* getNode(const wchar_t w) {
	static struct sTries* tries = NULL;
	static size_t trie_index = 0;
	static size_t len = 0;

	if (tries == NULL) {
		tries = malloc(sizeof(struct sTries));
		tries[trie_index].tries = malloc(sizeof(struct sTrie)*MAXTREELEN);
		tries[trie_index].len = 0;
	}

	len++;
	if (len%MAXTREELEN == 0) {
		trie_index++;
		tries = realloc(tries, sizeof(struct sTries)*(trie_index + 1));
		tries[trie_index].tries = malloc(sizeof(struct sTrie)*MAXTREELEN);
		tries[trie_index].len = 0;
	}

    struct sTrie* pNode = &tries[trie_index].tries[len%MAXTREELEN];
    memset(pNode->p, 0, sizeof(pNode->p));
    pNode->p[0] = w;
    pNode->p_len = 1;
    pNode->isEnd = false;
    pNode->kids = NULL;
    pNode->len = 0;

    return pNode;
}

void insertNode(register struct sTrie* node_p, register wchar_t* wcs_p) {
    register struct sTrie* p,* child;
    register long i, j;
    
	while(1) {
		Loop:
		if (*wcs_p == L'\0') {
			node_p->isEnd = true;
			return;
		}

		for (i = 0;i < node_p->len;++i) {
			if (node_p->kids[i]->p[0] == *wcs_p) {
				node_p = node_p->kids[i];
				wcs_p++;
				goto Loop;
			}
		}

		node_p->kids = realloc(node_p->kids, sizeof(struct sTrie*)*(node_p->len + 1));
		child = getNode(*wcs_p);
		node_p->kids[node_p->len] = child;
		node_p->len++;

		for (i = 0;i < node_p->len;++i) {
			p = node_p->kids[i];
			for (j = i - 1;j >= 0 && node_p->kids[j]->p[0] > p->p[0];--j) {
				node_p->kids[j + 1] = node_p->kids[j];
			}

			node_p->kids[j + 1] = p;
		}

		node_p = child;
		wcs_p++;
	}
}

void printNode(register struct sTrie* node_p, wchar_t* wcs, int level) {
    if (node_p->isEnd) {
        wcs[level] = L'\0';
    }

    for (register long i = 0;i < node_p->len;++i) {
        wcs[level] = node_p->kids[i]->p[0];
        printNode(node_p->kids[i], wcs, level + 1);
    }
}

struct Word {
    wchar_t* word;
    size_t len;
};

struct sWords {
    struct Word* words;
    size_t len;
    size_t cap;
};

struct sMission {
	int score;
	int len;
	int p;
};

void CompressTrie(struct sTrie* node_p) {
    if (node_p->isEnd || node_p->len > 1) {
        for (int i = 0;i < node_p->len;++i)
            CompressTrie(node_p->kids[i]);
		return;
    } else {
        struct sTrie* child = node_p->kids[0];
        if (node_p->p_len >= MAXWCHARLEN - 1) {
            CompressTrie(child);
            return;
        }

        node_p->p[node_p->p_len]  = child->p[0];
        node_p->p_len++;
        node_p->isEnd = child->isEnd;
        free(node_p->kids);
        node_p->kids = child->kids;
        node_p->len = child->len;
        CompressTrie(node_p);
    }
}

void sprintNode(struct sWords* words, register struct sTrie* node_p, wchar_t* wcs, size_t level) {
    if (node_p->isEnd) {
        wcs[level] = L'\0';
        if (words->len >= words->cap) {
            words->cap += 40;
            words->words = realloc(words->words, sizeof(struct Word)*(words->cap));
        }
        words->words[words->len].len = wcslen(wcs);
        words->words[words->len].word = malloc(sizeof(wchar_t*)*(words->words[words->len].len + 1));
        wcsncpy(words->words[words->len].word, wcs, words->words[words->len].len);
        words->words[words->len].word[words->words[words->len].len] = L'\0';
        words->len++;
    }
    for (long i = 0;i < node_p->len;++i) {
        wcsncpy(wcs + level, node_p->kids[i]->p, node_p->kids[i]->p_len);
        sprintNode(words, node_p->kids[i], wcs, level + node_p->kids[i]->p_len);
    }
}

struct sTrie* startwithNode(register struct sTrie* node_p, register const wchar_t* wcs_p) {
    Start:
    if (*wcs_p == L'\0')
        return node_p;

    register size_t start = 0;
    register size_t end = node_p->len;
    register size_t mid;
    

    while (start < end) {
        mid = (start + end)/2;
        if (node_p->kids[mid]->p[0] == *wcs_p) {
            if (wcsncmp(node_p->kids[mid]->p, wcs_p, node_p->kids[mid]->p_len) == 0) {
                wcs_p += node_p->kids[mid]->p_len;
                node_p = node_p->kids[mid];
                goto Start;
            }
            return NULL;
        } else if (node_p->kids[mid]->p[0] < *wcs_p) {
            start = mid + 1;
        } else
            end = mid;
    }

    return NULL;
}

int compare(const void* a, const void* b) {
    const struct Word* word1 = a;
    const struct Word* word2 = b;

    if (word1->len > word2->len)
        return -1;
    if (word1->len < word2->len)
        return 1;

    return 2;
}

int compareMission(const void* a, const void* b) {
    const struct sMission* word1 = a;
    const struct sMission* word2 = b;

	if (word1->score > word2->score)
		return -1;
	if (word1->score < word2->score)
		return 1;
    if (word1->len > word2->len)
        return -1;
    if (word1->len < word2->len)
        return 1;

    return 2;
}

void PrintStartWith(struct sTrie* node, const wchar_t* wcs_p) {
    wchar_t tmp[1000];
    int level = swprintf(tmp, 1000, L"%S", wcs_p);
    node = startwithNode(node, wcs_p);
	if (node == NULL) {
		printf("No match!\n");
		return;
	}
    struct sWords words;
    words.len = 0;
    words.cap = 1;
    words.words = malloc(sizeof(struct Word));
    sprintNode(&words, node, tmp, level);

    qsort(words.words, words.len, sizeof(struct Word), compare);

    for (register int i = 0;i < (40 < words.len ? 40 : words.len);++i) {
      wprintf(L"%S\n", words.words[i].word);
    }
}

void PrintStartWithMission(struct sTrie* node, const wchar_t* wcs_p, const wchar_t* mission) {
    wchar_t tmp[1000];
    int level = swprintf(tmp, 1000, L"%S", wcs_p);
    node = startwithNode(node, wcs_p);
	if (node == NULL) {
		printf("No match!\n");
		return;
	}
    struct sWords words;
    words.len = 0;
    words.cap = 1;
    words.words = malloc(sizeof(struct Word));
    sprintNode(&words, node, tmp, level);

	struct sMission missions[words.len];
	for (register int i = 0;i < words.len;++i) {
		int score = 0;
		for (wchar_t* c = wcsstr(words.words[i].word, mission);c != NULL;c = wcsstr(c + 1 , mission),score++);
		missions[i].score = score;
		missions[i].p = i;
		missions[i].len = words.words[i].len;
	}

    qsort(missions, words.len, sizeof(struct sMission), compareMission);

    for (register int i = 0;i < (40 < words.len ? 40 : words.len);++i) {
      wprintf(L"%S\n", words.words[missions[i].p]);
    }
}

int main(void) {
    setlocale(LC_ALL, "en_US.UTF-8");
    struct sTrie* root = getNode(0);
    FILE* db_f;
    db_f = fopen("words.db", "r");
    if (!db_f) {
        fprintf(stderr, "No DB!\n");
        return 0;
    }

    while(1) {
        size_t size;
        wchar_t* wcs = fgetwln(db_f, &size);
        if (size == 0)
            break;
        
        wcs[size -2] = L'\0';
        insertNode(root, wcs);
    }
    fclose(db_f);

    CompressTrie(root);
    while(1) {
		size_t size;
		wchar_t* cmd = fgetwln(stdin, &size);
		cmd[size - 1] = L'\0';
		wchar_t* ptr,* mission;
		wcstok(cmd, L" ", &ptr);
		mission = wcstok(NULL, L" ", &ptr);
		cls();
		float start = (float)clock()/CLOCKS_PER_SEC;
		if (mission)
			PrintStartWithMission(root, cmd, mission);
		else
			PrintStartWith(root, cmd);
		float end = (float)clock()/CLOCKS_PER_SEC;
		printf("time: %f\n", end - start);
	}
    return 0;
}
