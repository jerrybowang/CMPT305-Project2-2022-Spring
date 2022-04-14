#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <queue>
#include "Instruction.h"
#include <map>

using namespace std;


// exception class
class FileReadException{
private:
	string errmsg;
public:
	FileReadException(const string& err){
		errmsg = err;
	}
	~FileReadException(){}
};


// global STAT vars

// {Integer, Floating, Branch. Load, Store}
unsigned long int Instruction_count[5] = {0};

// IF state only
unsigned long int Instruction_need_to_fetch;

int W = 0;

unsigned long int cycle_count = 0;



// global vars-----------
// address, line position of last completed address
map<unsigned long int,unsigned long int> depen;

// the latest instance of address and its line number
map<unsigned long int,unsigned long int> occor;

unsigned long int line_pos = 1;

// signals
bool end_read_file = false;
bool end_sim = false;

// looks
bool branch_lock = false;
bool int_EX_lock = false;
bool float_EX_lock = false;
bool load_MEM_lock = false;
bool store_MEM_lock = false;


// Q's
queue<Instruction> ID_Q;
queue<Instruction> EX_Q;
queue<Instruction> MEM_Q;
queue<Instruction> WB_Q;
// intermediate Q
queue<Instruction> ID_Q_imm;
queue<Instruction> EX_Q_imm;
queue<Instruction> MEM_Q_imm;
queue<Instruction> WB_Q_imm;

//--------------

// helper functions
unsigned long int Instruction_sum(){
	unsigned long int sum = 0;
	for (int i = 0; i < 5; i ++){
		sum += Instruction_count[i];
	}
	return sum;
}

void update_occor(unsigned long int line_number){
	// add it to occor, and uodate
	if (occor.find(line_number) != occor.end()){
		occor[line_number] = line_pos;
	}
	else{
		occor.insert(pair<unsigned long int,unsigned long int>(line_number, line_pos));
	}
}

void pre_fetch(ifstream& inFile, unsigned int start_inst){
	if (inFile.is_open()){
		string line;
		unsigned long int line_number;
		for (unsigned int i = 1; i < start_inst; i ++){
			// guard condiction
			if (! getline(inFile,line)){
				cout << "pre_fetch: end of file\n";
				throw FileReadException("pre_fetch: end of file");
				return;
			}
			if (line.length() < 1){
				cout << "pre_fetch: empty line\n";
				throw FileReadException("pre_fetch: empty line");
				return;
			}
			stringstream ss(line);
			string temp;
			// get line number
			getline(ss,temp,',');
			line_number = stoul(temp, nullptr, 16);
			// add it to depen, or update
			if (depen.find(line_number) != depen.end()){
				depen[line_number] = line_pos;
			}
			else{
				depen.insert(pair<unsigned long int,unsigned long int>(line_number, line_pos));
			}
			// add it to occor, and uodate
			update_occor(line_number);
			// incriment line_pos
			line_pos++;
		}

	}
}

void flush_imm_Q(){
	while(!ID_Q_imm.empty()){
		Instruction ins = ID_Q_imm.front();
		ID_Q.push(ins);
		ID_Q_imm.pop();
	}

	while(!EX_Q_imm.empty()){
		Instruction ins = EX_Q_imm.front();
		EX_Q.push(ins);
		EX_Q_imm.pop();
	}

	while(!MEM_Q_imm.empty()){
		Instruction ins = MEM_Q_imm.front();
		MEM_Q.push(ins);
		MEM_Q_imm.pop();
	}

	while(!WB_Q_imm.empty()){
		Instruction ins = WB_Q_imm.front();
		WB_Q.push(ins);
		WB_Q_imm.pop();
	}
}


// main process functions

void Process_IF(ifstream& inFile);
void Process_ID();
void Process_EX();
void Process_MEM();
void Process_WB();
void Process_print();

int main(int argc, char* argv[]) {
	// input arguments
	if(argc >= 5){
		string file_path = argv[1];
		int start_inst = atoi(argv[2]);
		int run_inst = atoi(argv[3]);
		int pipeline_width = atoi(argv[4]);

		// create file object
		ifstream inFile(file_path);

		// Add error checks for input variables here, exit if incorrect input

		if (!inFile.is_open()){
			cout << "File read error: cannot read file in path: " << file_path <<endl;
			return 0;
		}

		if (start_inst < 1){
			cout << "incorrect input: start_inst < 1\n";
			// close file
			inFile.close();
			return 0;
		}

		if (run_inst < 0){
			cout << "incorrect input: run_inst < 0\n";
			// close file
			inFile.close();
			return 0;
		}

		if (pipeline_width < 1){
			cout << "incorrect input: pipeline_width < 1\n";
			// close file
			inFile.close();
			return 0;
		}

		// initialize global params
		Instruction_need_to_fetch = run_inst;
		W = pipeline_width;

		try{
			// run pre-fetching to get all commpleted Instruction address
			pre_fetch(inFile, start_inst);


			// run only we need to fetch Instruction
			if (Instruction_need_to_fetch > 0){
				// run simulations
				while(end_sim == false){
					// cout << "\nCycle: " << cycle_count<<endl;
					// cout << "IF\n";
					Process_IF(inFile);
					// cout << "ID\n";
					Process_ID();
					// cout << "EX\n";
					Process_EX();
					// cout << "MEM\n";
					Process_MEM();
					// cout << "WB\n";
					Process_WB();
					cycle_count++;
					// flush all Instruction
					flush_imm_Q();

					// reset EX and MEM lock (branch lock no need to reset every cycle)
					int_EX_lock = false;
					float_EX_lock = false;
					load_MEM_lock = false;
					store_MEM_lock = false;

				}

			}

			// print restuls
			Process_print();
		}
		catch (FileReadException){
			cout<< "caught FileReadException\n";
			return 0;
		}

	}
	else{
		cout << "Insufficient number of arguments provided!\n";
	}
	return 0;

}


// ----------------------------
// implimentation
void Process_IF(ifstream& inFile){
	// if condiction meet, do nothing
	if(end_read_file){
		return;
	}
	if (!inFile.is_open()){
		cout << "Process_IF File read error: cannot read file\n";
		throw FileReadException("cannot read file");
		return;
	}

	// prepare temp vars
	string line;
	unsigned long int line_address;
	unsigned int type;
	vector<unsigned long int> dependen;
	Instruction_type ins_type;

	// start to fetch and create Instruction object
	for (int i = 0; i < W; i ++){
		// if locked, do nothing
		if (branch_lock){
			return;
		}
		// if signal raised, do nothing
		if (end_read_file){
			return;
		}

		// start grab info from file
		// guard condiction
		if (! getline(inFile,line)){
			cout << "Process_IF pre_fetch: end of file\n";
			end_read_file = true;
			end_sim = true;
			throw FileReadException("EOF");
			return;
		}
		if (line.length() < 1){
			cout << "Process_IF pre_fetch: empty line\n";
			end_read_file = true;
			end_sim = true;
			throw FileReadException("empty line");
			return;
		}

		// we get the line, decriment counter
		Instruction_need_to_fetch--;

		stringstream ss(line);
		string temp;
		// get line address
		getline(ss,temp,',');
		line_address = stoul(temp, nullptr, 16);
		// get type
		getline(ss,temp,',');
		type = stoi(temp);
		switch(type){
			case 1:
			ins_type = Integer;
			break;
			case 2:
			ins_type = Floating;
			break;
			case 3:
			ins_type = Branch;
			break;
			case 4:
			ins_type = Load;
			break;
			case 5:
			ins_type = Store;
			break;
			default:
			cout << "Process_IF error: cannot decode type\n";
			ins_type = BAD_TYPE;
		}
		// get list of dependencies
		while (getline(ss,temp,',')){
			dependen.push_back(stoul(temp, nullptr, 16));
		}

		// create Instruction object
		Instruction ins(line_address, ins_type, dependen, line_pos,occor);

		// clear temp vector for next use
		dependen.clear();

		// check if it is branch
		if (ins.get_type() == Branch){
			branch_lock = true;
		}
		// check if this is the last Instruction
		if (Instruction_need_to_fetch == 0){
			ins.set_end_signal(true);
			end_read_file = true;
		}

		// finished IF, push to ID imm Q
		ID_Q_imm.push(ins);

		// ins.print();

		// update occor map
		update_occor(line_address);

		// incriment line_pos
		line_pos++;

	}
}

void Process_ID(){

	for (int i = 0; i < W; i ++){
		//check whether the ID_Q is empty. If ture, go next step
		if (ID_Q.empty()){
			return;
		}

		//create a temp value to accept the first instruction in ID_Q
		Instruction temp = ID_Q.front();



		//Delete the instruction in ID_Q that has done ID process
		ID_Q.pop();

		// finished ID, push to EX imm Q
		EX_Q_imm.push(temp);
		// temp.print();
	}

}

void Process_EX(){
	for (int i = 0; i < W; i ++){
		//check whether the EX_Q is empty. If ture, go next step
		if (EX_Q.empty()){
			return;
		}
		//create a temp value to accept the first instruction in EX_Q
		Instruction temp = EX_Q.front();

		//if the instruction is dependent on some instruction
		//then return directly until that not dependent
		if(temp.dependency_check(depen)==false){
			return;
		}

		if(temp.get_type() == Integer) {
			// if int ALU locked, do nothing
			if (int_EX_lock){
				return;
			}
		}
		// if loat ALU locked, do nothing
		if(temp.get_type() == Floating){
			if (float_EX_lock){
				return;
			}
		}

		if(temp.get_type() == Branch){
			if(branch_lock){
				branch_lock = false;
				EX_Q.pop();
				MEM_Q_imm.push(temp);
				return;
			}
		}
		// check if it is Integer
		if (temp.get_type() == Integer){
			int_EX_lock = true;
		}
		// check if it is float
		if (temp.get_type() == Floating){
			float_EX_lock = true;
		}
		// pop finished exuction
		EX_Q.pop();

		// add/update dependency to map
		if (temp.get_type() == Integer || temp.get_type() == Floating){
			if (depen.find(temp.get_address()) == depen.end()){
				depen.insert(pair<unsigned long int,unsigned long int>(temp.get_address(),temp.get_line_position()));
			}
			else{
				depen[temp.get_address()] = temp.get_line_position();
			}
		}

		// push into temp_MEM queue
		MEM_Q_imm.push(temp);
		// temp.print();

	}
}

void Process_MEM(){
	for (int i = 0; i < W; i ++){
		//check whether the EX_Q is empty. If ture, go next step
		if (MEM_Q.empty()){
			return;
		}
		//create a temp value to accept the first instruction in MEM_Q
		Instruction temp = MEM_Q.front();
		if(temp.get_type() == Load ) {
			// if Load mem locked, do nothing
			if (load_MEM_lock){
				return;
			}
		}
		// if store mem locked, do nothing
		if(temp.get_type() == Store){
			if (store_MEM_lock){
				return;
			}
		}
		// check if it is Load
		if (temp.get_type() == Load){
			load_MEM_lock = true;
		}
		// check if it is store
		if (temp.get_type() == Store){
			store_MEM_lock = true;
		}
		// pop finished exuction
		MEM_Q.pop();

		// add/update dependency to map
		if (temp.get_type() == Load || temp.get_type() == Store){
			if (depen.find(temp.get_address()) == depen.end()){
				depen.insert(pair<unsigned long int,unsigned long int>(temp.get_address(),temp.get_line_position()));
			}
			else{
				depen[temp.get_address()] = temp.get_line_position();
			}
		}


		// push into temp_WB queue
		WB_Q_imm.push(temp);
		// temp.print();

	}
}

void Process_WB(){
	for (int i = 0; i < W; i ++){
		//check whether the WB_Q is empty. If ture, go next step
		if (WB_Q.empty()){
			return;
		}

		//create a temp value to accept the first instruction in WB_Q
		Instruction temp = WB_Q.front();

		//check whether the insturction is the last one
		//if ture, set the end_sim true to stop simulation
		if (temp.get_end_signal()==true){
			end_sim = true;
		}

		//add the completed instruction to map
		// update if necesscary
		//for checking dependencey

		if (depen.find(temp.get_address()) == depen.end()){
			depen.insert(pair<unsigned long int,unsigned long int>(temp.get_address(),temp.get_line_position()));
		}
		else{
			depen[temp.get_address()] = temp.get_line_position();
		}


		//count the instruction type
		switch(temp.get_type()){
			case Integer:
			Instruction_count[0]++;
			break;
			case Floating:
			Instruction_count[1]++;
			break;
			case Branch:
			Instruction_count[2]++;
			break;
			case Load:
			Instruction_count[3]++;
			break;
			case Store:
			Instruction_count[4]++;
			break;
			default:
			cout << "Process_WB error: cannot get instruction type\n";
		}

		//Delete the instruction in WB_Q that has done WB process
		WB_Q.pop();
		// temp.print();
	}
}

void Process_print(){
	//print the cycles of the simulation
	cout << "Simulation clock: "<< cycle_count << endl;

	cout << "The total execution time (in cycles) at the end of simulation: " << cycle_count << endl;

	//count the total completed instructions
	unsigned long int sum = Instruction_sum();

	//print the histogram containing the breakdown of retired instructions by instruction type
	float Integer_per = 0;
	float Floating_per = 0;
	float Branch_per = 0;
	float Load_per = 0;
	float Store_per = 0;
	if(sum>0){
		Integer_per = ((float)Instruction_count[0]/sum)*100;
		Floating_per = ((float)Instruction_count[1]/sum)*100;
		Branch_per = ((float)Instruction_count[2]/sum)*100;
		Load_per = ((float)Instruction_count[3]/sum)*100;
		Store_per = ((float)Instruction_count[4]/sum)*100;
	}
	cout << "Histogram: \n";
	printf ("%.4f%% are integer instructions \n%.4f%% are floating point \n%.4f%% are branches \n%.4f%% are loads \n%.4f%% are stores \n\n", Integer_per, Floating_per, Branch_per, Load_per, Store_per);

}
