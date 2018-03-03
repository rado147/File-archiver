#ifndef _DIRECT_HEADER_

#define _DIRECT_HEADER_

#include <iostream>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <direct.h>

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

size_t count_dirs_number(char* name, ofstream& out, size_t start_pos)
{
	size_t name_sz = strlen(name) - start_pos;
	
	if (name_sz > 0)
		out.write((char*)& name_sz, sizeof(name_sz));
	
	size_t original_name_sz = strlen(name);
	for (size_t i = start_pos; i < original_name_sz; i++)
		out.write(&name[i], sizeof(name[i]));
	
	static size_t n = 1;
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
		{
			n++;
			count_dirs_number(n_name, out, start_pos);
		}

		box = readdir(c_dir);

		delete[] n_name;
	}
	return n;
}

bool is_existing_file_name(char* file_name)
{
	DIR* dir = opendir("./");

	struct dirent* box;
	box = readdir(dir);

	while (box)
	{
		if (!strcmp(file_name, box->d_name))
			return true;

		box = readdir(dir);
	}

	return false;
}


#endif _DIRECT_HEADER_