#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <strings.h>
#include <limits.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


#define N 5 //NUMBER OF THREAD IN THE POOL
pthread_t thread_pool[N];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_var = PTHREAD_COND_INITIALIZER;

void * thread_function(void *arg);
void * connection_handler(void* p_client_socket);



struct node { //creating node for linked list
    struct node* next;
    int *client_socket;
};
typedef struct node node_t;

node_t* head = NULL;
node_t* tail = NULL;


FILE * studfile;
FILE * assistantsfile;
struct User * Students;
struct User * TA;

struct loggedIn {
    int type;
    int isLoggedIn;
    int id;
};



void error (char *msg){
    perror(msg);
    exit(1);
}

void enqueue(int *client_socket) {
    node_t *newnode = malloc(sizeof(node_t));
    newnode->client_socket = client_socket;
    newnode->next = NULL;
    if (tail == NULL) {
        head = newnode;
    } else {
        tail->next = newnode;
    }
    tail = newnode;
}

int* dequeue() {
    if (head == NULL) {
        return NULL;
    } else {
        int *result = head->client_socket;
        node_t *temp = head;
        head = head->next;
        if (head == NULL) {tail = NULL;}
        free(temp);
        return result;
    }
}

//statements
void enqueue(int *client_socket);
int* dequeue();
//opening a file
FILE * openfile (char * filename){
    FILE * file = fopen(filename, "r");
    if (file == NULL){
        printf("No such file in directory %s", filename);
        exit(1);
    }
    return file;
}

//User that use the system.
struct User {
    int id;
    int userType;
    int grade;
    char password[256];
    struct User * next;
};
//list of users- return the head of the list
struct User * create_list(FILE * file, int type){
    struct User *head = malloc(sizeof(struct User));
    struct User *tail; 
    int input_id;
    char file_id[10];//the id from the file
    char file_password[256];
    //getting the id and input_password from the file based on the file's template
    fgets(file_id, 10, file);
    fgetc(file);
    fgets(file_password, 256, file);
    //
    char * end_line;
    //searching for the \n and ends the string with null char
    if( (end_line = strchr(file_password, '\n')) != NULL)
        *end_line = '\0';

    //initializing the nodes
    input_id = atoi(file_id);
    head->id = input_id;
    strcpy(head->password,file_password);
    tail = head;
    if (type == 0) {
        head->grade = 0;
    }
    head->userType = type;
    //
// creating the rest of the list
    while (1) {
        if (fgets(file_id, 10, file) == NULL) {
            return head; }
        struct User *temp = malloc(sizeof(*temp));
        fgetc(file);
        fgets(file_password, 256, file);
        input_id = atoi(file_id);
        temp->id = input_id;
        char * end_line;
        if( (end_line = strchr(file_password, '\n')) != NULL)
            *end_line = '\0';
        strcpy(temp->password,file_password);
        if (type == 0) {
            temp->grade = 0;}
        temp->userType = type;
        if (temp->id <= head->id) {
            temp->next = head;
            head = temp;
            tail = temp;}

        else{
            while (tail->next != NULL && tail->next->id < temp->id) {
                tail = tail->next;
            }
            temp->next = tail->next;
            tail->next = temp;
            tail = head;
        }}}


// list of the logged in users- we use it to free the remaining logged users
struct LoggedInUsers {
    int id;
    int type;
    struct LoggedInUsers * next;
};
struct LoggedInUsers * HEAD = NULL;

// Prints the grades of all students
int GradeList(struct User * user){
    while(user != NULL){
        printf("%d: %d\n", user->id, user->grade);
        user = user ->next;}
    return 0;
}

//Insert new user and keep the list sorted- for the Gradelist
void sortedInsert (struct User ** head, struct User * nNode){
    if (*head== NULL || (*head) -> id >= nNode ->id){
        nNode-> next = *head;
        *head= nNode;
        return;
    }
    struct User * current = *head;
    while (current->next!=NULL&&current->next->id<nNode->id){
        current = current->next;
    }
    nNode->next = current->next;
    current->next = nNode;
    return;
}

struct User * UpdateGrade (int id,struct User * user, int grade){
    //if the id is in the list
    struct User * head = malloc(sizeof(struct User));
    head = user;
    while(user != NULL){
        if (user->id == id){
            user->grade = grade;
            return head;}
        user = user -> next;}
    //if the id is not in the list
    struct User * new_user = malloc(sizeof(struct User *));
    new_user -> id = id;
    new_user -> grade = grade;
    new_user -> userType = 0;
    strcpy(new_user -> password, " ");
    sortedInsert(&head, new_user);
    return head;}


//function that makes the login- both for assistant and student
int Login(int id, char * input_password, struct User *user, struct User *assistant){
        while(assistant != NULL){
        if (assistant->id == id){
            if(strcmp(assistant->password, input_password)==0){
                return 1;
            }
            else {return -1;}
        }
        assistant = assistant ->next;}
        
    while(user != NULL){
        if (user->id == id){
            int comp = strcmp(user -> password, input_password);
            if(strcmp(user->password, input_password)==0){
                return 0;}
            else {return -1;}}
        user = user ->next;}
        return -1;}


//Reads if allowed
int ReadGrade(int id, struct User * user, struct loggedIn * connected){
    if (connected -> type == 0){
        if (id != 0){
            return -1; 
        }
        while (user != NULL){
            if (user -> id == connected -> id){
                return user -> grade; 
            }
            user = user -> next;}
    } else if (connected -> type == 1){
        if (id == 0){
            return -3; 
        } else {
            while (user != NULL){
                if (user -> id == id){
                    return user -> grade; 
                }
                user = user -> next;
            }
            return -2; 
        }
    }
    return -2;

}

//Free users list
void free_list(struct User * HEAD){
    struct User * temphead;
    temphead = malloc(sizeof(struct User *));
    temphead = HEAD;
    while (temphead){
        struct User * n = temphead;
        temphead = temphead -> next;
        free(n);
    }

}

void * thread_function(void *arg) {
    while (1) {
        int * pclient;
        pthread_mutex_lock(&mutex);
        if ((pclient = dequeue()) == NULL) {
            pthread_cond_wait(&condition_var, &mutex);
            //try again
            pclient = dequeue();
        }
        pthread_mutex_unlock(&mutex);

        if (pclient != NULL) {
            //we have a connection
            connection_handler(pclient);
        }
    }
}

void freeNode(struct LoggedInUsers ** HEAD, int id){
    struct LoggedInUsers * current = * HEAD;
    struct LoggedInUsers * temp;
    if (current == NULL){
        return;
    }
    else if (current -> id == id) {
        free(current);
        return;
    }
    while (current -> next != NULL){
        if (current -> next -> id == id){
            temp = current -> next;
            current -> next = current -> next -> next;
            free(temp);
            return;
        }

    }
    return;
} //should complete func

void * connection_handler(void* p_client_socket) {
    int client_socket = *((int*)p_client_socket);
    free(p_client_socket); //we really don't need this anymore.
    char buffer[256];
    bzero (buffer,256);
    char op[15];
    char id[10];
    char param[256];

    struct loggedIn * connected;
    connected = malloc(sizeof(struct loggedIn));

    //read from socket//
    while(read(client_socket, buffer, sizeof(buffer)) > 0){
        sscanf(buffer, "%s %s %s", op, id, param);
        //printf("param is: %s, paramlen: %zu\n", param, strlen(param));
        int loggedInUser;
        char operation_type='Z';
        if (strcmp(op, "Login") == 0){
            operation_type = 'L';
        } else if (strcmp(op, "ReadGrade") == 0){
            operation_type = 'R';
        } else if (strcmp(op, "GradeList") == 0){
            operation_type = 'G';
        } else if (strcmp(op, "UpdateGrade") == 0){
            operation_type = 'U';
        } else if (strcmp(op, "Logout") == 0){
            operation_type = 'O';
        } else if (strcmp(op, "Exit") == 0){
            operation_type = 'X';
        } 

        switch (operation_type) {
            case 'L':
            {
                int idnum = atoi(id);
                char end[3] = "\n";
                int res = Login(idnum, param, Students, TA);
                if (res == 0){
                    connected -> type = 0;
                    connected -> isLoggedIn = 1;
                    connected -> id = atoi(id);
                    char welcome[100] = "Welcome Student ";
                    strcat(welcome, id);
                    strcat(welcome, end);
                    int i = send(client_socket,  welcome, sizeof(welcome), 0);
                }
                if (res == 1){
                    char end[3] = "\n";
                    connected -> type = 1;
                    connected -> isLoggedIn = 1;
                    connected -> id = atoi(id);
                    char welcome[100] = "Welcome TA ";
                    strcat(welcome, id);
                    strcat(welcome, end);
                    write(client_socket,  welcome, sizeof(welcome));
                }


                if (res == -1){
                    connected -> isLoggedIn = 0;
                    write(client_socket,  "Wrong user information\n" , sizeof(buffer));
                }
                char zero[10] = {'0'};
                strcpy(id, zero);
                break;
            }
            case 'R':
            {
                if (connected -> isLoggedIn != 1){
                    char buff[20] = "Not logged in\n";
                    send(client_socket, buff, sizeof(buffer), 0);
                    char zero[10] = {'0'};
                    strcpy(id, zero);
                    break;
                }
                char end[3] = "\n";
                int idnum = atoi(id);
                char s_grade[4];
                if (connected -> isLoggedIn == 0){
                    char * notLoggedIn = "Not logged in";
                    send(client_socket,  notLoggedIn, sizeof(notLoggedIn), 0);
                    break;
                }
                int grade = ReadGrade(idnum, Students, connected);
                sprintf(s_grade, "%0d", grade);
                if (grade >= 0) {
                    char end[3] = "\n";
                    strcat(s_grade, end);
                    int i = send(client_socket,  s_grade, sizeof(grade), 0);
                } if (grade == -1){
                    char action_not_allowed[20] = "Action not allowed\n";
                    int i = send(client_socket,  action_not_allowed, sizeof(action_not_allowed), 0);
                } else if (grade == -2){
                    char invalid_id[20] = "Invalid id\n";
                    int i = send(client_socket,  invalid_id, sizeof(invalid_id), 0);
                } else if (grade == -3 ){
                    char missing_argument[20] = "Missing argument\n";
                    int i = send(client_socket,  missing_argument, sizeof(missing_argument), 0);
                char zero[10] = {'0'};
                strcpy(id, zero);
                break;
            }
            case 'G':
            {
                if (connected -> isLoggedIn != 1){
                    char buff[20] = "Not logged in\n";
                    send(client_socket, buff, sizeof(buffer), 0);
                    char zero[10] = {'0'};
                    strcpy(id, zero);
                    char z[256] = {'0'};
                    strcpy(param, z);
                    char done[5] = "Done";
                    send(client_socket, done, sizeof(done), 0);
                    break;
                }
                if (connected -> type == 0) {
                    char action_not_allowed[20] = "Action not allowed\n";
                    char done[256] = "Done";
                    int i = send(client_socket, action_not_allowed, sizeof(action_not_allowed), 0);
                    sleep(2);
                    i = send(client_socket, done, sizeof(done), 0);
                    char zero[10] = {'0'};
                    strcpy(id, zero);
                    break;
                }
                else if(connected -> type == 1){
                    struct User * std_list = malloc(sizeof(struct User *));
                    std_list = Students;
                    char user_pass[256];
                    char done[5] = "Done";
                    while(std_list != NULL){
                        sprintf(user_pass, "%d : %d\n", std_list -> id, std_list -> grade);
                        int i = send(client_socket, user_pass, sizeof(user_pass), 0);
                        std_list = std_list ->next;
                    }
                    int i = send(client_socket, done, sizeof(done), 0);
                    free(std_list);
                }
                break;
            }
            case 'U':
            {
                if (connected -> isLoggedIn != 1){
                    char buff[20] = "Not logged in\n";
                    send(client_socket, buff, sizeof(buffer), 0);
                    char zero[10] = {'0'};
                    strcpy(id, zero);
                    char done[5] = "Done";
                    send(client_socket, done, sizeof(done), 0);
                    break;
                }
                if (connected -> type == 0){
                    char action_not_allowed[20] = "Action not allowed\n";
                    int i = send(client_socket,  action_not_allowed, sizeof(action_not_allowed), 0);
                }
                else if (connected -> type == 1){
                    int idnum = atoi(id);
                    int grade = atoi(param);
                    pthread_mutex_lock(&mutex);
                    Students = UpdateGrade(idnum, Students, grade);
                    pthread_mutex_unlock(&mutex);
                }
                char * res = "";
                int i = send(client_socket,  res, sizeof(res), 0);
                char zero[10] = {'0'};
                strcpy(id, zero);
                break;
            }
            case 'O':
            {
                char buff[256];
                char strid[10];
                if (connected -> isLoggedIn == 0){
                    char * notLoggedIn = "Not logged in\n";
                    char buff[256];
                    int i = send(client_socket,  notLoggedIn, sizeof(buff), 0);
                    break;
                }
                char goodbye[20] = "Goodbye ";
                sprintf(strid, "%0d\n", connected -> id);
                strcat(goodbye, strid);
                connected -> id = 0;
                connected -> isLoggedIn = 0;
                char * res = " \n";
                int i = send(client_socket,  goodbye, sizeof(goodbye), 0);
                break;
            }
            case 'X':
            {
                char bye[256] = "BYE";
                int i = send(client_socket,  bye, sizeof(bye), 0);
                sleep(2);
                free(connected);
                close(client_socket);
            }
            case 'Z':
            {
                char exitprog[256] = "Wrong input\n";
                send(client_socket, exitprog, sizeof(exitprog), 0);
                char id_to_0[10] = {'0'};
                char param_to_0[256] = {'0'};
                strcpy(id, id_to_0);
                strcpy(param, param_to_0);
                break;
            }
        }}
}
close(client_socket);
return NULL;
}
int main(int argc,char *argv[]){//creating thread pool
    for (int i=0; i < N; i++) {
        pthread_create(&thread_pool[i], NULL, thread_function, NULL);
    }
    int sockfd, portno, n, client_socket;
    socklen_t lenthOfClient;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;

    if (argc<2) {
        fprintf (stderr,"ERROR,no port provided\n");
        exit (1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM,0);
    if (sockfd<0)
        error ("ERROR opening socket");
    bzero ((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi (argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd,(struct sockaddr *)&serv_addr,sizeof (serv_addr))<0)
        error ("ERROR on binding");

    listen (sockfd,5);
    int c = sizeof(struct sockaddr_in);

    studfile = openfile("students.txt");
    assistantsfile = openfile("assistants.txt");
    if (studfile && assistantsfile) {
        Students = create_list(studfile, 0);
        TA = create_list(assistantsfile, 1);
    }

    while(client_socket = accept(sockfd, (struct sockaddr *)&cli_addr, (socklen_t *)&lenthOfClient)){
        int * pclient = malloc(sizeof(int));
        *pclient = client_socket;
        pthread_mutex_lock(&mutex);
        enqueue(pclient);
        pthread_cond_signal(&condition_var);
        pthread_mutex_unlock(&mutex);
    }
    return 0;
}