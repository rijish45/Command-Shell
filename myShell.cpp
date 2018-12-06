#include "myShell.h"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <limits>
#include <unistd.h>
#include <iterator>

extern char ** environ; //needed for execve call 



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


bool found = false;
string command = parsed[0]; //The command user inputs
vector<string> vec_path;
char * path = getenv("PATH"); //Get the path variable 

//Now that we have the path we need to parse it using the colon delimiter and store it in the vec_path vector
istringstream ss(path);
string token;
while(getline(ss, token, ':')) 
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

  
		

return EXIT_SUCCESS;

}