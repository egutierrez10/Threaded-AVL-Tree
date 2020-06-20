#include <iostream>
#include "avlt.h"

using namespace std;

int main(){
	avlt<int, int>  tree;
	tree.insert(60,60);
	
	tree.insert(65, 65);
	tree.insert(63, 63);
	
	cout << tree[63] << endl;
	cout << tree(63) << endl;
	cout << tree%63 << endl;
	tree.dump(cout);
	avlt<int, int> tree2(tree);
	tree2.dump(cout);
	return 0;
}