#ifndef _PRIORITY_QUEUE_HEADER_

#define _PRIORITY_QUEUE_HEADER_

#include "huffman_tree.h"

//class used for building the Huffman_Tree using Huffman's algorithm

class priority_queue
{
private:
	void copy_from(priority_queue const&);

private:
	huffman_tree* container;
	size_t container_size;
	size_t container_used;

public:
	priority_queue();
	priority_queue(priority_queue const&);
	priority_queue& operator=(priority_queue const&);
	~priority_queue();
	void extend_queue();
	void enqueue(huffman_tree&);
	huffman_tree& dequeue();

	bool is_empty() const;
	size_t get_used() const;

	void fill_queue(int*&);
};

inline void priority_queue::copy_from(priority_queue const& other)
{
	this->container = new huffman_tree[other.container_size];

	for (size_t i = 0; i < other.container_used; i++)
		this->container[i] = other.container[i];

	this->container_used = other.container_used;
	this->container_size = other.container_size;
}

inline priority_queue::priority_queue() : container(new huffman_tree[2]), container_size(2), container_used(0) {}

inline priority_queue::priority_queue(priority_queue const& other)
{
	this->copy_from(other);
}

inline priority_queue& priority_queue::operator=(priority_queue const& other)
{
	if (this != &other)
	{
		delete[] this->container;

		this->copy_from(other);
	}
	return *this;
}

inline priority_queue::~priority_queue()
{
	delete[] this->container;
}

inline bool priority_queue::is_empty() const
{
	return this->container_used == 0;
}

//enqueuing has to put the new(enqueued) element on its place
//the queue is sorted all the time

void priority_queue::enqueue(huffman_tree& tree)
{
	if (this->is_empty())
	{
		this->container[this->container_used++] = tree;
		return;
	}

	this->extend_queue();

	size_t i = 0;
	while (i < this->container_used && this->container[i].root->frequency > tree.root->frequency)
	{
		i++;
	}
	this->container_used++;
	
	if (i == this->container_used - 1)
		this->container[i] = tree;
	else
	{
		size_t j = this->container_used - 1;

		while (j > i)
		{
			this->container[j] = this->container[j - 1];
			j--;
		}
		this->container[i] = tree;
	}
}

huffman_tree& priority_queue::dequeue()
{
	return this->container[--this->container_used];
}

void priority_queue::extend_queue()
{
	if (this->container_used == this->container_size - 1)
	{
		this->container_size *= 2;

		huffman_tree* newQueue = new huffman_tree[this->container_size];

		for (size_t i = 0; i < this->container_used; i++)
			newQueue[i] = this->container[i];

		delete[] this->container;
		this->container = newQueue;
	}
}

inline size_t priority_queue::get_used() const 
{
	return this->container_used;
}

//once we have all the info about the data we are encoding(characters used and their frequencies), 
//we fill the queue so that it can be ready for the tree construction

void priority_queue::fill_queue(int*& arr)
{
	size_t i = 0;
	for (i; i < 128; i++)
	{
		if (arr[i] != 0)
		{
			char c = static_cast<char>(i);

			huffman_tree t;
			t.root = new huffman_node(arr[i], c);

			this->enqueue(t);
		}
	}
	for (i; i < 256; i++)
	{
		if (arr[i] != 0)
		{
			char c = static_cast<char>(127 - i);

			huffman_tree t;
			t.root = new huffman_node(arr[i], c);

			this->enqueue(t);
		}
	}
}


#endif _PRIORITY_QUEUE_HEADER_