#include "splay_tree.h"
#include <string>
#include <fstream>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <ctime>

using namespace std;

//int path; // length of path

inline node* splay (node* for_splay)
{
	if (for_splay->parent != NULL)
	{
		if (for_splay->parent->left_child == for_splay) { zig_right(for_splay->parent); }
		else { zig_left(for_splay->parent);	}
		splay(for_splay);	
	}
	else { return for_splay; }
}

int main()
{
	int num;
	int n; // total number of elements
	int f; // total number of find operations
	string s;
	node* root = NULL;
	int begin = 1;
	clock_t begin_time = clock();

	for (string str; getline(cin, str);)
	{
		if (str[0] == '#')
		{
			if (root != NULL)
			{
				discard_tree(root);
				root = NULL;
				begin = 1;
			}
			istringstream iss(str); // read input string
			iss >> s >> n; // remember string and number
			cout << (1.0*path) / f << " " << n << endl; // return the avg path
			path = 0;
			f = 0;
		}
		else if (str[0] == 'I')
		{	
			istringstream iss(str);
			iss >> s >> num;
			if (begin == 1) // check if it is a new tree
			{
				root = (node*)malloc(sizeof(node));
				root->key = num;
				root->parent = NULL;
				root->left_child = NULL;
				root->right_child = NULL;
				begin = 0;
			}
			else
			{			
				node* inserted = insert(num, root);
				splay(inserted);
				root = inserted;
			}
		}
		else if (str[0] == 'F')
		{
			istringstream iss(str);
			iss >> s >> num;
			++f;
			node* found = search(num, root);
			splay(found);
			root = found;
		}	
		// printing the tree
		//traverse(root);
		//cout << endl;
		//cout << "----------------------------" << endl;
	}
	double dur = double(clock() - begin_time) / CLOCKS_PER_SEC;
	cout << "Duration: " << dur << endl;
	return 0;
}