#ifndef _GLOBAL_HEADER_

#define _GLOBAL_HEADER_

#include "priority_queue.h"
#include "hash.h"
#include <fstream>
#include "direct.h"

char* create_archive_name(char* name, char* postf)
{
	char* n_name = new char[256];
	size_t n_sz = strlen(name);
	size_t postf_sz = strlen(postf);

	size_t i = 0;
	for (i; i < n_sz; i++)
		n_name[i] = name[i];

	for (i; i < n_sz + postf_sz; i++)
		n_name[i] = postf[i - n_sz];

	n_name[i] = '\0';

	return n_name;
}

char* decompressed_name(char* name)
{
	size_t index = strlen(name) - 1;
	
	while (index > 0 && name[index] != '_')
		index--;

	char* n_name = new char[strlen(name) - 7];

	size_t i = 0;
	for (i; i < index; i++)
		n_name[i] = name[i];

	n_name[i] = '\0';

	return n_name;
}

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

void archivate_file(char* name)
{
	ofstream out;
	out.open(create_archive_name(name, "_arch.bin"), ios::binary);
	if (out.fail())
		throw exception("Cannot open file!");

	int f_num = 1;
	out.write((char*)& f_num, sizeof(f_num));
	
	archivate_file_inner(out, name);

	out.close();
}

//extracts the content of an archive with a given name

void extract_archive(char* name)
{
	ifstream in(name, ios::binary);
	if (in.fail())
		throw exception("Cannot open file!");

	//reading the size of the original file
	size_t file_sz;
	in.seekg(-4, ios::end); 
	in.read((char*)& file_sz, sizeof(file_sz));
	
	in.clear();
	in.seekg(0, ios::beg);

	int* arr = new int[257];
	for (size_t i = 0; i < 256; i++)
	{
		in.read((char*)& arr[i], sizeof(arr[i]));
	}

	priority_queue q;
	q.fill_queue(arr);

	huffman_tree t;
	t = create_huffman_tree(q);
	
	ofstream out(decompressed_name(name), ios::binary);
	if (out.fail())
		throw exception("Cannot open file!");

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
	
	out.close();
	in.close();
}

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

void recursive_traversal_dirs(ofstream& out, char* name)
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
			recursive_traversal_dirs(out, n_name);

		else if (box->d_name[0] != '.' && is_regular(n_name))
		{
			out.clear();

			long long cur_position = static_cast<long long>(out.tellp());
			out.seekp(sizeof(int) + consecutive_file * sizeof(long long), ios::beg);
			int t = static_cast<int>(out.tellp());
			out.write((char*)& cur_position, sizeof(long long));
			
			out.clear();
			out.seekp(cur_position, ios::beg);
			consecutive_file++;

			size_t name_sz = strlen(box->d_name);
			out.write((char*)& name_sz, sizeof(name_sz));
			out.write((char*)& box->d_name, name_sz);
			
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

	if (is_regular(name))
	{
		size_t files_number = 1;
		size_t name_sz = strlen(name);
		long long start_position = sizeof(files_number) + sizeof(long long);
	
		out.write((char*)& files_number, sizeof(files_number));
		out.write((char*)& start_position, sizeof(start_position));
		out.write((char*)& name_sz, sizeof(name_sz));
		out.write(name, name_sz);

		archivate_file_inner(out, name);
	}

	else if (is_directory(name))
	{
		size_t files_number = count_files_number(name);

		out.write((char*)& files_number, sizeof(files_number));
		out.seekp(sizeof(int) + files_number * sizeof(long long), ios::beg);

		recursive_traversal_dirs(out, name);
	}

	delete[] arch_name;
	out.close();
}


void extract(char* name)
{
	ofstream out;
	
	ifstream in;
	in.open(name, ios::binary);
	if (in.fail())
		throw exception("Cannnot read file!");

	size_t files_number;
	in.read((char*)& files_number, sizeof(files_number));
	
	long long cur_file_start;
	long long next_file_start;
	size_t consecutive_file = 0;

	while (consecutive_file < files_number)
	{
		in.clear();

		in.seekg(sizeof(int) + consecutive_file * sizeof(long long), ios::beg);
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
		in.read((char*)& file_name, name_sz);
		file_name[name_sz] = '\0';
		
		out.open(file_name, ios::binary);
		if (out.fail())
			throw exception("Cannot open file!");

		extract_archive_inner(in, out, next_file_start);
		out.close();

		std::cout << file_name << " is extracted..." << std::endl;

		consecutive_file++;
	}

	in.close();
}



#endif _GLOBAL_HEADER_
