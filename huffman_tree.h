#ifndef _HUFFMAN_TREE_HEADER_

#define _HUFFMAN_TREE_HEADER_

#include "huffman_node.h"
#include <iostream>

//the tree class that will be used for encoding and decoding

class huffman_tree
{
public:
	huffman_node* root;
private:
	void copy_function(huffman_node*&, huffman_node* const&);
	void chain_deletion(huffman_node*&);

public:
	huffman_tree();
	huffman_tree(huffman_tree const&);
	huffman_tree& operator=(huffman_tree const&);
	~huffman_tree();
	void print_leaves(huffman_node* root) const;
};

inline huffman_tree::huffman_tree() : root(nullptr) {}

void huffman_tree::copy_function(huffman_node*& root, huffman_node* const& other_root)
{
	root = nullptr;
	if (other_root)
	{
		root = new huffman_node(other_root->frequency, other_root->character);
		this->copy_function(root->left, other_root->left);
		this->copy_function(root->right, other_root->right);
	}
}

inline huffman_tree::huffman_tree(huffman_tree const& other)
{
	this->copy_function(this->root, other.root);
}

inline huffman_tree& huffman_tree::operator=(huffman_tree const& other)
{
	if (this != &other)
	{
		this->chain_deletion(this->root);
		this->copy_function(this->root, other.root);
	}

	return *this;
}

inline void huffman_tree::chain_deletion(huffman_node*& root)
{
	if (root != nullptr)
	{
		chain_deletion(root->left);
		chain_deletion(root->right);

		delete root;
		root = nullptr;
	}
}

inline huffman_tree::~huffman_tree()
{
	this->chain_deletion(this->root);
}

//DFS method for printing the leaves
//only the leaves have a certain character other than '\0' and the path from the root to a child is important

void huffman_tree::print_leaves(huffman_node* root) const
{
	if (root)
	{
		if (!root->left && !root->right)
			std::cout << root->character << " ";

		this->print_leaves(root->left);
		this->print_leaves(root->right);
	}
}


#endif _HUFFMAN_TREE_HEADER_
