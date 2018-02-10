#ifndef _DIRECT_HEADER_

#define _DIRECT_HEADER_

#include <iostream>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

using namespace std;


bool is_directory(const char *path) {
	struct stat statbuf;
	if (stat(path, &statbuf) != 0)
		return false;
	return S_ISDIR(statbuf.st_mode);
}

bool is_regular(const char *path) {
	struct stat statbuf;
	if (stat(path, &statbuf) != 0)
		return false;
	return S_ISREG(statbuf.st_mode);
}

int count_files_number(char* name)
{
	static int n = 0;
	DIR* c_dir = nullptr;

	c_dir = opendir(name);

	struct dirent* box;
	box = readdir(c_dir);

	while (box)
	{
		char* n_name = new char[300];
		n_name[0] = '\0';
		strcat_s(n_name, 300, name);
		strcat_s(n_name, 300, "/");
		strcat_s(n_name, 300, box->d_name);

		if (box->d_name[0] != '.' && is_directory(n_name))
			count_files_number(n_name);

		if (box->d_name[0] != '.' && is_regular(n_name))
			n++;

		box = readdir(c_dir);

		delete[] n_name;
	}
	return n;
}



#endif _DIRECT_HEADER_