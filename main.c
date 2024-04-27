#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>


#define MAX_POEM_LENGTH 100
#define DATA_FILE "poems.txt"

time_t time( time_t *second );
key_t ftok(const char *path, int id);

struct Message {
    long mtype;
    char mtext[MAX_POEM_LENGTH];
};

FILE *openFile(const char *filename, const char *mode) {
    FILE *file = fopen(filename, mode);
    if (file == NULL) {
        printf("Error opening file %s.\n", filename);
        exit(EXIT_FAILURE);
    }
    return file;
}

void addPoem() {
    FILE *file = openFile(DATA_FILE, "a+");

    char poem[MAX_POEM_LENGTH];
    printf("Enter the new poem: ");
    getchar(); 
    fgets(poem, MAX_POEM_LENGTH, stdin);
    printf("%s\n",poem);

    fprintf(file,"%s",poem);
    fclose(file);

    printf("Poem added successfully!\n");
}

void listPoems() {
    FILE *file = openFile(DATA_FILE, "a+");

    char poem[MAX_POEM_LENGTH];
    int count = 0;
    printf("List of poems:\n\n\n");
    while (fgets(poem, MAX_POEM_LENGTH, file) != NULL) {
        printf("%d. %s", ++count, poem);
    }
    printf("\n\n\n");

    fclose(file);
}

void deletePoem() {
    FILE *file = openFile(DATA_FILE, "r");

    char poem[MAX_POEM_LENGTH];
    listPoems();

    printf("Enter the number of the poem to delete: ");
    int poemNumber;
    scanf("%d", &poemNumber);

    FILE *tempFile = fopen("temp.txt", "w");
    if (tempFile == NULL) {
        printf("Error creating temporary file.\n");
        fclose(file);
        return;
    }

    int count = 0;
    while (fgets(poem, MAX_POEM_LENGTH, file) != NULL) {
        count++;
        if (count != poemNumber) {
            fprintf(tempFile, "%s", poem);
        }
    }

    fclose(file);
    fclose(tempFile);

    remove(DATA_FILE);
    rename("temp.txt", DATA_FILE);

    printf("Poem deleted successfully!\n");
}

void modifyPoem() {
    FILE *file = openFile(DATA_FILE, "r");
    
    char poem[MAX_POEM_LENGTH];
    listPoems();

    printf("Enter the number of the poem to modify: ");
    int poemNumber;
    scanf("%d", &poemNumber);

    char modified_poem[MAX_POEM_LENGTH];
    printf("Enter the modified poem: ");
    getchar(); 
    fgets(modified_poem, MAX_POEM_LENGTH, stdin);
    printf("%s\n",modified_poem);
    FILE *tempFile = fopen("temp.txt", "w");

    int count = 0;
    while (fgets(poem, MAX_POEM_LENGTH, file) != NULL) {
        count++;
        if (count == poemNumber) {
            strcpy(poem, modified_poem);
            fprintf(tempFile, "%s\n", poem);
        } else {
            fprintf(tempFile, "%s", poem);
        }
    }

    fclose(file);
    fclose(tempFile);

    remove(DATA_FILE);
    rename("temp.txt", DATA_FILE);

    printf("Poem modified successfully!\n");
}

// Function to randomly choose a Hungarian boys' name and return it
const char* getRandomHungarianName() {
    // Array of Hungarian boys' names
    const char *names[] = {"Bálint", "Levente", "Gergő", "Dávid"};
    int num_names = sizeof(names) / sizeof(names[0]);
    
    // Seed the random number generator
    srand(time(NULL));
    
    // Generate a random index within the range of the array
    int random_index = rand() % num_names;
    
    // Return the randomly chosen name
    return names[random_index];
}

void Handler(int signal) {
    if (signal == SIGUSR1) {
        printf("Hi,mom! I got to Barátfa.\n");
    }
}

char* chooseTwoPoem(){
    FILE *file = openFile(DATA_FILE, "r");
    char line[MAX_POEM_LENGTH];
    char *chosenPoems[2] = {NULL, NULL};
    int count = 0;
    srand(time(NULL));

    // Read the file to select two random poems
    while (fgets(line, sizeof(line), file) != NULL) {
        if (count < 2) {
            chosenPoems[count] = strdup(line);
            count++;
        } else {
            int randomIndex = rand() % (count + 1);
            if (randomIndex < 2) {
                free(chosenPoems[randomIndex]);
                chosenPoems[randomIndex] = strdup(line);
            }
        }
    }

    fclose(file);

    // Concatenate the chosen poems into a single string
    char *result = malloc(MAX_POEM_LENGTH * 2); // Allocate enough memory
    strcpy(result, chosenPoems[0]);
    strcat(result, chosenPoems[1]);

    // Free memory for chosen poem strings
    free(chosenPoems[0]);
    free(chosenPoems[1]);

    return result;
}

int send_message(int mqueue, const struct Message m) {
    int status;
    status = msgsnd(mqueue, &m, strlen(m.mtext) + 1, 0);
    if (status < 0)
        perror("msgsnd error");
    return 0;
}

int receive_message(int mqueue) {
    struct Message m;
    int status;
    status = msgrcv(mqueue, &m, MAX_POEM_LENGTH, 5, 0);
    if (status < 0)
        perror("msgrcv error");
    else
        printf("The code of the message is: %ld, text is: %s\n", m.mtype, m.mtext);
    return 0;
}

void Watering(key_t key) {
    int pipefd[2]; // Pipe file descriptors
    pid_t child_pid;
    int messg, status; 
    char buffer[100];

    // user can read and write
    messg = msgget(key, 0600 | IPC_CREAT);
    if (messg < 0) {
        perror("msgget error");
        exit(EXIT_FAILURE);
    }
    const char *random_name = getRandomHungarianName();

    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    child_pid = fork();
    if (child_pid == -1) {
        perror("fork error");
        exit(EXIT_FAILURE);
    } else if (child_pid == 0) { 
        // Child process (Bunny Boy)
        // Close read end of the pipe
        close(pipefd[0]);
        printf("This is %s",random_name);
        // Send signal to parent process
        kill(getppid(), SIGRTMIN);

        // Write to the pipe to notify parent
        write(pipefd[1], "Ok! I am gonna send two poems.", sizeof("Ok! I am gonna send two poems."));
        printf("%d",1);
        receive_message(messg);  // Parent sends a message. 
         printf("%d",2);
        // Close write end of the pipe
        close(pipefd[1]);

        exit(EXIT_SUCCESS);
    } else {
        // Parent process (Mama Bunny)
        close(pipefd[1]);
        // Wait for signal from child
        signal(SIGRTMIN, Handler);
        read(pipefd[0], buffer, sizeof(buffer));
        pause(); // Pause the parent process until a signal is received
        printf("%s\n", buffer);
        const struct Message m = { 5, *chooseTwoPoem() }; 
        send_message(messg,m);
        wait( NULL ); 
          // After terminating child process, the message queue is deleted. 
          status = msgctl( messg, IPC_RMID, NULL ); 
          if ( status < 0 ) 
               perror("msgctl error"); 
        close(pipefd[0]);
    }
}

int main(int argc, char* argv[]) {
    // msgget needs a key, created by ftok
    key_t key = ftok(argv[0], 1);
    int choice;
    do {
        printf("\n1. Add a new poem\n");
        printf("2. List all poems\n");
        printf("3. Delete a poem\n");
        printf("4. Modify a poem\n");
        printf("5. Watering\n");
        printf("6. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                addPoem();
                break;
            case 2:
                listPoems();
                break;
            case 3:
                deletePoem();
                break;
            case 4:
                modifyPoem();
                break;
            case 5:
                Watering(key);
            case 6:
                printf("Exiting...\n");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    } while (choice != 6);

    return 0;
}
