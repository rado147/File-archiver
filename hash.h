#ifndef _HASH_HEADER_

#define _HASH_HEADER_

#define hash_size 257

#include "huffman_tree.h"

using namespace std;

//not really a hash;
//provides constant time access to the paths required for encoding

class simple_hash
{
public:
	char* table[hash_size];

public:
	simple_hash();
	~simple_hash();

	void fill_table(huffman_node*, char*&, int = 0);
	void get_char_path(char) const;
};

inline simple_hash::simple_hash()
{
	for (size_t i = 0; i < hash_size; i++)
		this->table[i] = nullptr;
}

inline simple_hash::~simple_hash()
{
	for (size_t i = 0; i < hash_size; i++)
		delete[] this->table[i];
}

//recursive function examing and saving all the require paths(from the root to all the leaves)

void simple_hash::fill_table(huffman_node* node, char*& path, int index)
{
	if (node->left == nullptr && node->right == nullptr)
	{
		size_t cell;
		if (static_cast<int>(node->character) < 0)
		{
			cell = 127 - static_cast<int>(node->character);
		}
		else
			cell = static_cast<size_t>(node->character);

		this->table[cell] = new char[index + 1];
		
		int i = 0;
		for (i; i < index; i++)
			this->table[cell][i] = path[i];

		this->table[cell][i] = '\0';

		return;
	}
	else
	{
		path[index] = '0';
		fill_table(node->left, path, ++index);
		index--;
		path[index] = '1';
		fill_table(node->right, path, ++index);
		index--;

	}
}


void simple_hash::get_char_path(char c) const
{
	if (static_cast<int>(c) < 0)
	{
		std::cout << this->table[127 - static_cast<int>(c)];// << std::endl;
		return;
	}
	std::cout << this->table[static_cast<size_t>(c)]; // << std::endl;
}


#endif _HASH_HEADER_
