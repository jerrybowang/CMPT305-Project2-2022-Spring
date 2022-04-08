#ifndef INSTRUCTION_H
#define INSTRUCTION_H
#include <iostream>
#include <vector>
#include <unordered_set>


using namespace std;

enum Instruction_type {Integer, Floating, Branch, Load, Store, BAD_TYPE};
typedef enum Instruction_type Instruction_type;

class Instruction{
private:
unsigned long int address;
Instruction_type type;
bool end_signal = false;
vector<unsigned long int> dependency;

public:
// constructor and destructor
Instruction(unsigned long int Address, Instruction_type Type, vector<unsigned long int>& Dependency){
	address = Address;
	type = Type;
	if (! Dependency.empty()){
		dependency.insert(dependency.begin(),Dependency.begin(),Dependency.end());
	}
}

~Instruction(){}

// getters
Instruction_type get_type() const{
	return type;
}

unsigned long int get_address() const{
	return address;
}

bool get_end_signal()const{
	return end_signal;
}

// setter
void set_end_signal(bool val){
	end_signal = val;
}

// other methoids
bool dependency_check(unordered_set<unsigned long int>& depen){
	if (dependency.empty()){
		return true;
	}

	// check if all dependency is excuted or not
	for (auto ele : dependency){
		if (depen.find(ele) == depen.end()){
			return false;
		}
	}
	return true;
}

// debug methoid
void print(){
	string i_type;
	switch(type){
			case Integer:
			i_type = "Integer";
			break;
			case Floating:
			i_type = "Floating";
			break;
			case Branch:
			i_type = "Branch";
			break;
			case Load:
			i_type = "Load";
			break;
			case Store:
			i_type = "Store";
			break;
			default:
			i_type = "BAD_TYPE";;
		}
	cout << "Instruction address: " << address;
	if (end_signal){
		cout << " (end)";
	}
	cout<<endl;
	
	cout << "Instruction type: " << i_type << endl;
	cout << "Dependency:{ ";
	for (auto ele : dependency){
		cout<< ele << ", ";
	}
	cout <<"}\n\n";
}


};

#endif


