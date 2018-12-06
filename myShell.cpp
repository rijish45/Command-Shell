#include "myShell.h"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <sstream>
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

}


//Convert the vector of command into a char * array for the execve call

// char ** myShell::return_array(vector<string> vec){

// size_t index;
// char ** c_array = new char*[vec.size() + 1];

// for (vector<string>::iterator it = vec.begin(); it < vec.end(); it++) {
//     c_array[index] = new char[it->length() + 1];
//     strncpy(c_array[index], it->c_str(), it->length() + 1);
//     index++;
//   }
//   c_array[index] = NULL;
//   return c_array;

// }


/*Execute a single command usin fork and execve
Consulted man pages for this part
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
      			
      			if(myShell.parsed.size() > 0)
      				myShell.execute();

      		}



   		 }
		

return EXIT_SUCCESS;

}