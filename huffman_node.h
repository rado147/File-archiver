#ifndef _HUFFMAN_NODE_

#define _HUFFMAN_NODE_

//box class used to build the tree for encoding and decoding

class huffman_node
{
public:
	//the character of a node and its frequency

	char character;
	size_t frequency;

public:
	huffman_node* left;
	huffman_node* right;

public:
	huffman_node() : frequency(0), character('\0'), left(nullptr), right(nullptr) {}
	huffman_node(size_t f, char c) : frequency(f), character(c), left(nullptr), right(nullptr) {}

};





#endif _HUFFMAN_NODE_