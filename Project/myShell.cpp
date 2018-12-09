/*
Rijish Ganguly
rg239
ECE 551
Command-Shell Mini-Project
 */

#include "myShell.h"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <limits>
#include <map>
#include <unistd.h>
#include <iterator>

extern char ** environ; //needed for execve call 
map < string, string > var_map; //map to store variables

/*

 This function takes the command from the user of the shell and exits if
 it encounters the command exit or EOF. Otherwise, it removes the whitespaces at 
 the end and beginning of the command 

*/

int myShell::get_command(){

char current[1024];
getcwd(current, sizeof(current));
cout << "myShell:" << current << " $ "; // The shell prompt
getline(cin, command);

if(cin.eof()){
    
    cout << endl; 
	cin.clear();
	exit(EXIT_SUCCESS);
}

else if(cin.fail()){
	cin.clear();
}


else if (command.compare("exit") == 0){
	cout << "Program exited with status " << EXIT_SUCCESS << "\n";
	setbuf(stdin, NULL);
	exit(EXIT_SUCCESS);

}

else if(command.size() == 0)
	return -1;

else{
	
	command.erase(0, command.find_first_not_of(" \t\n\r"));
	command.erase(command.find_last_not_of(" \t\n\r") + 1);
}

return 0;

}


myShell::myShell(){}; // The constructor for our myShell
myShell::~myShell(){}; // The destructor for our myShell


/*

Parse the input skipping whitespaces using
istringstream

*/

void myShell::split_input () {
	
	istringstream ss(command);
	for(string str; ss >> str; )
    	parsed.push_back(str);
}

/*

Execute a single command usin fork and execve
Consulted man pages for this part - waitpid(2)

*/

void myShell::execute(){

	pid_t pid, w;
	int status;

	//Turn the vector into a char ** by iteration and using strncpy

	size_t index = 0;
	char ** c_array = new char*[parsed.size() + 1];

    for (vector<string>::iterator it = parsed.begin(); it < parsed.end(); it++) {
    		c_array[index] = new char[it->length() + 1];
    		strncpy(c_array[index], it->c_str(), it->length() + 1);
    		index++;
  }
  c_array[index] = NULL;
  
	pid = fork();
	if (pid == -1) { //Child process creation fails
        
        cerr << "Failed to create a child process: " << strerror(errno) << endl;
        exit(EXIT_FAILURE);
    }

    //Code executed by child 

    if (pid == 0) {            
        
        execve(c_array[0], c_array, environ);
        cerr << "execve failure: " << strerror(errno) << endl;
        exit(EXIT_FAILURE);
        
      			
    }
 		
    else {          // Code executed by parent 
        
        usleep(5000);
        
        do {
            
            w = waitpid(pid, &status, WUNTRACED | WCONTINUED);
            
            if (w == -1) {
                perror("waitpid");
                exit(EXIT_FAILURE);
            }

           //When the program exits
           if (WIFEXITED(status)) {
               cout << "Program exited with status " << WEXITSTATUS(status) << "\n";
            } 
            //When the program gets killed
            else if (WIFSIGNALED(status)) {
                cout << "Program was killed by signal " << WTERMSIG(status) << "\n";
            }
            //When the program gets stopped 
            else if (WIFSTOPPED(status)) {
                cout << "Program was stopped by signal" << WSTOPSIG(status) << "\n";
            } 
            else if (WIFCONTINUED(status)) {
                cout << "continued" << "\n";
            }
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    //Free the memory

    for (int i = 0; i < parsed.size(); i++) {
    	delete [] c_array[i];
  	}
  	
  	delete [] c_array;
}


/*
	
	Implementation of the search command which searches environment
	PATH variable to see if a command exists and returns a boolean value
	after completion of search

*/

bool myShell::search_command(){
char * path = getenv("PATH");  //Get the path variable 
bool found = false;
string command = parsed[0]; //The command user inputs
vector<string> vec_path;


//Now that we have the path we need to parse it using the colon delimiter and store it in the vec_path vector
istringstream ss(path);
string token;
while(getline(ss, token, ':'))  //Use : as the delimiter
	vec_path.push_back(token);

if(command.find('/') != string::npos){

   ifstream fs(command.c_str()); 
   if(fs.good()) //ifs.good() returns false if the file doesn't exist
   		found = true;
 
}
// The file doesn't contain a / in it
else{

	for (vector<string>::iterator it = vec_path.begin(); it != vec_path.end(); it++) {
      
      string actual_path = *it + "/" + command;
      ifstream ifs(actual_path.c_str()); 
      if (ifs.good()) { //make sure the file exists
  	    found = true;    
  	    parsed[0] = actual_path; // replace the input command with the actual path
  	    break;
      }
    }
  }

return found;

}

/*

Implementation of the built-in cd command which helps to 
change directories

*/

int myShell::run_cd_command(){

if(parsed.size() > 2){

	cerr << "Too many arguments for cd" << endl;
	return -1;
}

//If there is no other argument after cd or the argument is ~, we change to the home directory

else if(parsed.size() == 1 || parsed[1] == "~"){

	string destination = getenv("HOME");  
	if(chdir(destination.c_str()) >= 0 ){
		return 1;
	}
	else {
        perror("-bash: cd");
        return -1;
    }

}
else{ //Change to the desired directory

		if(chdir(parsed[1].c_str()) >= 0){
			return 1;
		}
		else{
			perror("-bash: cd");
        	return -1;
        }
	}

}


/*

Make sure the variable name under consideration is valid
i.e. it contatins number, letters and _ only
Used in several other functions

*/

bool myShell::validate_var(string & str){
for (int i = 0; i < str.size(); i++) {
    if ( str[i] != '_' && !isalnum(str[i])) 
    	return false;
  }

  return true;

}


/*

Implementation of the built-in  set command which helps us
set up variables with values in a map

*/
int myShell::run_set_command(){
	
	if(parsed.size() < 2){
		cerr << "The set command requires more arguments" << endl;
		return -1;
	}
	
	else if(parsed.size() > 3){

		cerr << "Too many arguments for the set command" << endl;
		return -1;

	} 
	 //If the third argument is not provided, the value of the variable is set to whitespace

	else if(parsed.size() == 2){
		if(validate_var(parsed[1])){
			map<string, string>::iterator it = var_map.find(parsed[1]);
			if(it == var_map.end()){
				var_map.insert(make_pair(parsed[1], " "));
				cout << "The variable " << parsed[1] << " has been set to whitespace" << endl;
				//cout << var_map[parsed[1]] << endl;
			}
			else{
				var_map.erase(it);
				var_map.insert(make_pair(parsed[1], " "));
				cout << "The variable " << parsed[1] << " has been re-set to whitespace" << endl;
				//cout << var_map[parsed[1]] << endl;
			
			}
		
           return 1;
		}

		else if(!validate_var(parsed[1])){ //If the variable name is not valid, display error message
			cerr << "The variable name is illegal" << endl;
			return -1;
		}
	}
    
else if(parsed.size() == 3){
		if(validate_var(parsed[1])){
			map<string, string>::iterator it = var_map.find(parsed[1]);
			if(it == var_map.end()){
				var_map.insert(make_pair(parsed[1], parsed[2]));
				cout << "The variable " << parsed[1] << " has been set to " <<  parsed[2] << endl;
				//cout << var_map[parsed[1]] << endl;
			}
			else{ //If the variable already exits, re-set its value to the new value
				var_map.erase(it);
				var_map.insert(make_pair(parsed[1], parsed[2]));
				cout << "The variable " << parsed[1] << " has been re-set to " << parsed[2] << endl;
				//cout << var_map[parsed[1]] << endl;
			}
			return 1;
		}

		else if(!validate_var(parsed[1])){
			cout << "The variable name is illegal" << endl;
			return -1;
		}
	}

	return 1;
}


/*
	This function acts as a helper to replace_var. It takes three input parameters and replaces a
	substring with another one
*/

void myShell::replace_str(string & str, const string & from, const string & to) {
    size_t start_pos = str.find(from);
    str.replace(start_pos, from.length(), to);
}


/*
	
	This function provides the command shell access to variables. If the user inputs
	$varname, it's value is displayed 

*/
void myShell::replace_var(){

	for(int i = 0; i < parsed.size(); i++){
		
		if(parsed[i].find("$") == string::npos){
			continue;
		}
		
		else {	//Case where there is atleast one $
			
			string evaluate = parsed[i]; //String is abc$hello-$world***
			
			size_t found_index = evaluate.find("$");
			string substr1 = evaluate.substr(found_index + 1); //substr1 is now hello-$world***
			string substr2;
			size_t found_index1 = substr1.find("$");
			
			if(found_index1 != string::npos){
				substr2 = substr1.substr(found_index1 + 1); //substr 2 is now world***
				substr1 = substr1.substr(0,found_index1);   //substr 1 is now  hello-
				//cout << substr1 << endl;
				//cout << substr2 << endl;

			}
			else{
				substr2 = "";
			    //subsr1 = hello-
			}

			if(substr2.compare("") == 0){ //just need to evaluate substr1

					int pos_illegal;
					for(int i = 0; i < substr1.size(); i++){
						if(!isalnum(substr1[i])){
							pos_illegal = i;
							break;
					}
					
					else
						pos_illegal = -1;
				}

					//cout << pos_illegal << endl;

				    string var;
					if(pos_illegal == -1)
						var = substr1;
					else
						var = substr1.substr(0, pos_illegal);
						
					string to_be_replaced = "$" + var; //The string that needs to be replaced
					//cout << to_be_replaced << endl;
			        string replace_value;
					map<string, string>::iterator it = var_map.find(var); 
					if(it != var_map.end()){

							replace_value = var_map[var]; //The value tha string needs to be replaced with
							replace_str(parsed[i], to_be_replaced, replace_value);
					}

					else{
						if ((getenv(var.c_str())) != NULL) {
            			char * var_value = getenv(var.c_str());
           				replace_value = var_value;
           				replace_str(parsed[i], to_be_replaced, replace_value);

           				}
           			}

		} //evaluated substr1

			else { //need to evaluate both substr1 and substr2 for this case 

				int pos_illegal;
					for(int i = 0; i < substr1.size(); i++){
						if(!isalnum(substr1[i])){
							pos_illegal = i;
							break;
					}
					
					else
						pos_illegal = -1;
				}

					//cout << pos_illegal << endl;

				    string var;
					if(pos_illegal == -1)
						var = substr1;
					else
						var = substr1.substr(0, pos_illegal);
						
					string to_be_replaced = "$" + var;
					//cout << to_be_replaced << endl;
			        string replace_value;
					map<string, string>::iterator it = var_map.find(var);
					if(it != var_map.end()){

							replace_value = var_map[var];
							replace_str(parsed[i], to_be_replaced, replace_value);
					}

					else{
						
						if ((getenv(var.c_str())) != NULL) {
            			char * var_value = getenv(var.c_str());
           				replace_value = var_value;
           				replace_str(parsed[i], to_be_replaced, replace_value);
           			}
            
				}



				int pos_illegal2;
					
					for(int i = 0; i < substr2.size(); i++){
						if(!isalnum(substr2[i])){
							pos_illegal2 = i;
							break;
					}
					
					else
						pos_illegal2 = -1;
				}

					//cout << pos_illegal2 << endl;

				    string var2;
					if(pos_illegal2 == -1)
						var2 = substr2;
					else
						var2 = substr2.substr(0, pos_illegal2);
						
					string to_be_replaced2 = "$" + var2;
					//cout << to_be_replaced << endl;
			        string replace_value2;
					map<string, string>::iterator it2 = var_map.find(var2);
					if(it2 != var_map.end()){

							replace_value2 = var_map[var2];
							replace_str(parsed[i], to_be_replaced2, replace_value2);
					}


					else{
						
						if ((getenv(var2.c_str())) != NULL) {
            			char * var_value2 = getenv(var2.c_str());
           				replace_value2 = var_value2;
           				replace_str(parsed[i], to_be_replaced2, replace_value2);
           			}
            
			}

	  } //evaluated both substr1 and substr2

			}// case where $ exists
					} //For loop ends, we have gone through all parsed commands and evaluated the variables
} //function ends


/*

 This function acts as a  helper function to the inbuilt inc function. Its checks if
 the stored value is numeric or not

*/
bool myShell::inc_number_helper(const string & str)
{
    string::const_iterator it = str.begin();
    while (it != str.end() && isdigit(*it)) ++it;
    return !str.empty() && it == str.end();
}


/*

 This function implements the inc command. It increments the value
 of a variable by 1. If the variable is not defined or it doesn't have a 
 numeric value, it assumes its' value to be 0 initially and increments to
 1.
	
*/
int myShell::run_inc_command(){

if(parsed.size() == 1){
  cerr << "The inc command requires more arguments" << endl;
  return -1;
}

else if (parsed.size() == 2){

   if(validate_var(parsed[1]))
   {
		map <string, string>::iterator it = var_map.find(parsed[1]);
		if(it == var_map.end()){
			var_map.insert(make_pair(parsed[1], "1"));
			cout << "The variable " << parsed[1] << " has been incremented to " << var_map[parsed[1]] << endl;
		}

		else{

			if(inc_number_helper(var_map[parsed[1]]) == false){
				var_map.erase(it);
				var_map.insert(make_pair(parsed[1], "1"));
				cout << "The variable " << parsed[1] << " has been incremented to " << var_map[parsed[1]] << endl;
			}

			else{

				int num = stoi(var_map[parsed[1]]); //Use the stoi function to convert string to long integer
				num++;
				var_map.erase(it);
				var_map.insert(make_pair(parsed[1], to_string(num)));
				cout << "The variable " << parsed[1] << " has been incremented to " << var_map[parsed[1]] << endl;
				}

		}
}

else{
		cerr << "The variable name is illegal" << endl;
		return -1;
	}

}

else if(parsed.size() > 2){

	cerr << "The inc command can handle on variable at a time" << endl;
	return -1;
}

return 1;
}

/*

	This function implements the export command. When the variable is
	exported it gets displayed when we run the env command

*/


int myShell::run_export_command(){

if(parsed.size() == 1){

	cerr << "The export command requires more arguments" << endl;
	return -1;
}

if(parsed.size() == 2){

	if(validate_var(parsed[1])){

		map<string, string>::iterator it = var_map.find(parsed[1]);
		if(it == var_map.end()){ // A variable cannot be exported if it's not been set
			cout << "The variable has not been defined yet. Set it first " << endl;
			return -1;
		}
		else
		{		
			if ( setenv(it->first.c_str(), it->second.c_str(), 1) != 0) {
				perror("The export command encountered a problem");
				return -1;
			}
			else
				cout << "The variable " << parsed[1] << " has been exported" << endl;
		}
	

	}

	else { 
		cerr << "The variable name " << parsed[1] << " is not valid" << endl;
		return -1;
	}
	

}

if(parsed.size() > 2){

	cerr << "The export command can handle one variable at a time " << endl;
	return -1;
}
return 1;

}

//The main function from where the execution starts

int main(int argc, char ** argv){

	myShell myShell; //Create a shell object

	while(true){ //Loop for the shell 
			
			myShell.command.clear();
			myShell.parsed.clear();

			if (myShell.get_command() != 0) //If the command is empty, continue to next iteration
      			 continue;

      		else{

      			 //The built-in commands for the shell
      			
      			myShell.split_input();  //Split the input based on whitespace
      			
      			
      			
      			int status;
      			if(myShell.parsed[0] == "cd"){ //The cd command can take $variable 
      				
      				myShell.replace_var();
      				status = myShell.run_cd_command();
      			}
      			else if(myShell.parsed[0] == "set"){ //The set command can take $variable as the third argument
      				
      				myShell.replace_var();
      				status = myShell.run_set_command();
      			}
      			else if(myShell.parsed[0] == "inc") //The inc command
      				status = myShell.run_inc_command();
      			else if(myShell.parsed[0] == "export") //The export command
      				status = myShell.run_export_command();
      			else if(myShell.parsed[0].find("$") != string::npos && (myShell.parsed.size() == 1)){
      				myShell.replace_var();
      				cout << myShell.parsed[0] << endl;
      			}

      		
      			else{
      			        //Execute a command after searching the command 
      				myShell.replace_var();
      				bool command_found = myShell.search_command();
      				if(command_found){
      					if(myShell.parsed.size() > 0)
      						myShell.execute();
      				}

      			else if (command_found == false){ //If the command is not found
      				cout << "Command " << myShell.parsed[0] << " not found" << "\n";
      			}

      		}

      	}
      }

		return EXIT_SUCCESS;
}
   	

  
		
