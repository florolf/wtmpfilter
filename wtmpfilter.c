#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <utmp.h>

unsigned char *ttybuf;
uint32_t ttybuf_sz;

static bool check_pass_record(char **users, int n_users, struct utmp *u) {
	/* Logout record */
	if(!u->ut_user[0]) {
		for(int i = 0; i <= ttybuf_sz; i++) {
			unsigned char *record = &ttybuf[i * UT_LINESIZE];

			if(*record && !strcmp(record, u->ut_line)) {
				memset(record, 0, UT_LINESIZE);

				return false;
			}
		}

		return true;
	}

	/* Regular record, apply filter logic.
	 *
	 * We depend on the user not to insert any special names here,
	 * cf. last.c of sysvinit.
	 */
	int matched = 0;

	for(int i = 0; i < n_users; i++) {
		if(!strcmp(u->ut_user, users[i])) {
			matched = 1;
			break;
		}
	}

	if(!matched)
		return true;

	/* Find a free tty slot */
	int slot;
	for(slot = 0; slot < ttybuf_sz; slot++) {
		unsigned char *record = &ttybuf[slot * UT_LINESIZE];

		if(!*record)
			break;

		if(!strcmp(record, u->ut_line))
			break;
	}

	/* Resize ttybuf if it's full */
	if(slot == ttybuf_sz) {
		uint32_t ttybuf_sz_new = ttybuf_sz * 2;

		ttybuf = realloc(ttybuf, ttybuf_sz_new * UT_LINESIZE);
		memset(&ttybuf[slot * UT_LINESIZE], 0, ttybuf_sz_new - ttybuf_sz);

		ttybuf_sz = ttybuf_sz_new;
	}

	memcpy(&ttybuf[slot * UT_LINESIZE], u->ut_line, UT_LINESIZE);
	return false;
}

int main(int argc, char **argv) {
	if(argc < 2) {
		fprintf(stderr, "usage: %s user1 [user2 .. userN]\n", argv[0]);
		return EXIT_FAILURE;
	}

	ttybuf_sz = 1;
	ttybuf = calloc(ttybuf_sz, UT_LINESIZE);

	while(1) {
		struct utmp u;
		if(fread(&u, sizeof(u), 1, stdin) != 1) {
			if(feof(stdin))
				return EXIT_SUCCESS;

			if(ferror(stdin)) {
				perror("error while reading record");
				return EXIT_FAILURE;
			}
		}

		if(check_pass_record(argv + 1, argc - 1, &u)) {
			if(fwrite(&u, sizeof(u), 1, stdout) != 1) {
				perror("error while writing record");
				return EXIT_FAILURE;
			}
		}
	}
}
