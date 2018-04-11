#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <ctime>
#include <cstdlib>

using namespace std;

struct node
{
    int key;
    int degree;
    int number;
    struct node* child;
    struct node* parent;
    struct node* left; // pointer to left sibling in a linked list
    struct node* right;
    bool mark;
};

// initialization
node* heap_min = NULL;
int heap_n = 0; // heap size
node** arr_n; // array of all inserted nodes
long long steps; // number of steps
long long extract_min_n; // number of DELETE MIN operations
bool standard = true; // mark for the implementation (true = standard fibonacci heap, false = naive fibonacci heap)

inline void add2list(node* x, node* y, int i)
{
    // insert x to the right of y
    if (y->right != y)
    {
        x->right = y->right;
        y->right->left = x;
    }
    else
    {
        x->right = x;
    }
    x->left = y;
    y->right = x;

    if (i == 1){
        ++steps;
    }
}

inline void insert(int elm, int key)
{
    node* new_node = (node*) malloc(sizeof(node));
    new_node->key = key;
    new_node->degree = 0;
    new_node->number = elm;
    new_node->child = NULL;
    new_node->parent = NULL;
    new_node->mark = false;

    if (heap_min == NULL)
    {
        heap_min = new_node;
        new_node->left = new_node; // the only child in a root linked list
        new_node->right = new_node;
    }
    else
    {
        add2list(new_node, heap_min, 0);
        if (new_node->key < heap_min->key)
        {
            heap_min = new_node;
        }
    }
    arr_n[elm] = new_node;
    ++heap_n;
}

inline void remove_from_list(node* x)
{
    if (x->left != x and x->right != x)
    {
        x->left->right = x->right;
        x->right->left = x->left;
    }
    else if (x->left != x)
    {
        x->left->right = x->left;
    }
    else if (x->right != x)
    {
        x->right->left = x->right;
    }
}

inline void link(node* y, node* x)
{
    remove_from_list(y);
    y->left = y;
    y->right = y;
    y->parent = x;
    // link to other children if present
    if (x->child == NULL)
    {
        x->child = y;
    }
    else
    {
        add2list(y, x->child, 0);
    }
    ++x->degree;
    y->mark = false;
    ++steps;
}

inline void similar_degree_search(node* x, node** arr)
{
    int deg = x->degree;
    node* y = arr[deg];
    if (x != y) {
        while (arr[deg] != NULL) {
            node* y = arr[deg];
            if (x->key > y->key) {
                node* tmp_x = x;
                x = y;
                y = tmp_x;
            }
            link(y, x);
            arr[deg] = NULL;
            ++deg;
        }
        arr[deg] = x;
    }
}

inline void cons_right(node* x, node** arr)
{
    node* old_right = x->right;
    similar_degree_search(x, arr);
    if (old_right != x)
    {
        cons_right(old_right, arr);
    }
}

inline void cons_left(node* x, node** arr)
{
    node* old_left = x->left;
    similar_degree_search(x, arr);
    if (old_left != x)
    {
        cons_left(old_left, arr);
    }
}

inline void consolidate()
{
    // create an array of degrees to track similar degrees
    node** degree_array = (node**) malloc(32 * sizeof(node*));

    for (int i = 0; i < 32; i++)
    {
        degree_array[i] = NULL;
    }

    node* old_min_left = heap_min->left;
    node* old_min_right = heap_min->right;

    // for each node in the root list
    // simulation of list traversal
    similar_degree_search(heap_min, degree_array);
    if (old_min_left != heap_min) {
        cons_left(old_min_left, degree_array);
    }
    if (old_min_right != heap_min) {
        cons_right(old_min_right, degree_array);
    }

    heap_min = NULL;

    for (int i = 0; i < 32; i++)
    {
        if (degree_array[i] != NULL)
        {
            if (heap_min == NULL)
            {
                // create a root list for containing just one node
                degree_array[i]->right = degree_array[i];
                degree_array[i]->left = degree_array[i];
                heap_min = degree_array[i];
            }
            else
            {
                add2list(degree_array[i], heap_min, 0);
                if (degree_array[i]->key < heap_min->key)
                {
                    heap_min = degree_array[i];
                }
            }
        }
    }
    free(degree_array);
    degree_array = NULL;
}

inline void traverse_right(node* x)
{
    node* old_right = x->right;
    add2list(x, heap_min, 1);

    heap_min->child = old_right;
    heap_min->child->left = heap_min->child;
    x->parent = NULL;

    if (old_right != x)
    {
        traverse_right(old_right);
    }
}

inline void traverse_left(node* x)
{
    node* old_left = x->left;
    add2list(x, heap_min, 1);

    heap_min->child = old_left;
    heap_min->child->left = heap_min->child;

    x->parent = NULL;

    if (old_left != x)
    {
        traverse_left(old_left);
    }
}

inline void extract_min()
{
    if (heap_min != NULL)
    {
        if (heap_min->child) {
            node *old_right = heap_min->child->right;
            node *old_left = heap_min->child->left;
            node* old_child = heap_min->child;

            add2list(heap_min->child, heap_min, 1);

            heap_min->child = old_right;
            heap_min->child->left = heap_min->child;
            heap_min->right->parent = NULL;

            // add to the root list every child of heap_min
            if (heap_min->child != old_child) {
                traverse_right(heap_min->child);
                if (old_child != old_left){
                    traverse_left(old_left);
                }
            }
            heap_min->child = NULL;
        }

        remove_from_list(heap_min);
        // if heap min is the only node
        if (heap_min->right == heap_min and heap_min->left == heap_min)
        {
            heap_min = NULL;
        }
        else
        {
            // replace heap min somehow
            int old_min_number = heap_min->number;
            if (heap_min->right != heap_min){
                heap_min = heap_min->right;
            }
            else{
                heap_min = heap_min->left;
            }
            free(arr_n[old_min_number]);
            arr_n[old_min_number] = NULL;
            --heap_n;
            consolidate();
        }
        ++extract_min_n;
    }
}

inline void cut(node* x)
{
    --x->parent->degree;
    node* old_right = x->right;
    node* old_left = x->left;
    remove_from_list(x);

    if (x->parent->child == x) {
        if (x != old_left) {
            x->parent->child = old_left;
        } else if (x != old_right) {
            x->parent->child = old_right;
        } else {
            x->parent->child = NULL;
        }
    }

    add2list(x, heap_min, 0);
    x->parent = NULL;
    x->mark = false;
}

inline void cascade_cut(node* y)
{
    if (y->parent != NULL)
    {
        if (y->mark == false)
        {
            y->mark = true;
        }
        else
        {
            node* old_parent = y->parent;
            cut(y);
            cascade_cut(old_parent);
        }
    }
}

inline void decrease_key(int elm, int new_key)
{
    node* x = arr_n[elm];
    x->key = new_key;

    // if min-heap property is violated
    if (x->parent != NULL and x->key < x->parent->key)
    {
        node* old_parent = x->parent;
        cut(x);
        if (standard){
            cascade_cut(old_parent);
        }
    }

    if (x->key < heap_min->key)
    {
        heap_min = x;
    }
}

inline void discard_heap()
{
    for (int i = 0; i < heap_n; ++i)
    {
        free(arr_n[i]);
        arr_n[i] = NULL;
    }
    free(arr_n);
    arr_n = NULL;
}

void parse_input()
{
    string s;
    int elm;
    int key;

    clock_t begin_time = clock();

    int old_elm;

	//FILE * file = popen("gen.exe -s 51 -x", "r");
    //char str[100];
    //while (fgets(str, sizeof(str)-1, file) != NULL)
    
    for (string str; getline(cin, str);)
    {
        istringstream iss(str);
        iss >> s >> elm >> key;

        if (s == "#")
        {
            //cout << "DISCARD " << elm << endl;
            discard_heap();
            arr_n = (node**) malloc(elm * sizeof(node*));
            heap_min = NULL;
            heap_n = 0;
            cout << steps <<endl;
            cout << (1.0*steps) / extract_min_n  << " " << old_elm << endl;
            steps = 0;
            extract_min_n = 0;
            old_elm = elm;
        }
        else if (s == "INS")
        {
            //cout << "INSERT " << elm << " " << key << endl;
            insert(elm, key);
        }
        else if (s == "DEC")
        {
            //cout << "DECREASE KEY " << elm << " " << key << endl;
            if (arr_n[elm] and arr_n[elm]->key > key)
            {
                decrease_key(elm, key);
            }
            /*
        else{
            cout << "IGNORED" << endl;
        }
             */
        }
        else if (s == "DEL")
        {
            //cout << "DELETE MIN " << endl;
            extract_min();
        }
    }
    cout << (1.0*steps) / extract_min_n  << " " << old_elm << endl;
    double dur = double(clock() - begin_time) / CLOCKS_PER_SEC;
    cout << "Duration: " << dur << endl;
}

int main()
{
    parse_input();
    return 0;
}
