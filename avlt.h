/*avlt.h*/

//
// Threaded AVL tree
//

#pragma once

#include <iostream>
#include <vector>
#include <stack>
#include <algorithm>  

using namespace std;

template<typename KeyT, typename ValueT>
class avlt
{
private:
  struct NODE
  {
    KeyT   Key;
    ValueT Value;
    NODE*  Left;
    NODE*  Right;
    bool   isThreaded; // true => Right is a thread, false => non-threaded
    int    Height;     // height of tree rooted at this node
  };

  NODE* Root;  // pointer to root node of tree (nullptr if empty)
  int   Size;  // # of nodes in the tree (0 if empty)
  NODE* state; // used by threaded inorder traversal to denote next key
  
  
  // helper function to return height of node passed as argument
  int _getHeight(NODE* cur){
      if(cur == nullptr)
         return -1;
      else
         return cur->Height;  
   }
  //
  // helper functions to return the Left and Right pointers; in
  // particular the ptr returned for the Right is controlled by 
  // the "isThreaded" field:  if "isThreaded" is true then nullptr
  // pointer is returned, otherwise the actual underyling ptr
  // is returned.
  //
  NODE* _getActualLeft(NODE* cur) const
  {
    return cur->Left;
  }

  NODE* _getActualRight(NODE* cur) const
  {
    if (cur->isThreaded)  // then actual Right ptr is null:
      return nullptr;
    else  // actual Right is contents of Right ptr:
      return cur->Right;
  }
  
  // 
  // _clearTree:
  // Parameters: Pointer to root node of "this" tree
  // Helper function to recursively delete each node in the tree from memory by performing a postorder traversal.
  //
  void _clearTree(NODE* cur){
	if(cur == nullptr)
		  return;
	  else{
		  _clearTree(cur->Left);
		  if(cur->isThreaded)
			  _clearTree(nullptr);
		  else
			  _clearTree(cur->Right);
		  delete cur;
	  }
	}
	
  // 
  // _dump:
  // Parameters: Pointer to root node of "this" tree
  // Helper function to recursively dump out (key, value) pairs from each node in the tree by performing a inorder traversal.
  // A different value is to be printed out depending on if node is threaded or not. 
  //
	void _dump(ostream& output, NODE* cur) const
  {
    if (cur == nullptr)
      return;
    else
    {
      
    // inorder traversal, with one output per line: either 
    // (key,value,height) or (key,value,height,THREAD)
    //
    // (key,value,height) if the node is not threaded OR thread==nullptr
    // (key,value,height,THREAD) if the node is threaded and THREAD denotes the next inorder key
      _dump(output, _getActualLeft(cur));

      output << "(" << cur->Key << "," << cur->Value << "," << cur->Height;

      if ((!cur->isThreaded) || (cur->Right == nullptr))
      {
        output << ")" << endl;
      }
      else
      {
        output << "," << cur->Right->Key << ")" << endl;
      }

      _dump(output, _getActualRight(cur));
    }
  }
  
  //
  // _LeftRotate
  //
  // Rotates the tree around the node N, where Parent is N's parent.  Note that
  // Parent could be null, which means N is the root of the entire tree.  If 
  // Parent denotes a node, note that N could be to the left of Parent, or to
  // the right.  You'll need to take all this into account when linking in the
  // new root after the rotation.  Don't forget to update the heights as well.
  //
	void _LeftRotate(NODE* Parent, NODE* N)
  {
      //Lable components
     NODE* R = N->Right;
     NODE* A = N->Left;
     NODE* B = R->Left;
     NODE* C = R->Right;
     int BH;
     //Rotate
     R->Left = N;
     if(B == nullptr){
		 BH = -1;
		 N->Right = R;
		 N->isThreaded = true;
	 }else{
		 N->Right = B;
		 BH = _getHeight(B);
	 }
     
     //update parent 
     if(Parent == nullptr)
       Root = R;
      else if(Parent->Key > R->Key)
         Parent->Left = R;
      else 
         Parent->Right = R;
      
     //Update N height 
      N->Height = 1 + max(_getHeight(A), BH);
     //Update L height 
     R->Height = 1 + max(_getHeight(N), _getHeight(C));
  }
  
  //
  // _RightRotate
  //
  // Rotates the tree around the node N, where Parent is N's parent.  Note that
  // Parent could be null, which means N is the root of the entire tree.  If 
  // Parent denotes a node, note that N could be to the left of Parent, or to
  // the right.  You'll need to take all this into account when linking in the
  // new root after the rotation.  Don't forget to update the heights as well.
  //
  void _RightRotate(NODE* Parent, NODE* N)
  {
     //Lable components
     NODE* L = N->Left;
     NODE* A = L->Left;
     NODE* B = L->Right;
     NODE* C = N->Right;
     int BH, CH;
     //Rotate
     L->Right = N;
	 if(L->isThreaded){
		 N->Left = nullptr;
		 BH = -1;
	  }else{
		N->Left = B;
		BH = _getHeight(B);
	  }
		L->isThreaded = false;
	if(N->isThreaded)
		CH = -1;
	else
		CH = _getHeight(C);
		
     //update parent 
     if(Parent == nullptr)
       Root = L;
      else if(Parent->Key > L->Key)
         Parent->Left = L;
      else 
         Parent->Right = L;
      
     //Update N height 
      N->Height = 1 + max(BH, CH);
     //Update L height 
     L->Height = 1 + max(_getHeight(A), _getHeight(N));
  }
  
  // 
  // _search:
  // Parameters: Pointer to current node(Root), lower boundary, upper boundary, vector to hold keys
  // Helper function to find all keys that fall within lower and upper boundaries. Function performs inorder traversal 
  // heading left until lower boundary is surpassed or encountered. Checks if key falls within boundaries to push to vector beore
  // heading right until upper boundary is surpassed or encountered.
  // 
	void _search(NODE* cur, KeyT lower, KeyT upper, vector<KeyT>& keys){
		if(cur == nullptr)
			return;
		else{
			if(cur->Key >= lower) // recursive call as long as current key is greater than lower bound
				_search(cur->Left, lower, upper, keys);
			if(cur->Key >= lower && cur->Key <= upper) //check to see if current key falls between lower and upper bound
				keys.push_back(cur->Key);
			if(cur->Key <= upper){ //recursive call as long as current key is less than upper bound
				if(cur->isThreaded)
					_search(nullptr, lower, upper, keys); //if current node is threaded dont head right, function will return back
				else
					_search(cur->Right, lower, upper, keys); //node is not threaded, head right calling function recursively
			}
		}
	}
	
	void _insertNode(KeyT key, ValueT value, int height){
	
	NODE* prev = nullptr;
    NODE* cur = Root;
    //
    // 1. Search to see if tree already contains key:
    //
    while (cur != nullptr)
    {
      if (key == cur->Key)  // already in tree
        return;
      if (key < cur->Key)  // search left:
      {
        prev = cur;
        cur = _getActualLeft(cur);
      }
      else
      {
        prev = cur;
        cur = _getActualRight(cur);
      }
    }//while

    //
    // 2. if we get here, key is not in tree, so allocate
    // a new node to insert:
    // 
    NODE* n = new NODE();
    n->Key = key;
    n->Value = value;
    n->Left = nullptr;
    n->Right = nullptr;
	n->Height = height;
    n->isThreaded = true;

    //
    // 3. link in the new node:
    //
    //
    if (prev == nullptr){ //node is to be the root of the tree
	  n->Right = nullptr;	//node is threaded to nullptr
      Root = n;
    }else if (key < prev->Key){ //node is assigned to the left of the parent node
	  prev->Left = n; 
	  n->Right = prev; //node is threaded to previous node
    }else{
	  n->Right = prev->Right; //node is threaded to whatever the previous node was threading to
      prev->Right = n; 
	  prev->isThreaded = false; 
	 }
	 
	}
	
	void _insert(NODE* cur){
		if(cur == nullptr)
			return;
		else{
			_insertNode(cur->Key, cur->Value, cur->Height);
			_insert(cur->Left);
			if(cur->isThreaded)
				_insert(nullptr);
			else
				_insert(cur->Right);
		}
	}
	
public:
  //
  // default constructor:
  //
  // Creates an empty tree.
  //
  avlt()
  {
    Root = nullptr;
    Size = 0;
	state = nullptr;
  }

  //
  // copy constructor
  //
  // NOTE: makes an exact copy of the "other" tree, such that making the
  // copy requires no rotations.
  //
  avlt (const avlt& other)
  {
    //
    // TODO::DONE
    //
    Root = nullptr;
    Size = other.Size;
    state = nullptr;
	//call to copy helper function
	_insert(other.Root);
	state = other.state;
  }

	//
  // destructor:
  //
  // Called automatically by system when tree is about to be destroyed;
  // this is our last chance to free any resources / memory used by
  // this tree.
  //
  virtual ~avlt()
  {
    //
    // TODO::DONE
    //
    _clearTree(Root);
  }

  //
  // operator=
  //
  // Clears "this" tree and then makes a copy of the "other" tree.
  //
  // NOTE: makes an exact copy of the "other" tree, such that making the
  // copy requires no rotations.
  //
  avlt& operator=(const avlt& other)
  {
    //
    // TODO:
    //
	clear();
	//call to helper 
	_insert(other.Root);
	Size = other.Size;
	state = other.state;
    return *this;
  }

  //
  // clear:
  //
  // Clears the contents of the tree, resetting the tree to empty.
  //
  void clear()
  {
    //
    // TODO::DONE
    //
    _clearTree(Root);
	Root = nullptr;
    Size = 0;
    state = nullptr;
  }

  // 
  // size:
  //
  // Returns the # of nodes in the tree, 0 if empty.
  //
  // Time complexity:  O(1) 
  //
  int size() const
  {
    return Size;
  }

  // 
  // height:
  //
  // Returns the height of the tree, -1 if empty.
  //
  // Time complexity:  O(1) 
  //
  int height() const
  {
    if (Root == nullptr)
      return -1;
    else
      return Root->Height;
  }

  // 
  // search:
  //
  // Searches the tree for the given key, returning true if found
  // and false if not.  If the key is found, the corresponding value
  // is returned via the reference parameter.
  //
  // Time complexity:  O(lgN) worst-case
  //
  bool search(KeyT key, ValueT& value) const
  {
    //
    // TODO::DONE
    //
    NODE* cur = Root;
	
    while (cur != nullptr)
    {
      if (key == cur->Key)  // already in tree
      {
        value = cur->Value;
        return true;
      }
	  
      if (key < cur->Key)  // search left:
      {	
        cur = _getActualLeft(cur);
      }
      else
      {
        cur = _getActualRight(cur);
      }
    }//while  
	
    // if get here, not found
    return false;
  }

  //
  // range_search
  //
  // Searches the tree for all keys in the range [lower..upper], inclusive.
  // It is assumed that lower <= upper.  The keys are returned in a vector;
  // if no keys are found, then the returned vector is empty.
  //
  // Time complexity: O(lgN + M), where M is the # of keys in the range
  // [lower..upper], inclusive.
  //
  // NOTE: do not simply traverse the entire tree and select the keys
  // that fall within the range.  That would be O(N), and thus invalid.
  // Be smarter, you have the technology.
  //
  vector<KeyT> range_search(KeyT lower, KeyT upper)
  {
    //
    // TODO::DONE
    //
	vector<KeyT> keys;
	_search(Root, lower, upper, keys); //call to helper function for getting keys between lower and upper bounds
	return keys;
  }

  //
  // insert
  //
  // Inserts the given key into the tree; if the key has already been insert then
  // the function returns without changing the tree.  Rotations are performed
  // as necessary to keep the tree balanced according to AVL definition.
  //
  // Time complexity:  O(lgN) worst-case
  //
  void insert(KeyT key, ValueT value)
  {
    //
    // TODO::DONE
    //
	NODE* prev = nullptr;
    NODE* cur = Root;
	stack<NODE*> nodes;
    //
    // 1. Search to see if tree already contains key:
    //
    while (cur != nullptr)
    {
      if (key == cur->Key)  // already in tree
        return;
	  nodes.push(cur);
      if (key < cur->Key)  // search left:
      {
        prev = cur;
        cur = _getActualLeft(cur);
      }
      else
      {
        prev = cur;
        cur = _getActualRight(cur);
      }
    }//while

    //
    // 2. if we get here, key is not in tree, so allocate
    // a new node to insert:
    // 
    NODE* n = new NODE();
    n->Key = key;
    n->Value = value;
	n->Height = 0;
    n->Left = nullptr;
    n->Right = nullptr;
    n->isThreaded = true;

    //
    // 3. link in the new node:
    //
    if (prev == nullptr){ //node is to be the root of the tree
	  n->Right = nullptr;	//node is threaded to nullptr
      Root = n;
    }else if (key < prev->Key){ //node is assigned to the left of the parent node
	  prev->Left = n; 
	  n->Right = prev; //node is threaded to previous node
    }else{
	  n->Right = prev->Right; //node is threaded to whatever the previous node was threading to
      prev->Right = n; 
	  prev->isThreaded = false; 
	 }

    // 
    // 4. update size and we're done:
    //
    Size++;
	
	//
	// 5. update heights
	//
	int hL, hR, hCur;
	int diff;
	NODE* next;
	while(!nodes.empty()){
	  cur = nodes.top();
      nodes.pop();
	  
	  if(nodes.empty()) //parent node
		  next = nullptr;
	  else 
		  next = nodes.top();
		  
	  hL = (cur->Left == nullptr)? -1: cur->Left->Height;
	  if(cur->isThreaded)
		  hR = -1;
	  else 
		  hR = cur->Right->Height;
	  if(hL > hR)
		  diff = hL - hR;
	  else 
		  diff = hR - hL;
		  
	  hCur = 1 + max(hL, hR);
	  if (cur->Height == hCur)  // didn't change, so no need to go further:
        break;
	  else if(diff == 0 || diff == 1)// check avl condition, height changed, update and keep going:
		 cur->Height = hCur;
	  else{
		  if(hL > hR){ //left leaning
			  if(n->Key < cur->Left->Key){
				  _RightRotate(next, cur);
			  }else{
				  _LeftRotate(cur, cur->Left);
				  _RightRotate(next, cur);
			  }
		  }else{
			 if(n->Key < cur->Right->Key){
				  _RightRotate(cur, cur->Right);
				  _LeftRotate(next, cur);
			  }else{
				  _LeftRotate(next, cur);
			  }
		  }
		 break;
	  }//end of else
	}//end of while
  }

  //
  // []
  //
  // Returns the value for the given key; if the key is not found,
  // the default value ValueT{} is returned.
  //
  // Time complexity:  O(lgN) worst-case
  //
  ValueT operator[](KeyT key) const
  {
    //
    // TODO::DONE
    //
	ValueT  val = ValueT{};
	this->search(key, val);
    return val;
  }

  //
  // ()
  //
  // Finds the key in the tree, and returns the key to the "right".
  // If the right is threaded, this will be the next inorder key.
  // if the right is not threaded, it will be the key of whatever
  // node is immediately to the right.
  //
  // If no such key exists, or there is no key to the "right", the
  // default key value KeyT{} is returned.
  //
  // Time complexity:  O(lgN) worst-case
  //
  KeyT operator()(KeyT key) const
  {
    //
    // TODO::DONE
    //
	NODE* cur = Root;

    //
    // we have to do a traditional search, and then work from
    // there to follow right / thread:
    //
    while (cur != nullptr)
    {
      if (key == cur->Key)  // found it:
      {
        // 
        // we want the key to the right, either immediately right
        // or the threaded inorder key.  Just need to make sure
        // we have a pointer first:
        if (cur->Right == nullptr)  // threaded but null:
          return KeyT{ };
        else
          return cur->Right->Key;
      }

      if (key < cur->Key)  // search left:
      {
        cur = _getActualLeft(cur);
      }
      else
      {
        cur = _getActualRight(cur);
      }
    }//while  

    return KeyT{ };
  }

  //
  // %
  //
  // Returns the height stored in the node that contains key; if key is
  // not found, -1 is returned.
  //
  // Example:  cout << tree%12345 << endl;
  //
  // Time complexity:  O(lgN) worst-case
  //
  int operator%(KeyT key) const
  {
    //
    // TODO::DONE
    //
	NODE* cur = Root;
	
	while (cur != nullptr)
    {
      if (key == cur->Key)  // found it:
      {
        return cur->Height;
      }

      if (key < cur->Key)  // search left:
      {
        cur = _getActualLeft(cur);
      }
      else
      {
        cur = _getActualRight(cur);
      }
    }//while  
    return -1;
  }

  //
  // begin
  //
  // Resets internal state for an inorder traversal.  After the 
  // call to begin(), the internal state denotes the first inorder
  // key; this ensure that first call to next() function returns
  // the first inorder key.
  //
  // Space complexity: O(1)
  // Time complexity:  O(lgN) worst-case
  //
  // Example usage:
  //    tree.begin();
  //    while (tree.next(key))
  //      cout << key << endl;
  //
  void begin()
  {
    //
    // TODO::DONE
    //
    state = Root;

    if (state != nullptr)  // advance as left as possible:
    {
      while (state->Left != nullptr)
        state = state->Left;
    }
  }

  //
  // next
  //
  // Uses the internal state to return the next inorder key, and 
  // then advances the internal state in anticipation of future
  // calls.  If a key is in fact returned (via the reference 
  // parameter), true is also returned.
  //
  // False is returned when the internal state has reached null,
  // meaning no more keys are available.  This is the end of the
  // inorder traversal.
  //
  // Space complexity: O(1)
  // Time complexity:  O(lgN) worst-case
  //
  // Example usage:
  //    tree.begin();
  //    while (tree.next(key))
  //      cout << key << endl;
  //
  bool next(KeyT& key)
  {
    //
    // TODO::DONE
    //
	if (state == nullptr)
    {
      //
      // no more keys:
      //
      return false;
    }

    //
    // we have at least one more, so grab it now,
    // advance, and return true:
    //
    key = state->Key;

    if (_getActualRight(state) == nullptr)
    {
      // follow the thread:
      state = state->Right;
    }
    else
    {
      state = state->Right;  // go right, and then left as far as we can:
      
      while (state->Left != nullptr)
        state = state->Left;
    }
    return true;
  }

  //
  // dump
  // 
  // Dumps the contents of the tree to the output stream, using a
  // recursive inorder traversal.
  //
  void dump(ostream& output) const
  {
    output << "**************************************************" << endl;
    output << "********************* AVLT ***********************" << endl;

    output << "** size: " << this->size() << endl;
    output << "** height: " << this->height() << endl;

    //
    // inorder traversal, with one output per line: either 
    // (key,value,height) or (key,value,height,THREAD)
    //
    // (key,value,height) if the node is not threaded OR thread==nullptr
    // (key,value,height,THREAD) if the node is threaded and THREAD denotes the next inorder key
    //

    //
    // TODO::DONE
    //
    _dump(output, Root);

    output << "**************************************************" << endl;
  }
	
};
