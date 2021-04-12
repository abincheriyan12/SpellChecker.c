//Abin Cheriyan
//Assignment 3 Network Spell Checker
//

/*
 includes code from the book and examples. will be cited at the end.
*/

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#define defaultport 8888
#define defaultdictionary "dictionary.txt"
#define dictionarylength 99171
#define maxlength 100
#define maxsize 100000
#define maxQsize 100
#define threads 5



char **dictionary;
//int openlistenfd(int);       //provided
void *workerthread(void *);  // for worker threads
void *loggerthread(void *);   // logger thread
void callworkerthreads();     //calls worker thread / creates them
char* wordformat(char *word);  // formatting the word

int variables();           //initializing variables
int numdictionary();       //number of words
int getword(char *word);   //getting the wrd
int getinput(char* userword); //getting input
void printdictionary();     //prints the dictioanry
int search(char *word, int i);  //searching for the word

//connection Queue
void putconnection(int socket);
int getconnection();
//log queue
void putlog(char *word);
char *getlog();


char *filename;
FILE *words;
int numWords;

// connection queue

int connectionQ[maxQsize];
int cwrite;
int cread;
int cQsize;

// log queue

char *logQ[maxQsize];
int logwrite;
int logread;
int logQsize;

pthread_cond_t logempty;
pthread_cond_t connectionempty;
pthread_cond_t logfill;
pthread_cond_t connectionfill;

pthread_mutex_t l;
pthread_mutex_t c;


int main(int argc, char *argv[])
{

    int portNumber;

    if (argc == 1)
    {
        portNumber = defaultport;
        filename = defaultdictionary;  //dictionary.txt
    }
    else if (argc == 2)
    {
        int num = atoi(argv[1]);
        if (num > 0)
        {
            portNumber = atoi(argv[1]);
            printf("Port Number: %d\n", portNumber);
            filename = defaultdictionary;

        } else {
            portNumber = defaultport;
            filename = argv[1];
            printf("Filename: %s\n", filename);
        }
    }
    else if (argc == 3)
    {
        int num = atoi(argv[1]);
        if (num > 0 )
        {
            portNumber = atoi(argv[1]);
            filename = argv[2];

            printf("Port Number is: %d\n",portNumber );
            printf("Filename: %s\n",filename);
        }
        else {

            portNumber = atoi(argv[2]);
            filename = argv [2];
            printf("The Port Number is: %d\n",portNumber );
            printf("Filename: %s\n",filename);

        }
    }

    /* Checks if port is in the proper range. */
    if (portNumber < 1024 || portNumber > 65535)
    {
        printf("Please enter a port number between 1024 and 65535.\n");
        exit(1);
    }

    // network initialization
    int sdescriptor, c;
    struct sockaddr_in server, client;
    //char *message;
    // Create socket (create active socket descriptor)
    sdescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (sdescriptor == -1)
    {
        puts("Error creating socket!");
        exit(1);
    }
    // prepare sockaddr_instructure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY; // defaults to 127.0.0.1
    server.sin_port = htons(portNumber);
    // Bind (corrent the server's socket address to the socket descriptor)
    int bind_result = bind(sdescriptor, (struct sockaddr*)&server, sizeof(server));
    if (bind_result < 0)
    {
        puts("Error: failed to Bind.");
        exit(1);
    }
    puts("Done:Connected");
    // Listen (converts active socket to a LISTENING socket which can accept connections)
    listen(sdescriptor, 20);
    //puts("Waiting for incoming connections...");

    if( ! variables ())
    {
        exit(0);
    }

    callworkerthreads();
    puts("Waiting for incoming connections...");
    numWords = numdictionary();
    char *welcome = "Welcome to spellchecker - Enter -1 to close the connection with the server\n";

    while (1)
    {
        //int client_socket = accept(sdescriptor, (struct sockaddr*)&client, &client);
        int fd = accept(sdescriptor, (struct sockaddr *) &client, (socklen_t *) &c);
        if (fd < 0)
        {
            printf("cant connect to client : %d\n", fd);
            continue;
        }

        printf("connection accepted. Client ID: %d\n", fd);
        send(fd, welcome, strlen(welcome), 0);
        //puts("Welcome to spellchecker - Enter -1 to close the connection with the server\n");
        putconnection(fd);

    }

    size_t i;
    for (i = 0; dictionary[i] != NULL; i++)
    {
        free(dictionary[i]);
    }
    free(dictionary);

}


int open_listenfd(int port)
{
    int listenfd, optval = 1;
    struct sockaddr_in serveraddr;

    /* Create a socket descriptor */
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        return -1;
    }

    /* Eliminates "Address already in use" error from bind */
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int)) < 0){
        return -1;
    }

    //Reset the serveraddr struct, setting all of it's bytes to zero.
    //Some properties are then set for the struct, you don't
    //need to worry about these.
    //bind() is then called, associating the port number with the
    //socket descriptor.
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)port);
    if (bind(listenfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0){
        return -1;
    }

    //Prepare the socket to allow accept() calls. The value 20 is
    //the backlog, this is the maximum number of connections that will be placed
    //on queue until accept() is called again.
    if (listen(listenfd, 20) < 0)
    {
        return -1;
    }
    return listenfd;
}

void callworkerthreads()
{
    size_t i;
    pthread_t workerthreads[threads];    //array to hold the threads
    for (i = 0; i < threads ; ++i)  //creating worker threads
    {
        if (pthread_create(&workerthreads[i],NULL,workerthread,NULL)!= 0)
        {
            perror("Failed to create a thread\n");
            exit(1);
        }
    }

    puts("Worker Threads created");
    //size_t j;
    pthread_t logthreads;    // create logger threads
    pthread_create(&logthreads, NULL, &loggerthread, NULL);

    if (pthread_create(&logthreads, NULL, loggerthread, NULL) != 0)
    {
        perror("Error: Failed to create thread\n");
        exit(1);
    }else{
        puts("Logger thread created\n");
    }

}

void *workerthread(void *args)
{
    char *prompt_msg = "Word checked >> \n";
    char *close_msg = "Connection with the server is closed.\n";
    //char *welcome = "Welcome to spellchecker - Enter -1 to close the connection with the server\n";
    while (1)
    {
        //char *getresponse;
        //char recvbuffer= [256] = "";
        char *word = (char *) malloc(sizeof(char) * 32); // allocating buffer
        int socket = getconnection();
        //int bytesreturned = recv(socket, recvbuffer, 256, 0);
        //send(socket, prompt_msg, strlen(prompt_msg), 0);


        while (read (socket, word, 32) > 0)
        {
            send(socket, prompt_msg, strlen(prompt_msg), 0);
            word = wordformat(word); // removing trialing chars

            //strcasecmp, strncasecmp - compare two strings ignoring case
            if (search(word, numWords))
            {
                strcat(word, " OK\n");
                puts(word);
            }else{
                strcat(word, " MISSPELLED\n");
                puts(word);
            }
            if (atoi(&word[0]) == -1)
            {
                send(socket, close_msg, strlen(close_msg), 0);
                //puts(close_msg);
                //printf("Connection with the server is closed\n");
                close(socket);
                break;
            }

            write(socket, word, strlen(word) + 1);
            putlog(word);                                   // instead of a function found in the book
            word = (char *) malloc (sizeof(char)*32);       //allocatioon buffer
        }
        close(socket);
    }
}

// searches for a word in the dictionary, if found returns 1
int search(char *word, int i)
{
    for(size_t j = 0; j < i; j++)
    {
        if(strcasecmp(dictionary[j], word) == 0)
        {
            return 1;
        }
    }
    return 0;
}

void *loggerthread(void *args)
{
    // create log file
    FILE *loggerfile = fopen("log.txt", "w");    //creating
    fclose(loggerfile);                          //closing
    loggerfile = fopen("log.txt", "a");          //appending

    while (1)
    {
        char *word = getlog();
        fprintf(loggerfile, "%s", word);
        fflush(loggerfile);
        free(word);
    }
    fclose(loggerfile);
}

char* wordformat(char *word)
{
    size_t i;
    for(i = 0; word[i] != '\0'; i++)
    {
        if (word[i] == '\n')
        {
            word[i] = '\0';
            return word;
        }
        else if (word[i] == '\t')
        {
            word[i] = '\0';
            return word;
        }
        else if (word[i] == '\r')
        {
            word[i] = '\0';
            return word;
        }
        else if (word[i] == ' ')
        {
            word[i] = '\0';
            return word;
        }
    }
    return word;
}


int numdictionary()
{
    words = fopen(defaultdictionary, "r"); // opening the dictionary in read mode

    if (words == NULL)
    {
        perror("Error opening the text file\n");
        exit(0);
    }

    char word[maxlength];

    dictionary = calloc(maxsize, sizeof(char *));  // allocating dictionary

    // while it hasnt hit EOF, get the next word from file and add it to the dictionary
    int i = 0;
    while(getword(word) != -1)
    {
        dictionary[i] = (char *) malloc(100);
        if(word[strlen(word) - 1] == '\n')
        {
            word[strlen(word) - 1] = '\0';
        }
        strcpy(dictionary[i], word);
        i++;
    }

    fclose(words);

    return i; // returns the number of words

}

int getword(char *word)
{
    size_t max = maxlength;
    int c = getline(&word, &max, words);
    return c;
}

int variables()
{
    /// will initialize variables and perror if there are errors and returns 0

    if(pthread_cond_init(&logfill, NULL) != 0)
    {
        puts("error");
        exit(0);

    }
    if(pthread_cond_init(&connectionempty, NULL) != 0)
    {
        puts("error");
        exit(0);

    }
    if(pthread_cond_init(&connectionfill, NULL) != 0)
    {
        puts("error");
        exit(0);

    }
    // initializing mutex
    if(pthread_mutex_init(&c, NULL) != 0)
    {
        puts("error");
        exit (0);
    }
    if(pthread_mutex_init(&l, NULL) != 0)
    {
        puts("error");
        exit(0);
    }
    return 1;

}
void printdictionary()
{
    for(size_t i = 0; dictionary[i] != NULL; i++)
    {
        puts(dictionary[i]);
    }
}


// still have stuff to do

// adds socket description to connection queue (textboook)
void putconnection(int socket)
{
    pthread_mutex_lock(&c); //locks so it doesnt get messed by others
    while (cQsize == maxQsize)
    {
        pthread_cond_wait(&connectionempty, &c); //if connection Q is empty
    }

    connectionQ[cwrite] = socket;
    cwrite = (cwrite + 1) % maxQsize;
    cQsize++;

    pthread_cond_signal(&connectionfill);
    pthread_mutex_unlock(&c);                //unlocking since we are done
}

//removing and returning socket descriptor from the connection Q (textbook)
int getconnection()
{
    pthread_mutex_lock(&c); //locks so it doesnt get messed by others
    while (cQsize == 0)
    {
        pthread_cond_wait(&connectionfill, &c); //if connection Q is empty
    }

    int socket  = connectionQ[cread];  //opposite of putting connection
    cread = (cread + 1) % maxQsize;
    cQsize--;

    pthread_cond_signal(&connectionempty);
    pthread_mutex_unlock(&c);

    return socket;
}

//adding string to the log Q (textbook)
void putlog(char *word)
{
    pthread_mutex_lock(&l); //locks so it doesnt get messed by others

    while (logQsize == maxQsize)
    {
        pthread_cond_wait(&logempty, &l); //if connection Q is empty
    }

    logQ[logwrite] = word;
    logwrite = (logwrite + 1) % maxQsize;
    logQsize++;

    pthread_cond_signal(&logfill);
    pthread_mutex_unlock(&l);
}

// removing and returning a string from the log Q (textbook copy)
char* getlog()
{
    pthread_mutex_lock(&l); //locks so it doesnt get messed by others

    while (logQsize == 0)
    {
        pthread_cond_wait(&logfill, &l); //if connection Q is empty
    }

    char *word  = logQ[logread];  //opposite of putting connection
    logread = (logread + 1) % maxQsize;
    logQsize--;

    pthread_cond_signal(&logempty);
    pthread_mutex_unlock(&l);

    return word;
}