/*
 * A sample implementation of a parser for Oracle Siebel's proprietary SiebelMessage format.
 * Copyright (C) 2014 Michael Goehler
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * SiebelMessage format sample:
 * @0*0*34*0*0*0*8*Comments0*19*RequestLanguageCode3*DEU15*RequestSendFlag1*N17*RequestLocaleCode0*12*PerfTestMode0*14*RecipientGroup0*13*CommRequestId0*19*CommProfileOverride20*SiebelReplyContainer12*SourceBusObj6*Action18*KeepSuccessMessage4*true17*StateFileOverride0*19*CommRequestParentId0*11*ProcessMode5*Local20*RequestDefaultMedium0*7*Charset0*11*TestAddress0*12*SourceIdList9*1-4JE0ZIS16*RecipientBusComp6*Action9*WebServer0*8*NumTasks0*11*RequestName0*15*PackageNameList40*eMail Response - Auto Acknowledge (HTML)19*MsgReplyAddressList19*testing@example.com12*TaskRecipMin0*20*ChildRecipSearchSpec0*13*NumRecipients0*10*CreateOnly1*011*MessageType0*15*RequestTimeZone0*13*TaskStartDate0*15*MessageTracking5*false9*LoginName6*SADMIN16*DefinedComponent0*15*RecipSearchSpec0*`
 *
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Some standard definitions to handle strings in C.
 *
 */

#define REALLOC_ADD 10

typedef struct _cstring_t {
    char *text;
    size_t size;
    size_t alloc;
    void (*expand)(struct _cstring_t *self, char x);
    void (*expand_arr)(struct _cstring_t *self, char *x);
    void (*strip)(struct _cstring_t *self, int pos, int len);
    void (*reset)(struct _cstring_t *self);
    void (*delete)(struct _cstring_t *self);
} cstring_t;

cstring_t *cstring_init();
void cstring_expand(cstring_t *self, char x);
void cstring_expand_arr(cstring_t *self, char *x);
void cstring_strip(cstring_t *self, int pos, int len);
void cstring_reset(cstring_t *self);
void cstring_delete(cstring_t *self);

cstring_t *cstring_init() {
    cstring_t *x = NULL;
    if((x = malloc(sizeof(cstring_t))) != NULL) {
        x->text = NULL;
        x->size = x->alloc = 0;
        x->expand = cstring_expand;
        x->expand_arr = cstring_expand_arr;
        x->strip = cstring_strip;
        x->reset = cstring_reset;
        x->delete = cstring_delete;
    } else {
        fprintf(stderr, "%s\n", "cstring_init() failed to allocate memory.");
        exit(EXIT_FAILURE);
    }
    return x;
}

void cstring_expand(cstring_t *self, char x) {
    if(self->size + sizeof(x) + sizeof(char) > self->alloc) {
        self->alloc += (REALLOC_ADD * sizeof(char));
        if((self->text = realloc(self->text, self->alloc)) == NULL) {
            fprintf(stderr, "%s\n", "cstring_expand() failed to reallocate memory.");
            exit(EXIT_FAILURE);
        }
    }
    self->text[self->size] = x;
    self->text[self->size+1] = '\0';
    self->size = strlen(self->text);
}

void cstring_expand_arr(cstring_t *self, char *x) {
    if(self->size + strlen(x) + sizeof(char) > self->alloc) {
        self->alloc = ((strlen(x) + self->size + 1) * sizeof(char));
        if((self->text = realloc(self->text, self->alloc)) == NULL) {
            fprintf(stderr, "%s\n", "cstring_expand() failed to reallocate memory.");
            exit(EXIT_FAILURE);
        }
    }
    self->text = strcat(self->text, x);
    self->size = strlen(self->text);
    self->text[self->size+1] = '\0';
}

void cstring_strip(cstring_t *self, int pos, int len) {
    if(pos + len >= self->size) {
        if(pos <= self->size) {
            self->text[pos] = '\0';
            self->size = pos;
        }
        return;
    }
    memmove(&self->text[pos], &self->text[pos+len], self->size - pos);
    self->size -= len;
}

void cstring_reset(cstring_t *self) {
    free(self->text);
    self->text = NULL;
    self->size = self->alloc = 0;
}

void cstring_delete(cstring_t *self) {
    free(self->text);
    free(self);
}

int is_utf8(char ch) {
    return (ch & 0x80) != 0x00;
}

int length_utf8(char ch) {
    int i = 0;
    while(is_utf8(ch)) {
        i++;
        ch <<= 1;
    }
    return i;
}

/*
 * Start of the actual example code.
 *
 */

void usage() {
    fprintf(stderr, "%s", "Usage: smp [FILE]\n\n");
    fprintf(stderr, "%s", "A parser for Oracle Siebel's proprietary SiebelMessage format.\n\n");
    fprintf(stderr, "%s", "Copyright (C) 2014 Michael Goehler\n");
    fprintf(stderr, "%s", "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.\n");
    fprintf(stderr, "%s", "This is free software: you are free to change and redistribute it.\n");
    fprintf(stderr, "%s", "There is NO WARRANTY, to the extent permitted by law.\n\n");
}

int main(int argc, char *argv[]) {

    int c, i, len, props, extends;
    char *file;
    FILE *input;
    cstring_t *str, *slen;

    if (argv[1]) {
        file = argv[1];
    } else {
        fprintf(stderr, "%s: %s\n", argv[0], "no input file");
        usage();
        exit(1);
    }
    if (!strcmp(file, "-")) {
        input = stdin;
    } else {
        input = fopen(file,"r");
        if (!input) {
            fprintf(stderr, "%s: %s: %s\n", argv[0], file, strerror(errno));
            usage();
            exit(1);
        }
    }

    c = fgetc(input);
    if (c == '@') {
        str = cstring_init();
        slen = cstring_init();

        // flags
        i = 0;
        while ((c = fgetc(input)) != EOF) {
            if (c == '*') {
                i++;
                if (i == 3)
                    props = atoi(str->text) * 2;
                if (i == 4)
                    extends = atoi(str->text) + 1;
                (str->reset)(str);
            } else {
                (str->expand)(str, c);
            }

            if (i == 6)
                break;
        }

        while (extends > 0) {
            // props
            len = 0;
            while ((c = fgetc(input)) != EOF) {
                if (c == '*' && str->size >= len) {
                    // new property
                    if (str->size > 0) {
                        printf((props % 2 == 0) ? "%s\n" : "%s: ", str->text);
                    } else {
                        printf("\n");
                    }
                    len = atoi(slen->text);
                    (str->reset)(str);
                    (slen->reset)(slen);
                    if (props == 0) {
                        // + 1 because of the extends name field
                        props = (len + 1) * 2;
                        do {
                            c = fgetc(input);
                        } while ( c != EOF && c != '*');
                        break;
                    }
                    props--;
                } else if (str->size >= len) {
                    (slen->expand)(slen, c);
                } else if (is_utf8(c)) {
                    (str->expand)(str, c);
                    for(i = 1; i <= length_utf8(c); i++) {
                        c = fgetc(input);
                        (str->expand)(str, c);
                        // increase len because str->size is in byte
                        len++;
                    }
                } else {
                    (str->expand)(str, c);
                }
            }

            // print last fields content after EOF was reached
            if (str->size > 0)
                printf((props % 2 == 0) ? "%s\n" : "%s: ", str->text);

            extends--;
        }
    } else {
        fprintf(stderr, "%s: %s: %s\n", argv[0], file, "not a siebel message");
        usage();
        exit(1);
    }

    (str->delete)(str);
    (slen->delete)(slen);

    return(0);
}

