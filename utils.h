#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

struct MemStr {
	char *memory;
	size_t size;
};

int starts_with(const char *pre, const char *str)
{
	char cp, cs;
	if (!*pre) return 1;

	while ((cp = *pre++) && (cs = *str++))
	{
		if (cp != cs) return 0;
	}
	if (!cs) return 0;
	return 1;

}

char *to_lower(char* s)
{
	for (char *p = s; *p; p++)*p = tolower(*p);
	return s;
}

static size_t write_cb(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	struct MemStr *mem = (struct MemStr *)userp;

	char *ptr = realloc(mem->memory, mem->size + realsize + 1);

	if (!ptr)
	{
		printf("\033[0;31mNot enough memory, realloc() returned NULL\033[0m");
		return 0;
	}

	mem->memory = ptr;
	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}

void curl_get(CURL **curl, char * url, struct MemStr *chunk, char *postfield)
{
	CURLcode res;

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)chunk);

	if (postfield == NULL) curl_easy_setopt(curl, CURLOPT_REFERER, "https://steamunlocked.net");
	if (postfield != NULL) curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield);

	res = curl_easy_perform(curl);

	if (res != CURLE_OK) {
		fprintf(stderr, "\033[0;31mcurl_easy_perform() failed : \033[0m%s\n", curl_easy_strerror(res));
	}

}

struct input {
	char* name;
	char* value;
};

#define INFO(...)                                                             \
    do {                  														\
    	printf("\033[0;36m");                                                     			\
        fprintf(stdout, __VA_ARGS__);                                          \
        fprintf(stdout, "\033[0m\n");                                                 \
    }                                                                          \
    while (0)