#ifndef _GLOBAL_HEADER_

#define _GLOBAL_HEADER_

#include "priority_queue.h"
#include "hash.h"
#include <fstream>
#include "direct.h"


//helping function concatenating two trees
//used in the function for creating a single tree

huffman_tree& concatenate_trees(huffman_tree& fTree, huffman_tree& sTree)
{
	huffman_tree* newTree = new huffman_tree();
	newTree->root = new huffman_node(fTree.root->frequency + sTree.root->frequency, '\0');

	newTree->root->left = fTree.root;
	newTree->root->right = sTree.root;

	return *newTree;
}

//function for building a tree using Huffman's algorithm

huffman_tree& create_huffman_tree(priority_queue& q)
{
	while (q.get_used() > 1)
	{
		huffman_tree first_tree = q.dequeue();
		huffman_tree second_tree = q.dequeue();

		huffman_tree concatenated_tree = concatenate_trees(first_tree, second_tree);

		q.enqueue(concatenated_tree);
	}

	return q.dequeue();
}

//function for gathering data about the file(s) we are going to encode
//saves it all in "arr"
//for example: arr[97] = 21 means that static_cast<char>(97) was encountered 21 times; ('a' was encountered 21 times)

void read_file_frequencies(char* name, int*& arr)
{
	ifstream in;

	in.open(name, ios::binary);
	if (in.fail())
		throw exception("Cannot open file!");

	char c;
	in.get(c);
	while (!in.eof())
	{
		int int_c = static_cast<int>(c);
		
		if (int_c < 0)
			arr[127 - int_c]++;
		else
			arr[int_c]++;

		in.get(c);
	}

	in.close();
}

//the function for creating the tree
//stores the required information for encoding in an object of class simple_hash
//this way we have constant time access to all the tree paths we need

void create_tree_and_fill_table(char* name, simple_hash& h, int*& arr)
{
	read_file_frequencies(name, arr);
	
	priority_queue q;
	q.fill_queue(arr);

	huffman_tree t;
	t = create_huffman_tree(q);
	
	char* path = new char[50];

	h.fill_table(t.root, path);
	delete[] path;
}


void read_single_int(ifstream& in, simple_hash& h, int& one_int, char*& remains)
{
	short sz = 0;
	char c;
	
	short rem_sz = static_cast<short>(strlen(remains));
	while (sz < rem_sz)
	{
		if (remains[sz] == '1')
			one_int |= 1UL << (31 - sz);

		sz++;
	}

	char* cur_path;

	while (sz < 32 && !in.eof())
	{

		in.get(c);

		if (in.eof())
			return;

		int int_c = static_cast<int>(c);
		if (int_c < 0)
			cur_path = h.table[127 - int_c];
		else
			cur_path = h.table[c];

		int p_sz = strlen(cur_path);
		for (int i = sz; i < sz + p_sz; i++)
		{
			if (i >= 32)
				remains[i - 32] = cur_path[i - sz];
			else
			{
				if (cur_path[i - sz] == '1')
					one_int |= 1UL << (31 - i);
			}
		}
		sz += p_sz;
	}

	remains[sz - 32] = '\0';
}

//archivate the file with the given name

void archivate_file_inner(ofstream& out, char* name)
{
	int* arr = new int[257];
	for (size_t i = 0; i < 256; i++)
		arr[i] = 0;

	simple_hash h;
	create_tree_and_fill_table(name, h, arr);

	//writing all the information needed for (later) extracting the archive
	//with the data in "arr" we can restore the huffman_tree
	for (size_t i = 0; i < 256; i++)
		out.write((char*)& arr[i], sizeof(arr[i]));

	ifstream in;
	in.open(name, ios::binary);

	if (in.fail())
		throw exception("Cannot open file!");

	//in "remains" is stored the exccess data (if there is any) after "read_single_int" func. is called
	char* remains = new char[20];
	remains[0] = '\0';

	while (!in.eof())
	{
		int one_int = 0;
		read_single_int(in, h, one_int, remains);
		out.write((char*)& one_int, sizeof(one_int));
	}

	//writing the original size of the file
	in.clear();
	in.seekg(0, ios::end);
	size_t file_sz = static_cast<size_t>(in.tellg());
	out.write((char*)& file_sz, sizeof(file_sz));

	delete[] arr;

	in.close();
}

//extracts the content of an archive with a given name

void extract_archive_inner(ifstream& in, ofstream& out, long long next_file_start)
{
	long long cur_file_start = static_cast<long long>(in.tellg());

	//reading the size of the original file
	size_t file_sz;
	in.seekg(next_file_start - 4, ios::beg);
	in.read((char*)& file_sz, sizeof(file_sz));

	in.clear();
	in.seekg(cur_file_start, ios::beg);

	int* arr = new int[257];
	for (size_t i = 0; i < 256; i++)
	{
		in.read((char*)& arr[i], sizeof(arr[i]));
	}

	priority_queue q;
	q.fill_queue(arr);

	huffman_tree t;
	t = create_huffman_tree(q);

	int one_int;
	huffman_node* cur_node = t.root;

	size_t writen_bytes = 0;
	while (writen_bytes < file_sz)
	{
		in.read((char*)& one_int, sizeof(one_int));
		int counter = 31;
		while (counter >= 0)
		{
			bool bit = (one_int >> counter) & 1U;
			if (bit)
				cur_node = cur_node->right;
			else
				cur_node = cur_node->left;

			if (!cur_node->left && !cur_node->right)
			{
				out.write((char*)& cur_node->character, sizeof(cur_node->character));
				writen_bytes++;

				cur_node = t.root;
			}

			counter--;

			if (writen_bytes == file_sz)
				break;
		}
	}

	delete[] arr;
}

void recursive_traversal_dirs(ofstream& out, char* name, size_t name_start_pos, size_t position_in_file)
{
	static size_t consecutive_file = 0;

	DIR* c_dir = nullptr;

	c_dir = opendir(name);

	struct dirent* box;
	box = readdir(c_dir);

	while (box)
	{
		char n_name[300];
		n_name[0] = '\0';
		strcat_s(n_name, 300, name);
		strcat_s(n_name, 300, "/");
		strcat_s(n_name, 300, box->d_name);

		if (box->d_name[0] != '.' && is_directory(n_name))
			recursive_traversal_dirs(out, n_name, name_start_pos, position_in_file);

		else if (box->d_name[0] != '.' && is_regular(n_name))
		{
			out.clear();

			long long cur_position = static_cast<long long>(out.tellp());
			out.seekp(position_in_file + consecutive_file * sizeof(long long), ios::beg);
			out.write((char*)& cur_position, sizeof(long long));
			
			out.clear();
			out.seekp(cur_position, ios::beg);
			consecutive_file++;

			size_t name_sz = strlen(n_name) - name_start_pos;
			out.write((char*)& name_sz, sizeof(name_sz));
			
			for (size_t i = name_start_pos; i < strlen(n_name); i++)
				out.write(&n_name[i], sizeof(n_name[i]));
			
			archivate_file_inner(out, n_name);

			std::cout << box->d_name << " is archivated..." << std::endl;
		}

		box = readdir(c_dir);
	}
}


void archivate(char* name)
{
	ofstream out;

	char* arch_name = new char[64];
	std::cout << "Enter archive name: ";
	cin.getline(arch_name, 63);

	out.open(arch_name, ios::binary);
	if (out.fail())
		throw exception("Cannnot open file!");

	delete[] arch_name;

	if (is_regular(name))
	{
		size_t dirs_number = 0;
		size_t files_number = 1;
		size_t name_sz;
		long long start_position = sizeof(files_number) + sizeof(dirs_number) + sizeof(long long);

		out.write((char*)& dirs_number, sizeof(dirs_number));
		out.write((char*)& files_number, sizeof(files_number));
		out.write((char*)& start_position, sizeof(start_position));
		
		size_t i = strlen(name);
		size_t original_name_sz = i;
		while (name[i - 1] != '/')
			i--;

		name_sz = strlen(name) - i;
		out.write((char*)& name_sz, sizeof(name_sz));

		for (size_t j = i; j < original_name_sz; j++)
			out.write(&name[j], sizeof(name[j]));

		archivate_file_inner(out, name);
	}

	if (is_directory(name))
	{
		size_t index = strlen(name) - 1;

		while (name[index] != '/')
			index--;

		size_t start_pos = ++index;

		out.seekp(sizeof(size_t), ios::beg);
		
		size_t dirs_number = count_dirs_number(name, out, start_pos);

		size_t cur_position = static_cast<size_t>(out.tellp());

		out.clear();
		out.seekp(0, ios::beg);
		out.write((char*)& dirs_number, sizeof(dirs_number));
		
		out.clear();
		out.seekp(cur_position, ios::beg);

		size_t files_number = count_files_number(name);

		out.write((char*)& files_number, sizeof(files_number));

		size_t position_in_file = static_cast<size_t>(out.tellp());
		
		out.seekp(cur_position + sizeof(size_t) + files_number * sizeof(long long), ios::beg);
		recursive_traversal_dirs(out, name, start_pos, position_in_file);

		out.close();
	}
}

void extract(char* name)
{
	ifstream in;

	in.open(name, ios::binary);
	if (in.fail())
		throw exception("Cannot open archive!");

	size_t dirs_number;
	in.read((char*)& dirs_number, sizeof(dirs_number));

	while (dirs_number != 0)
	{
		size_t name_sz;
		in.read((char*)& name_sz, sizeof(name_sz));

		char dir_name[200];
		char cur_symbol;
		for (size_t i = 0; i < name_sz; i++)
		{
			in.read(&cur_symbol, sizeof(cur_symbol));
			dir_name[i] = cur_symbol;
		}
		dir_name[name_sz] = '\0';

		_mkdir(dir_name);

		dirs_number--;
	}

	ofstream out;

	size_t files_start_pos = static_cast<size_t>(in.tellg());

	size_t files_number;
	in.read((char*)& files_number, sizeof(files_number));

	long long cur_file_start;
	long long next_file_start;
	size_t consecutive_file = 0;

	while (consecutive_file < files_number)
	{
		in.clear();

		in.seekg(files_start_pos + sizeof(size_t) + consecutive_file * sizeof(long long), ios::beg);
		in.read((char*)& cur_file_start, sizeof(long long));

		if (consecutive_file < files_number - 1)
			in.read((char*)& next_file_start, sizeof(long long));
		else
		{
			in.seekg(0, ios::end);
			next_file_start = static_cast<long long>(in.tellg());
		}

		in.clear();
		in.seekg(cur_file_start, ios::beg);

		size_t name_sz;
		in.read((char*)& name_sz, sizeof(name_sz));

		char file_name[300];
		in.read(file_name, name_sz);
		file_name[name_sz] = '\0';

		out.open(file_name, ios::binary);
		if (out.fail())
		{
			out.close();
			consecutive_file++;
			continue;
		}

		extract_archive_inner(in, out, next_file_start);
		out.close();

		std::cout << file_name << " is extracted..." << std::endl;

		consecutive_file++;
	}
	
	in.close();
}



#endif _GLOBAL_HEADER_
