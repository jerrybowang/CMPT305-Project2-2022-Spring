#ifndef INSTRUCTION_H
#define INSTRUCTION_H
#include <iostream>
#include <vector>
#include <map>


using namespace std;

enum Instruction_type {Integer, Floating, Branch, Load, Store, BAD_TYPE};
typedef enum Instruction_type Instruction_type;

class Instruction{
private:
	unsigned long int address;
	Instruction_type type;
	bool end_signal = false;
	vector<unsigned long int> dependency;
	vector<unsigned long int> dependency_ref;
	unsigned long int line_position;

public:
	// constructor and destructor
	Instruction(unsigned long int Address, Instruction_type Type, vector<unsigned long int>& Dependency, unsigned long int Line_position, map<unsigned long int,unsigned long int>& occor){
		address = Address;
		type = Type;
		line_position = Line_position;
		if (! Dependency.empty()){
			dependency.insert(dependency.begin(),Dependency.begin(),Dependency.end());
		}

		// now get the dependency line reference
		for(int i = 0; i < dependency.size(); i ++){
			dependency_ref.push_back(occor[dependency[i]]);
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

	unsigned long int get_line_position() const{
		return line_position;
	}

	// setter
	void set_end_signal(bool val){
		end_signal = val;
	}

	// other methoids
	bool dependency_check(map<unsigned long int,unsigned long int>& depen){
		if (dependency.empty()){
			return true;
		}

		// check if all dependency is excuted or not
		for (int i = 0; i < dependency.size(); i ++){
			// cout  << line_position << " ins " << address << " check depn " << dependency[i]<<endl;
			// if not find it, false
			if (depen.find(dependency[i]) == depen.end()){
				// cout << "not found\n";
				return false;
			}
			// if it is not the instance we want, false
			else if (depen[dependency[i]] < dependency_ref[i]){
				// cout << depen[dependency[i]] << " < " << dependency_ref[i]<<endl;
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
