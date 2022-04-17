# Client-and-server-thread-pool
The client application:
The client application will be called “GradeClient” and receives 2 parameters in the command line:
● GradeClient hostname port
○ hostname – name of the grade server (e.g.localhost)
○ port – port on which the grade server is listening.
The client application consists of 2 processes: command line interpreter and communication process. The command line interpreter receives commands from the user and the communication process handles communication with the server. The two processes communicate using an unnamed pipe.
When the application starts the command line interpreter prints the prompt “> ” and waits for commands from the user (the prompt is printed after each command), while the communication process connects to the server and promotes to it user commands.
Users can send one of the following commands:
●
Login id password : The user (student or TA) tries to login to the grade server with the given id and password. If login succeeds the application will print “Welcome Student id” or ”Welcome TA id” accordingly. Otherwise, it will print “Wrong user information”. Login will succeed even if a user with the same details is logged on from another client. It will fail only in the following cases:
o There is a user already logged in from this client. o The id doesn’t exist in the server.
o The password is incorrect.
The following commands (except Exit) are allowed only if a user is logged in. Otherwise, the message “Not logged in” will be printed. You may assume that command length including the parameters doesn’t exceed 256 chars.
● ●
ReadGrade : Print the grade of the logged in student. The operation can be done only by a student. If the operation was requested by a TA then the message “Missing argument” will be printed.
ReadGrade id: Print the grade of the specified id. The operation can be done only by a TA. If there is no student with the given id then the message “Invalid id” will be printed. If the operation was requested by a student then the message “Action not allowed” will be printed.
2
 Operating Systems – Winter 2021/22 (Wet Assignment 2)
· GradeList : Print a list of all the grades in the server. The operation can be done only by a TA. If the operation was requested by a student then the message “Action not allowed” will be printed. The list will be ordered in an ascending id order, and will be printed in the following format:
id: grade id: grade ...
UpdateGrade id grade : Update The grade of id in the server. If there was no such id in the server then it will be added. The operation can be done only by a TA. If the operation was requested by a student then the message “Action not allowed” will be printed.
Logout : The user will be disconnected from the grade server. The operation will fail if no user is logged in. If logout succeeds the application will print “Good bye id”. Otherwise, it will print “Not logged in”.
●
●
Note: a Logout command does not terminate the connection with the server. It only means that this user logged out.
● Exit : The client application will exit (both processes will finish). If a user was connected, the application will logout before it exits.
If the command is not one of the above the application will print “Wrong Input”. Example:
GradeClient localhost 12345
> Login 234567890 student1 Welcome Student 234567890 > ReadGrade
74
> UpdateGrade 234567890 98 Action not allowed
> Logout
3

 Good bye 234567890 > Hi
Operating Systems – Winter 2021/22 (Wet Assignment 2)
Wrong Input
> ReadGrade 234567890 Not logged in
> Login 011223344 TA1 Welcome TA 011223344
> fd
Wrong Input
> UpdateGrade 234567890 0 > ReadGrade 234567890
0
> GradeList
234567890: 0 123456789: 83 > Exit
The server application:
Introduction: Thread pool
If a server wants to work efficiently, the main thread can only receive new connections and for each connection it creates a thread to handle it. Although creating a thread to implement a task is cheaper than a process, but it is still an operation that takes resources and time. In order to solve this problem, the concept of thread pool was introduced. Instead of creating a thread for each task, the main thread creates a constant number of working threads during its initialization, and they live all the time the server is up. When a task is received it is
4

 Operating Systems – Winter 2021/22 (Wet Assignment 2)
inserted to a task list and a working thread is waked up to handle it. When no task is available the working threads will do nothing (wait on a condition variable).
The Grade Server
The server application will be called “GradeServer” and receive 1 parameter in the command line:
● GradeServer port
o port – server port number
The server is a multi-threaded application in which the main thread listens for new connections and a pool of working threads handles user connections.
When the server starts, the main thread will:
● initialize the data structures of the server.
o The ids and passwords of the TAs are read from a file called assistants.txt which resides in the same directory of the server application. Each line of the file has a single id:password couple (No spaces, id is nine digits). You may assume the file is in the correct format.
o The ids and passwords of the students are read from a file called students.txt which resides in the same directory of the server application. Each line of the file has a single id:password couple (No spaces). You may assume the file is in the correct format.
o The grades of all students (from the file students.txt) are initialized to 0.
● Create N=5 working threads which will be ready to handle clients.
● Open a socket with the port specified in the command line.
● The server will run an infinite loop in which it will accept users’ connections. For
each connection a new task is added to the task list and a working thread is
waked up to handle the task.
● The server ends only when it receives a kill command or Ctrl-C from the
command prompt. The signal handler should close the socket and exit.
All Working threads will run the same function. The function runs an infinite loop in which it takes a task from the task list and handles the user’s queries. When the list is empty the thread will wait on a condition variable until new tasks are inserted. All working threads work with the same global tables and should synchronize access to them.
5

3. Notes and Tips:
Operating Systems – Winter 2021/22 (Wet Assignment 2)
1. Youmustuseconditionvariablesfortheworkersthreadwaitingfornewtasks (semaphores are not allowed).
2. Youshouldwriteaportablecodeinthecommunicationbetweentheserverand the clients (use htonl/s and ntohl/s functions when needed).
3. YoushouldwriteinC(orC++ifyouknowit)only.
4. Youshouldclosesocketsafteryoufinishusingthem.
5. ThekernelhasaTIME_WAITbeforeithastotallyreleasedasocket.Lookatthis explanation: http://hea-www.harvard.edu/~fine/Tech/addrinuse.html .You can use any of the suggested "Strategies for Avoidance", or simply ignore this problem.
6. Don’taddanyprintoutsbeyondthatisrequested,otherwise,youmightfailinthe automatic check (“Wrong Input” means only the string “Wrong Input” and not “wrong input” nor “bad input”...).
