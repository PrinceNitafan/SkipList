#include "SkipList.h"
#include <iostream>

using namespace std;

SkipList::SkipList() : listHeads() {
    makeNewLevelList();
    makeNewLevelList();
} 

// makes a new list on the top level of existing list.
// call only when top list is NULL or just the two sentinels. 
void SkipList::makeNewLevelList() {
    SkipList::Entry* minusInfinity = new Entry("!!", "");	// "!!" < any other string.
    SkipList::Entry* plusInfinity = new Entry("}}", "");	// "}}" > any other key.

    Quad* first = new Quad(minusInfinity);
    Quad* last = new Quad(plusInfinity);

    int numLists = listHeads.size();
    Quad* oldFirst = numLists == 0 ? NULL : listHeads[numLists - 1];
    Quad* oldLast  = numLists == 0 ? NULL : oldFirst->next;

    first->prev = NULL;
    first->next = last;
    first->above = NULL;
    first->below = oldFirst;

    last->prev = first;
    last->next = NULL;
    last->above = NULL;
    last->below = oldLast;

    if(oldFirst != NULL) {
        oldFirst->above = first;
        oldLast->above = last;
    }
    listHeads.push_back(first);
}

void SkipList::printOneList(int listNum) {
	Quad* bottomCurrent = listHeads[0];
	Quad* current = listHeads[listNum];

	while(bottomCurrent->next != NULL) {
		std::string toPrint;
		if(current->entry->getKey() == bottomCurrent->entry->getKey()) {
			toPrint = current->entry->getKey();
			current = current->next;
		}
		else {
			toPrint = "--";
		}
		cout << "--" << toPrint;
		bottomCurrent = bottomCurrent->next;
	}
	cout << "--" << bottomCurrent->entry->getKey() << "--" << endl;
}

void SkipList::print() {
	int numLists = listHeads.size();
	for(int i = numLists - 1; i >= 0; i--) {
		printOneList(i);
	}
} 

SkipList::Entry* SkipList::find(Key k) {
    int numLists = listHeads.size();
    Quad* current = listHeads[numLists - 1];

    while (current->below != NULL) {
        current = current->below;			// drop down
        while(k >= current->next->entry->getKey()) {	// scan forward
            current = current->next;
        }
    } 

    if(current->entry->key == k) {
        return current->entry;
    }
    else {
        return NULL;
    }
}

// the "trail" is a vector of the last node visited on each list
// the last element of trail is on the lowest list; the first is on the highest.
std::vector<SkipList::Quad*>* SkipList::findWithTrail(Key k) {
    std::vector<SkipList::Quad*>* trail = new std::vector<Quad*>();

    int numLists = listHeads.size();
    Quad* current = listHeads[numLists - 1];
    while (current != NULL) {
        while(k >= current->next->entry->getKey()) {	// scan forward
            current = current->next;
        }
	trail->push_back(current);
        current = current->below;			// drop down
    } 
    return trail;
}




//-----------------------------------------------------------------//


// fill in the assigned functions here.

// This is a helper function for insert
// Counts how many lists the element will be on
int SkipList::coinFlipper() {
    // 0 is head, 1 is tail
    int i = 0; int counter = -1;
    while(i != 1) {
        i = rand() % 2;
        counter++;
    }
    return counter;
}

// Insert function
void SkipList::insert(Key k, Value v) {
   
    // This checks if the key already exists
    // if it exists, it would remove current key, 
    // then insert new key
    if (find(k) != NULL) {
        remove(k);
        insert(k, v); return;
    }

    int i = coinFlipper();

    // This sections adds new levels if needed
    if (i >= listHeads.size()-1) {
        int numNew = (i+1)-(listHeads.size()-1);

        while (numNew != 0) {
            makeNewLevelList();
            numNew--;
        }
    }
    
    vector<Quad*>* newTrail = findWithTrail(k);
    int listPosition = newTrail->size()-1;
    int lastListPos = (newTrail->size()-1)-i;


    // This adds the new element to the skiplists
    while (listPosition >= lastListPos) {

        Entry* newEntry = new Entry(k, v);
        Quad* newQuad = new Quad(newEntry);
        Quad* current = newTrail->at(listPosition);

        current->next->prev = newQuad;
        newQuad->next = current->next;
        current->next = newQuad;
        newQuad->prev = current;
        newQuad->below = NULL; newQuad->above = NULL;
  
        // This updates the above and below of the elements added
        if (listPosition != newTrail->size()-1) {
            newQuad->below = newTrail->at(listPosition+1)->next;
            newTrail->at(listPosition+1)->next->above = newQuad;
        }
        listPosition--;
        
    }

    delete newTrail; return;
}

// The remove function
void SkipList::remove(Key k) {

    // This checks if the key being removed are sentinels
    if (k == "!!" || k == "}}") return;

    int counter = 0;


    if (find(k) != NULL) {
        vector<Quad*>* newTrail = findWithTrail(k);
        Quad* current = newTrail->at(newTrail->size()-1);

        // This deletes the elements and travesl above
        while (current->above != NULL) {

            current->next->prev = current->prev;
            current->prev->next = current->next;
            current->next = NULL; current->prev = NULL;
            
            current = current->above;
            delete current->below->entry; delete current->below;
            counter++;
        }
        
        // This deletes the last element of the same key in the list
        current->next->prev = current->prev;
        current->prev->next = current->next;
        current->next = NULL; current->prev = NULL;
        counter++;
        delete current->entry; delete current; 

        // This deletes any excess lists
        if (counter == listHeads.size()-1) {
            current = listHeads[listHeads.size()-1];
            while (current->below->next->entry->getKey() == "}}" &&
                listHeads.size() > 2) {
                delete current->next->entry; delete current->next;
                delete current->entry; delete current;

                listHeads.pop_back();
                current = listHeads[listHeads.size()-1];
            }
        }
        delete newTrail; return;

    } else {
        return;
    }
    
}

// This is the ceilingEntry function
SkipList::Entry* SkipList::ceilingEntry(Key k) {

    vector<Quad*>* newTrail = findWithTrail(k);

    Quad* current  = newTrail->at(newTrail->size()-1);

    // This checks if the key is not in the element, 
    // else return the current element
    if (current->entry->getKey() != k) {
        current = current->next;
        delete newTrail;
        return current->entry;

    } else {
        delete newTrail;
        return current->entry;
    }

}

// This is the floorEntry function
SkipList::Entry* SkipList::floorEntry(Key k) {

    vector<Quad*>* newTrail = findWithTrail(k); 

    // return the exact key or greatest lesser key
    Quad* current  = newTrail->at(newTrail->size()-1);

    delete newTrail;
    return current->entry;

}

// This is the greaterEntry function
SkipList::Entry* SkipList::greaterEntry(Key k) {

    vector<Quad*>* newTrail = findWithTrail(k);

    Quad* current  = newTrail->at(newTrail->size()-1);

    // This returns the key that is least greater key than the inputted key 
    if (current->entry->getKey() > k) {
        delete newTrail;
        return current->entry;

    } else {
        current = current->next;
        delete newTrail;
        return current->entry;
    }
}

// This is the lesserEntry function
SkipList::Entry* SkipList::lesserEntry(Key k) {

    vector<Quad*>* newTrail = findWithTrail(k);

    Quad* current  = newTrail->at(newTrail->size()-1);

    // This returns the greatest lesser key than the inputted key
    if (current->entry->getKey() < k) {
        delete newTrail;
        return current->entry;

    } else {
        if (current->prev != NULL) {
            current = current->prev;
            delete newTrail;
            return current->entry;
        }

        delete newTrail;
        return current->entry;
    }   
}

// This is the destructor function
SkipList::~SkipList() {

    // This function travesl from left to right, deleting entry and quads
    for (int i = 0; i < listHeads.size(); i++) {
        Quad* current  = listHeads[i];

        while (current->next != NULL) {
            current = current->next;
            delete current->prev->entry;
            delete current->prev;
        }

        delete current->entry;
        delete current;
    }
    
    listHeads.clear();
}