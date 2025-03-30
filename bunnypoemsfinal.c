#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>


#define MAX_POEM_LENGTH 100
#define MAX_FILE_NAME_LENGTH 50
#define MSG_SIZE MAX_POEM_LENGTH + 1

// Structure for message queue
struct msg_buffer {
    long msg_type;
    char msg_text[MSG_SIZE];
};


// Function to add a new poem to the data file
void addPoem(FILE *file) {
    char poem[MAX_POEM_LENGTH];
    printf("Enter the new poem (press Enter twice to finish):\n");
    fgets(poem, sizeof(poem), stdin);
    // Read multiple lines until an empty line is encountered
    while (strcmp(poem, "\n") != 0) {
        fprintf(file, "%s", poem);
        fgets(poem, sizeof(poem), stdin);
    }
    printf("New poem added successfully!\n");
}

// Function to list all poems from the data file
void listPoems(FILE *file) {
    char poem[MAX_POEM_LENGTH];
    int index = 1;
    printf("List of poems:\n");
    rewind(file); // Reset file pointer to the beginning
    while (fgets(poem, sizeof(poem), file) != NULL) {
        printf("%d. %s", index++, poem);
    }
}

// Function to delete a poem from the data file by index
void deletePoem(FILE *file) {
    int index;
    printf("Enter the poem line you want to delete: ");
    scanf("%d", &index);
    getchar(); // Consume the newline character
    
    // Create a temporary file to store poems except the one to be deleted
    FILE *temp_file = fopen("temp.txt", "w");
    rewind(file);
    char poem[MAX_POEM_LENGTH];
    int currentIndex = 1;
    while (fgets(poem, sizeof(poem), file) != NULL) {
        if (currentIndex != index) {
            fprintf(temp_file, "%s", poem);
        }
        currentIndex++;
    }
    fclose(temp_file); // Close the temporary file
    fclose(file);
    
    // Remove the original file and rename the temporary file
    remove("poems.txt");
    rename("temp.txt", "poems.txt");
    printf("Poem deleted successfully!\n");
}

// Function to modify an existing poem in the data file by index
void modifyPoem(FILE *file) {
    int index;
    printf("Enter the poem line you want to modify: ");
    scanf("%d", &index);
    getchar(); // Consume the newline character
    
    // Create a temporary file to store modified poem
    FILE *temp_file = fopen("temp.txt", "w");
    rewind(file);
    char poem[MAX_POEM_LENGTH];
    int currentIndex = 1;
    while (fgets(poem, sizeof(poem), file) != NULL) {
        if (currentIndex == index) {
            char newPoem[MAX_POEM_LENGTH];
            printf("Enter the new version of the poem (press Enter twice to finish):\n");
            fgets(newPoem, sizeof(newPoem), stdin);
            // Read multiple lines until an empty line is encountered
            while (strcmp(newPoem, "\n") != 0) {
                fprintf(temp_file, "%s", newPoem);
                fgets(newPoem, sizeof(newPoem), stdin);
            }
        } else {
            fprintf(temp_file, "%s", poem);
        }
        currentIndex++;
    }
    fclose(temp_file);
    fclose(file);
    
    // Remove the original file and rename the temporary file
    remove("poems.txt");
    rename("temp.txt", "poems.txt");
    printf("Poem modified successfully!\n");
}

void waterGirls(FILE *file) {
    // Array to store child process IDs
    pid_t children[4];

    // Forking four child processes to simulate the bunny boys
    for (int i = 0; i < 4; i++) {
        pid_t pid = fork();

        if (pid < 0) {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Child process behavior
            sleep(1); // Add sleep time before printing
            printf("Bunny Boy %d (%d) is ready to water the girls in Baratfa!\n", i+1, getpid());
            // The child process exits after printing its readiness
            exit(EXIT_SUCCESS);
        } else {
            // Parent process tracks the child process IDs
            children[i] = pid;
        }
    }

    // Randomly choosing one bunny boy
    srand(time(NULL));
    int chosenIndex = rand() % 4;
    pid_t chosenBoy = children[chosenIndex];
    sleep(1); // Add sleep time before printing
    printf("Bunny Boy %d (%d) is chosen to water the girls in Baratfa!\n", chosenIndex+1, chosenBoy);

    // Creating pipe
    int fd[2];
    if (pipe(fd) == -1) {
        perror("Pipe failed");
        exit(EXIT_FAILURE);
    }

    // Forking a child process for the chosen bunny boy
    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }

    // Child process for the chosen bunny boy
    if (pid == 0) {
        // Closing write end
        close(fd[1]);

        // Sending 2 poems to the chosen bunny boy via pipe
        char poem1[MAX_POEM_LENGTH], poem2[MAX_POEM_LENGTH];
        rewind(file);
        fgets(poem1, sizeof(poem1), file); // Read the first poem
        fgets(poem2, sizeof(poem2), file); // Read the second poem

        sleep(1); // Add sleep time before printing
        printf("\nReceived poems from Mama Bunny:\n1. %s2. %s", poem1, poem2);

        // Selecting a poem
        srand(time(NULL));
        int selected = rand() % 2 + 1;
        sleep(1); // Add sleep time before printing
        printf("\nSelected poem: %d\n", selected);

        // Send selected poem back to Mama Bunny via message queue
        key_t key = ftok("poems.txt", 'A');
        int msgid = msgget(key, 0666 | IPC_CREAT);
        struct msg_buffer message;
        message.msg_type = 1;
        if (selected == 1)
            strcpy(message.msg_text, poem1);
        else
            strcpy(message.msg_text, poem2);
        msgsnd(msgid, &message, sizeof(message), 0);

        // Bunny boy tells the poem and waters the girls
        sleep(1); // Add sleep time before printing
        printf("Bunny Boy %d (%d): %sMay I water! ", chosenIndex+1, chosenBoy, message.msg_text);
        fflush(stdout);
        sleep(1);
        printf("\nWatering...\n");
        fflush(stdout);
        sleep(3);
        printf("Watering finished.");

        exit(EXIT_SUCCESS);
    } else { // Parent process
        // Sending 2 poems to the chosen bunny boy via pipe
        char poem1[MAX_POEM_LENGTH], poem2[MAX_POEM_LENGTH];
        rewind(file);
        fgets(poem1, sizeof(poem1), file); // Read the first poem
        fgets(poem2, sizeof(poem2), file); // Read the second poem
        write(fd[1], poem1, strlen(poem1)+1);
        write(fd[1], poem2, strlen(poem2)+1);

        // Closing write end
        close(fd[1]);

        // Waiting for the chosen bunny boy process to finish
        waitpid(pid, NULL, 0);
    }
}



int main() {
    FILE *file;
    char fileName[MAX_FILE_NAME_LENGTH];
    strcpy(fileName, "poems.txt");

    // Open the data file in append mode
    

    int choice;
    do {
        file = fopen(fileName, "a+");
        if (file == NULL) {
            printf("Error opening file!\n");
            return 1;
        }
        printf("\n1. Add a new poem\n2. List all poems\n3. Delete a poem\n4. Modify a poem\n5. Watering\n6. Exit\n");
        printf("Enter your choice: ");
        // Validate input to ensure it's a number between 1 and 5
        if (scanf("%d", &choice) != 1 || choice < 1 || choice > 6) {
            printf("Invalid choice! Please enter a number between 1 and 6.\n");
            // Clear input buffer in case of invalid input
            while (getchar() != '\n');
            continue; // Skip to next iteration of the loop
        }
        getchar(); // Consume the newline character

        switch (choice) {
            case 1:
                addPoem(file);
                fclose(file);
                break;
            case 2:
                listPoems(file);
                fclose(file);
                break;
            case 3:
                deletePoem(file);
                break;
            case 4:
                modifyPoem(file);
                break;
            case 5:
                waterGirls(file);
                fclose(file);
                break;
            case 6:
                printf("Exiting...\n");
                fclose(file);
                break;
                
        }
    } while (choice != 6);
    return 0;
}