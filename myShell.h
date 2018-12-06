#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>

using namespace std;

class myShell{



public:


		string command; //For a single command
		vector<string> parsed; //get the parsed string arguments
	

		myShell();
		~myShell();
		
		//void delete_spaces(string & str);
		int get_command();

		//parse the input and store the parameters in a vector
		void split_input ();

		//execute a command
		void execute();

		//Return a char array from a vector
		char ** return_array(vector<string> vec);









};