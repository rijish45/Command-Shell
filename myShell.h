#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>

using namespace std;

class myShell{



public:


		string command; //For a single command
		vector<string> parsed; //get the parsed string arguments
	

		myShell(); //constructor
		~myShell(); //destructor
		
		//void delete_spaces(string & str);
		int get_command();

		//parse the input and store the parameters in a vector
		void split_input ();

		//execute a command
		void execute();

		//Return a char array from a vector
		//char ** return_array(vector<string> vec);

		
		bool search_command();

		bool validate_var(string & str);
		
		int run_set_command();

		int run_cd_command();

		int run_inc_command();
		bool inc_number_helper(const string & str);

		int run_export_command();

};