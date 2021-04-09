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

#define defaultport 8080
#define defaultdictionary dictionary.txt
#define dictionarylength 99171
#define maxlength 100
#define maxsize 100000
#define maxQsize 100
#define threads 10



//Function that will be implemented as i finish
char **dictionary;
int openlistenfd(int);       //provided
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
            printf("\nport number: %d\n", portNumber);
            filename = "dictionary.txt";

            /* Checks if port is in the proper range. */
            if (portNumber < 1024 || portNumber > 65535)
            {
                printf("Please enter a port number between 1024 and 65535.\n");
                exit(1);
            }
        } else {
            portNumber = defaultport;
            filename = argv[1];
            printf("\nfilename: %s\n", filename);
        }
    }
    else if (argc == 3)
    {
        ////////////////////////////

        /* Checks if port is in the proper range. */
        if (portNumber < 1024 || portNumber > 65535)
        {
            printf("Please enter a port number between 1024 and 65535.\n");
            exit(1);
        }

    }


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
    if (listen(listenfd, 20) < 0) {
        return -1;
    }
    return listenfd;
}

void callworkerthreads()
{
    pthread_t workerthreads[threads];    //array to hold the threads
    for (size_t i = 0; i < threads ; ++i)  //creating worker threads
    {
        if (pthread_create(&workerthreads[i],NULL,workerthread,NULL)!= 0)
        {
            perror("Failed to create a thread\n");
            exit(1);
        }
    }

    pthread_t logthreads[threads];    // create logger threads
    for (size_t i = 0; i < threads; ++i)
    {
        if (pthread_create(&logthreads[i],NULL,loggerthread,NULL) != 0)
        {
            perror("Error: Failed to create thread\n");
            exit(1);
        }
    }
}

void *workerthread(void *args) {
    while (1) {
        char *getresponse;
        char *word = (char *) malloc(sizeof(char) * 32); // allocating buffer

        ////
        ////
    }
}

void *loggerthread(void *args)
{
    // create log file
    FILE *loggerfile = fopen("log.txt", "w");
    fclose(loggerfile);
    loggerfile = fopen("log.txt", "a");

    while (1)
    {
        //

    }

    fclose(loggerfile);
}

char* wordformat(char *word)
{
 //
}


int numdictionary()
{
    words = fopen("dictionary.txt", "r"); // open the dictionary in read mode

    if (words == NULL)
    {
        perror("error opening file\n");
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

// gets a word from user input
int getinput(char* userword)
{
    puts("enter a word");
    if (fgets(userword, maxlength, stdin) == NULL)
    {
        return 0;
    }
    return 1;

}
int variables()
{
    /// will initialize variables and perror if there are errors and returns 0

}

// still have stuff to do
