
/*
  Rijish Ganguly
  rg239
  ECE 551
  Command-Shell Mini-Project

 */






#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>

using namespace std;

class myShell{



public:


		string command; //For a single command
		vector<string> parsed; //Get the parsed string arguments
	
		myShell(); //Constructor
		~myShell(); //Destructor
		
		//void delete_spaces(string & str);
		
		int get_command(); //Get command from the user

		//Parse the input and store the parameters in a vector
		void split_input ();

		//Execute a command
		void execute();

		//Search path to determine if the command exists
		bool search_command(); 

		//Search path to determine if the command exists

		bool validate_var(string & str); 
		

		/*

			Built-in command - cd, export, set and inc 

		*/
		
		int run_set_command(); //Run the built-in set command
		int run_cd_command(); //Run the built-in cd command
		int run_inc_command(); //Run the built-in inc command
		bool inc_number_helper(const string & str); //Helper function for the inc command
		int run_export_command(); //Run the built-in export command



	
		//The functions required to support to accessing commands
		
		void replace_var();	//Replace the variable
		void replace_str(string & str, const string & from, const string & to); //Helper function for the replace_var function


};
