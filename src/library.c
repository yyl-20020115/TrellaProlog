#include "library.h"
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

library* g_libs = 0;
unsigned char* load_file_content(const char* path, unsigned int* plen) {
	unsigned char* buffer = 0;
	FILE* fp = fopen(path, "rb");
	if (fp != 0) {
		fseek(fp, 0, SEEK_END);

		unsigned int lp = ftell(fp);

		fseek(fp, 0, SEEK_SET);

		if (plen != 0) {
			buffer = malloc(*plen = lp);
			if (buffer != 0) {
				buffer[lp] = '\0';
				fread(buffer, sizeof(unsigned char), lp, fp);
			}
		}

		fclose(fp);
	}

	return buffer;
}
library* load_libraries(const char* path) {
	library* libs = 0;
	if (path != 0) {
		int count = 0;
		struct dirent* ent = 0;
		DIR* pDir = 0;
		char dir[4096] = { 0 };
		struct stat statbuf = { 0 };
		if ((pDir = opendir(path)) !=0)
		{
			while ((ent = readdir(pDir)) != 0)
			{
				stat(dir, &statbuf);
				if (!S_ISDIR(statbuf.st_mode))
				{
					int len = strlen(ent->d_name);
					if (len>3 && _stricmp(ent->d_name + len - 3, ".pl") == 0) {
						count++;
					}
				}

			} //while
			closedir(pDir);
		}
		int i = 0;

		libs = malloc(sizeof(library) * (count + 1));
		if (libs != 0) {
			if ((pDir = opendir(path)) != 0)
			{
				while ((ent = readdir(pDir)) != 0)
				{
					stat(dir, &statbuf);
					if (!S_ISDIR(statbuf.st_mode))
					{
						int len = strlen(ent->d_name);
						if (len > 3 && _stricmp(ent->d_name + len - 3, ".pl") == 0) {
							char* name = _strdup(ent->d_name);
							name[len - 3] = '\0';

							libs[i].name = name;
							libs[i].start = load_file_content(ent->d_name, &libs[i].length);
							i++;
						}
					}
				} //while
				closedir(pDir);
			}
			libs[i].name = 0;
			libs[i].length = 0;
			libs[i].start = 0;
		}
	}
	return libs;
}
void free_libraries(library* libs) {
	for (library* lib = libs;lib != 0 && lib->name != 0;lib++) {
		free(lib->name);
		free(lib->start);
	}
	free(libs);
}
// {
//	 {"abnf", library_abnf_pl, &library_abnf_pl_len},
//	 {"apply", library_apply_pl, &library_apply_pl_len},
//	 {"assoc", library_assoc_pl, &library_assoc_pl_len},
//	 {"atts", library_atts_pl, &library_atts_pl_len},
//	 {"builtins", library_builtins_pl, &library_builtins_pl_len},
//	 {"charsio", library_charsio_pl, &library_charsio_pl_len},
//	 {"dcgs", library_dcgs_pl, &library_dcgs_pl_len},
//	 {"dict", library_dict_pl, &library_dict_pl_len},
//	 {"dif", library_dif_pl, &library_dif_pl_len},
//	 {"error", library_error_pl, &library_error_pl_len},
//	 {"format", library_format_pl, &library_format_pl_len},
//	 {"freeze", library_freeze_pl, &library_freeze_pl_len},
//	 {"http", library_http_pl, &library_http_pl_len},
//	 {"json", library_json_pl, &library_json_pl_len},
//	 {"lambda", library_lambda_pl, &library_lambda_pl_len},
//	 {"lists", library_lists_pl, &library_lists_pl_len},
//	 {"ordsets", library_ordsets_pl, &library_ordsets_pl_len},
//	 {"pairs", library_pairs_pl, &library_pairs_pl_len},
//	 {"pio", library_pio_pl, &library_pio_pl_len},
//	 {"random", library_random_pl, &library_random_pl_len},
//	 {"si", library_si_pl, &library_si_pl_len},
//	 {"sqlite3", library_sqlite3_pl, &library_sqlite3_pl_len},
//	 {"sqlite3_register", library_sqlite3_register_pl, &library_sqlite3_register_pl_len},
//	 {"ugraphs", library_ugraphs_pl, &library_ugraphs_pl_len},
//	 {"when", library_when_pl, &library_when_pl_len},
//	 {"yall", library_yall_pl, &library_yall_pl_len},
//
//	 {0}
//};
