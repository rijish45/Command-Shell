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
map< string, string > var_map;



int myShell::get_command(){

char current[1024];
getcwd(current, sizeof(current));

cout << "myShell:" << current << " $ ";
getline(cin, command);


if (cin.fail()) {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

else if(command.size() == 0){

	return -1;
	cin.clear();
}

else if (cin.eof() || (command.compare("exit") == 0))
{
	cout << "Program exited with status " << EXIT_SUCCESS << "\n";
	exit(EXIT_SUCCESS);

}

else{
	command.erase(0, command.find_first_not_of(" \t\n\r"));
	command.erase(command.find_last_not_of(" \t\n\r") + 1);
}

return 0;


}


myShell::myShell(){};
myShell::~myShell(){};



//Parse the input skipping whitespaces
void myShell::split_input () {
	
	istringstream ss(command);
	for(string str; ss >> str; )
    	parsed.push_back(str);

    // for (int i = 0; i < parsed.size(); i++)
    // {
    // 	cout << parsed[i] << endl;
    // }

}




/*Execute a single command usin fork and execve
Consulted man pages for this part - waitpid(2)
*/

void myShell::execute(){

	pid_t pid, w;
	int status;

	size_t index = 0;
	char ** c_array = new char*[parsed.size() + 1];

    for (vector<string>::iterator it = parsed.begin(); it < parsed.end(); it++) {
    		c_array[index] = new char[it->length() + 1];
    		strncpy(c_array[index], it->c_str(), it->length() + 1);
    		index++;
  }
  c_array[index] = NULL;
  
	
	 

	pid = fork();
	if (pid == -1) {
        
        cerr << "Failed to create a child process: " << strerror(errno) << endl;
        exit(EXIT_FAILURE);
    }

    /* Code executed by child */

    if (pid == 0) {            
        
        execve(c_array[0], c_array, environ);
        cerr << "execve failure: " << strerror(errno) << endl;
        exit(EXIT_FAILURE);
        
      			
    }
 		
    else {                    /* Code executed by parent */
        
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

    for (int i = 0; i < parsed.size(); i++) {
    	delete [] c_array[i];
  	}
  	
  	delete [] c_array;


  	

 }



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

   ifstream ifs(command.c_str()); 
   if(ifs.good()) //ifs.good() returns false if the file doesn't exist
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


int myShell::run_cd_command(){

if(parsed.size() > 2){

	cerr << "Too many arguments for cd" << endl;
	return -1;
}

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
else{

		if(chdir(parsed[1].c_str()) >= 0){
			return 1;
		}
		else{
			perror("-bash: cd");
        	return -1;
        }
	}

}

bool myShell::validate_var(string & str){

for (int i = 0; i < str.size(); i++) {
    if (!isalnum(str[i]) && str[i] != '_') 
    	return false;
  }

  return true;

}




int myShell::run_set_command(){
	
	if(parsed.size() < 2){
		cerr << "The set command requires more arguments" << endl;
		return -1;
	}


	else if(parsed.size() > 3){

		cerr << "Too many arguments for the set command" << endl;
		return -1;

	}

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

		else if(!validate_var(parsed[1])){
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
			else{
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



bool myShell::replace_str(string & str, const string & from, const string & to) {
    size_t start_pos = str.find(from);
    if(start_pos == string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}



//Let the string be abc$hello-$world***


void myShell::replace_var(){

	for(int i = 0; i < parsed.size(); i++){
		if(parsed[0].find("$") == string::npos){
			continue;
		}
		
		else {	//Case where there is atleast one $
			
			string evaluate = parsed[i]; //String is abc$hello-$world***
			size_t found_index = evaluate.find("$");
			string substr1 = evaluate.substr(found_index + 1); //substr1 is now hello-$world***
			string substr2;
			size_t found_index1 = substr1.find("$");
			if(found_index != string::npos){
				
			}
			
			//case where there is no other variable we need to evaluate, confirmed one dollar sign
			if(found_index1 == string::npos){ //substr is now hello-
				int pos_illegal;
				for(int i = 0; i < substr1.size(); i++){
					if(!isalnum(substr1[i])){
						pos_illegal = i;
						break;
					}
					else
						pos_illegal = -1;
				}
					cout << pos_illegal << endl;

				    string var;
					if(pos_illegal == -1)
						var = substr1;
					else
						var = substr1.substr(0, pos_illegal);
						
			
					string to_be_replaced = "$" + var;
					cout << to_be_replaced << endl;
			        string replace_value;
					map<string, string>::iterator it = var_map.find(var);
					if(it != var_map.end()){

							replace_value = var_map[var];
							bool flag_val = replace_str(parsed[i], to_be_replaced, replace_value);
					}

				} //one dollar sign evaluated

		else{ //evaluate second $ sign hello-$world***

				substr2 = evaluate.substr(found_index1 + 1);

				

		   }//evaluate second $ sign


}//case where there is atleast one $ ends
	


	} //for loop through all the strings end here



}








bool myShell::inc_number_helper(const string & str)
{
    string::const_iterator it = str.begin();
    while (it != str.end() && isdigit(*it)) ++it;
    return !str.empty() && it == str.end();
}


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

				int num = stoi(var_map[parsed[1]]);
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

	cerr << "The inc command can handle on varibale at a time" << endl;
	return -1;
}

	
return 1;


}


int myShell::run_export_command(){


if(parsed.size() == 1){

	cerr << "The export command requires more arguments" << endl;
	return -1;
}

if(parsed.size() == 2){

	if(validate_var(parsed[1])){

		map<string, string>::iterator it = var_map.find(parsed[1]);
		if(it == var_map.end()){
			cout << "The variable has not been defined yet. Set it first." << endl;
			return -1;
		}
		else
		{
			if ( setenv(it->first.c_str(), it->second.c_str(), 1) != 0) {
				perror("The export command encountered a problem");
				return -1;
			}
		}
	

	}
	else{
		cerr << "The variable name " << parsed[1] << " is not valid" << endl;
		return -1;
	}


}


return 1;



}

int main(int argc, char ** argv){

	myShell myShell;

	while(true)
		{
			myShell.command.clear();
			myShell.parsed.clear();

			if (myShell.get_command() != 0) 
      			 continue;

      		else{
      			
      			myShell.split_input();
      			int status;
      			
      			if(myShell.parsed[0] == "cd")
      				status = myShell.run_cd_command();
      			else if(myShell.parsed[0] == "set")
      				status = myShell.run_set_command();
      			else if(myShell.parsed[0] == "inc")
      				status = myShell.run_inc_command();
      			else if(myShell.parsed[0] == "export")
      				status = myShell.run_export_command();
      			else if(myShell.parsed[0].find("$") != string::npos){
      				myShell.replace_var();
      				cout << myShell.parsed[0] << endl;
      			}

      		
      			else{
      			
      				bool command_found = myShell.search_command();
      				if(command_found){
      					if(myShell.parsed.size() > 0)
      						myShell.execute();
      				}

      			else if (command_found == false){
      				cout << "Command " << myShell.parsed[0] << " not found" << "\n";
      			}

      		}

      	}
      }

		return EXIT_SUCCESS;

}
   	

  
		