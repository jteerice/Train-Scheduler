#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_STATION_LENGTH (25)
#define SIZE 500

typedef struct train_route_def
{
    int origin_station;
    int dest_station;
    int start_time;
    int end_time;
    int duration;
    struct train_route_def *next;
} train_route;

int getChoice();
void argCheck(int argc);
int getVal();
void read_file_stations(FILE *file, char stations[][MAX_STATION_LENGTH]);
train_route *read_file_trains(FILE *file, train_route *routes);
train_route *make_route(int origin_station, int dest_station, int start_time, int end_time);
train_route *add_route(train_route *routes, train_route *new_route);
bool direct_route(train_route *routes, int origin_station, int dest_station);
train_route *shortest_time(train_route *routes);
void list_routes(train_route *routes, char stations[][MAX_STATION_LENGTH], int time);
int find_station_number(int num_stations, char stations[][MAX_STATION_LENGTH], char search_str[MAX_STATION_LENGTH]);
void find_station_name(char stations[][MAX_STATION_LENGTH], int station_num, char station_name[MAX_STATION_LENGTH]);
int convert_to_24(int minute_time);
int convert_from_24(int time_24);
int line_count(FILE *file);

int main(int argc, char *argv[]) {

    int stationNum = 0;
    int stationOrig = 0;
    int stationDest = 0;
    char stationName[MAX_STATION_LENGTH];
    FILE *ifp1 = NULL;
    FILE *ifp2 = NULL;
    train_route *head = NULL;
    train_route *shortestRoute = NULL;

    argCheck(argc); // Checking for correct amount of command line arguments
    ifp1 = fopen(argv[1], "r");
    if (ifp1 == NULL) {
        printf("Could not open %s for reading.\n", argv[1]);
        exit(1);
    }
    ifp2 = fopen(argv[2], "r");
    if (ifp2 == NULL) {
        printf("Could not open %s for reading.\n", argv[2]);
        exit(1);
    }

    char stations[line_count(ifp1)][MAX_STATION_LENGTH];   
    read_file_stations(ifp1, stations);
    head = read_file_trains(ifp2, head);

    // Loop continues until user quits the program
    // User is prompted for new menu option if user enters invalid option number
    while (true) {
        printf("\n\n------------------------------------------------------\n");
        printf("             Welcome to the Train Program\n");
        printf("------------------------------------------------------\n\n");
        printf("   0) Quit\n");
        printf("   1) Find a station number\n");
        printf("   2) Find a station name\n");
        printf("   3) Is there a direct path between 2 stations?\n");
        printf("   4) Find shortest time on train for a direct trip\n");
        printf("   5) List all routes after a specified time (hhmm)\n\n");

        switch(getChoice()) {
            case 0:
                printf("Quitting program...\n\n");
                exit(1);
                break;

            case 1:
                printf("Enter a station name: ");
                fgets(stationName, MAX_STATION_LENGTH, stdin);
                stationName[strcspn(stationName, "\n")] = 0;

                if (find_station_number(line_count(ifp1), stations, stationName) == -1) {
                    printf("%s station does not exist.\n", stationName);
                    printf("Make sure there isn't a typing error.\n\n");
                }
                else {
                    printf("%s is station number %d\n\n", stationName, find_station_number(line_count(ifp1), stations, stationName));
                }

                break;

            case 2:
                printf("Enter a station #: ");
                stationNum = getVal();

                if (stationNum < 0 || stationNum > (line_count(ifp1) - 1)) {
                    printf("Station does not exist\n\n");
                }
                else {
                    find_station_name(stations, stationNum, stationName);
                    printf("Station %d is %s\n", stationNum, stationName);
                }

                break;

            case 3:
                printf("Enter origin station number: ");
                stationOrig = getVal();
                printf("Enter destination station number: ");
                stationDest = getVal();

                if (direct_route(head, stationOrig, stationDest) == true) {
                    printf("There is a direct path between station %d and station %d\n\n", stationOrig, stationDest);
                }
                else {
                    printf("There is NOT a direct path between station %d and station %d\n\n", stationOrig, stationDest);
                }

                break;

            case 4:
                printf("The shortest route is from\n");
                shortestRoute = shortest_time(head); // Saving the shortest route into a placeholder pointer so the function doesn't have to be called numerous times
                printf("%3d: %s @ %d\n", shortestRoute->origin_station, stations[shortestRoute->origin_station], shortestRoute->start_time);
                printf("     to\n");
                printf("%3d: %s @ %d\n\n", shortestRoute->dest_station, stations[shortestRoute->dest_station], shortestRoute->end_time);
                break;

            case 5:
                printf("Enter earliest starting time in 24-hour format (hhmm): ");
                list_routes(head, stations, getVal());
                break;

            default:
                printf("Invalid choice: please restart and select valid menu option\n\n");
                exit(1);
                break; 
        }
    }

    fclose(ifp1);
    fclose(ifp2);
    return 0;
}

// Reading file into the stations 2D array
void read_file_stations(FILE *file, char stations[][MAX_STATION_LENGTH]) {

    int i = 0;
    int count = line_count(file);
    int index = 0;

    for (i = 0; i < count; i++) {
        fscanf(file, "%d", &index);
        fscanf(file, "%s", stations[index]); // Station number used as index
    }

    rewind(file);
}

// Reading file into train routes linked list
train_route *read_file_trains(FILE *file, train_route *routes) {

    int i = 0;
    int count = line_count(file);
    int orig = 0;
    int dest = 0;
    int start = 0;
    int end = 0;

     for (i = 0; i < count; i++) {
            fscanf(file, "%d %d %d %d", &orig, &dest, &start, &end);
            routes = add_route(routes, make_route(orig, dest, start, end));
     }

     return routes;
}

// Create new train route node
train_route *make_route(int origin_station, int dest_station, int start_time, int end_time) {

    train_route *new = malloc(sizeof(train_route));

    new->origin_station = origin_station;
    new->dest_station = dest_station;
    new->start_time = convert_to_24(start_time);
    new->end_time = convert_to_24(end_time);
    new->duration = (end_time - start_time);
    new->next = NULL;

    return new;
}

// Add the node to the front of the linked list
train_route *add_route(train_route *routes, train_route *new_route) {

    new_route->next = routes;
    routes = new_route;

    return routes; // Return new head of linked list
}

// Determine if origin station # and destination station # match user input
bool direct_route(train_route *routes, int origin_station, int dest_station) {

    while (routes != NULL) {
        if (routes->origin_station == origin_station && routes->dest_station == dest_station)
            return true;
        else if (routes == NULL)
            return false; // If loop reaches end of linked list without finding a matching route, return false
        else 
            routes = routes->next;
    }
}

// Determine the shortest route duration and return that route
train_route *shortest_time(train_route *routes) {

    train_route *tmp = NULL;
    tmp = routes;
    int shortestTime = 0;
    shortestTime = routes->duration;

    while (routes != NULL) {
        if (routes->duration < shortestTime) {
            shortestTime = routes->duration;
            tmp = routes;
        }
        routes = routes->next;
    }

    return tmp;
}

// Find routes that start on or after the user indicated start time
void list_routes(train_route *routes, char stations[][MAX_STATION_LENGTH], int time) {

    while (routes != NULL) {
        if (routes->start_time >= time) {
            printf("%2d: %-15s@%5d  to%4d: %-15s@%5d\n", routes->origin_station, stations[routes->origin_station], routes->start_time, routes->dest_station, stations[routes->dest_station], routes->end_time);
        }
        routes = routes->next;
    }
}

// Retrieve station number using user input
int find_station_number(int num_stations, char stations[][MAX_STATION_LENGTH], char search_str[MAX_STATION_LENGTH]) {

    int i = 0;
    int stationNum = -1;

    for (i = 0; i < num_stations; i++) {
        if (strcmp(stations[i], search_str) == 0)
            stationNum = i;
    }
    
    return stationNum;
}

// Retrieve station number using user input
void find_station_name(char stations[][MAX_STATION_LENGTH], int station_num, char station_name[MAX_STATION_LENGTH]) {

    strcpy(station_name, stations[station_num]); // Copy station name into stationName variable to "return" the station name

}

int convert_to_24(int minute_time) {

    int time = 0;

    while (minute_time > 0) {
        if (minute_time >= 60) {
            time += 100;
            minute_time -= 60;
        }
        else {
            time += minute_time;
            minute_time -= minute_time;
        }
    }

    return time;
}

int convert_from_24(int time_24) {

    int time = 0;

    while (time_24 > 0) {
        if (time_24 >= 100) {
            time += 60;
            time_24 -= 100;
        }
        else {
            time += time_24;
            time_24 -= time_24;
        }
    }

    return time;
}

int line_count(FILE *file) {

    int counter = 1;
    char c;

    while (c != EOF) {
        c = fgetc(file);
        if (c == '\n')
            counter += 1;
    }

    rewind(file);
    return counter;
}

// Function called at every menu loop to retrieve user option
int getChoice() {

    char inputBuffer[SIZE];
    int choice = 0;

    printf("Please pick an option: ");
    fgets(inputBuffer, SIZE, stdin);
    if (sscanf(inputBuffer, "%d", &choice) != 1) 
        choice = -1;

    return choice;
}

void argCheck(int argc) {

    if (argc != 3) {
        printf("Syntax error: ./<exec> <stationFile> <trainFile>\n");
        exit(1);
    }
}

// Function to return a user specified integer
int getVal() {

    char inputBuffer[SIZE];
    int val = 0;
    int errorCheck = 0;

    while (errorCheck == 0) {
        fgets(inputBuffer, SIZE, stdin);
        if (sscanf(inputBuffer, "%d", &val) != 1) { // Built in error checking
            printf("Invalid entry: please enter an integer.\n");
        }
        else {
            errorCheck = 1;
        }
    }

    return val;
}
