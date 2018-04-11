#define SPLAY_TREE_H

#include <string>
#include <fstream>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <ctime>

using namespace std;

int path;

struct node 
{
	int key;
	struct node* left_child;	
	struct node* right_child;
	struct node* parent;
};

inline node* zig_right(node* p)
{
	node* x = p->left_child;
	p->left_child = x->right_child;
	if (x->right_child != NULL)
	{
		x->right_child->parent = p;
	}
	x->right_child = p;
	x->parent = p->parent;
	if (p->parent != NULL)
	{
		if (p->parent->left_child == p)
		{
			p->parent->left_child = x;
		}
		else
		{
			p->parent->right_child = x;
		}
	}
	p->parent = x;
	return x;
}

inline node* zig_left(node* p)
{
	node* x = p->right_child;
	p->right_child = x->left_child;
	if (x->left_child != NULL)
	{
		x->left_child->parent = p;
	}
	x->left_child = p;
	x->parent = p->parent;
	if (p->parent != NULL)
	{
		if (p->parent->left_child == p)
		{
			p->parent->left_child = x;
		}
		else
		{
			p->parent->right_child = x;
		}
	}
	p->parent = x;
	return x;
}

inline node* search(int key, node* start)
{
	if (key < start->key)
	{
		++path;
		return search(key, start->left_child); 
	}
	else if (key > start->key)
	{
		++path;
		return search(key, start->right_child);
	}
	else
	{
		return start;
	}
}

inline node* insert(int key, node* start)
{
	if (key < start->key)
	{
		if (start->left_child == NULL)
		{
			node* new_node = (node*) malloc(sizeof(node));
			new_node->key = key;
			start->left_child = new_node;
			new_node->parent = start;
			new_node->left_child = NULL;
			new_node->right_child = NULL;
			return new_node;
		}
		else { insert(key, start->left_child); }
	}
	else if (key >= start->key)
	{
		if (start->right_child == NULL)
		{
			node* new_node = (node*) malloc(sizeof(node));
			new_node->key = key;
			start->right_child = new_node;
			new_node->parent = start;
			new_node->left_child = NULL;
			new_node->right_child = NULL;
			return new_node;
		}
		else { insert(key, start->right_child); }
	}
}

inline void discard_tree (node* for_delete)
{
	if (for_delete != NULL)
	{
		if (for_delete->left_child != NULL) { discard_tree(for_delete->left_child); }
		if (for_delete->right_child != NULL) { discard_tree(for_delete->right_child); }
		free(for_delete);
		for_delete = NULL;
	}
}

void traverse(node *root) 
{
	if (root != NULL)
	{
		cout << root->key << " ";
		cout << root->key << "l ";
		traverse(root->left_child);
		cout << root->key << "r ";
		traverse(root->right_child);	
	}
}
