#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA_FILE_IDX 1
#define OPS_FILE_IDX 2

//Enumerations of Subfields within Categories for Accessing Entry categories via corresponding index
const char* EducationFields[] = {"Bachelor's Degree or Higher", 
                                    "High School or Higher"};
const char* EthnicitiesFields[] = {"Ethnicities.American Indian and Alaska Native Alone",
                                    "Ethnicities.Asian Alone",
                                    "Ethnicities.Black Alone",
                                    "Ethnicities.Hispanic or Latino",
                                    "Ethnicities.Native Hawaiian and Other Pacific Islander Alone",
                                    "Ethnicities.Two or More Races","Ethnicities.White Alone",
                                    "Ethnicities.White Alone not Hispanic or Latino"};
const char *IncomeFields[] = {"Income.Median Household Income",
                                "Income.Per Capita Income",
                                "Income.Persons Below Poverty Level"};

const char* SignificantFields[] = {"County", 
                                    "State", 
                                    "Education", 
                                    "Ethnicities", 
                                    "Income", 
                                    "Population.2014 Population"};

typedef struct{
    char* county;
    char* state;
    float education[2];
    float ethnicities[7];
    int income[3]; 
    int population2014;

}Entry;

typedef struct{
    Entry** entries; // Array of Entry pointers
    int n; // Stores number of entries
    int capacity; // Stores current capacity of array
}EntryArray;

/*Constructs new entry dynamically*/
Entry *create_entry(){
    Entry *new_entry = malloc(sizeof(Entry));
    if(new_entry == NULL){
        perror("create_entry malloc");
        exit(1);
    }

    new_entry->county = strdup("Placeholder");
    new_entry->state = strdup("Placeholder");
    for(int i=0;i<2;i++){
        new_entry->education[i] = 0.0;
    }
    for(int i=0;i<7;i++){
        new_entry->ethnicities[i] = 0.0;
    }
    for(int i=0;i<3;i++){
        new_entry->income[i] = 0;
    }
    new_entry->population2014 = 0;

    return new_entry;
}

/*Frees all dynamically allocated memory for EntryArray*/
void free_entry_array(EntryArray* array){

}

/*Validates argument files existence*/
int validate_args(int argc, char* argv[]){
    if(argc < 3){
        fprintf(stderr, "Usage: ./main <data_file> <operations_file>\n");
        return 1;
    }else{
        //Check for file existence
        if(access(argv[DATA_FILE_IDX], F_OK)){
            fprintf(stderr, "ERR: Demographics Data File Not Found\n");
            return 1;
        }

        if(access(argv[OPS_FILE_IDX], F_OK)){
            fprintf(stderr, "ERR: Operations File Not Found\n");
            return 1;
        }
    }
    return 0;
}

/*Check if given field is relevant to this assignment
Returns 1 if so; 0 otherwise*/
int relevant_field(char* field){
    for(int i=0;i<6;i++){
        if(strncmp(field, SignificantFields[i], strlen(SignificantFields[i]))==0){
            return 1;
        }
    }
    return 0;
}

/*Removes quotes from beginning and end of string
Implemented in this way to handle pointer correctly while keeping strsep compatibility*/
char *dequote(char *string){
    size_t length = strlen(string);
    if(length > 0 && string[0]=='"'){
        // Shift string left one bit
        for(int i=0; i<length; i++){
            string[i] = string[i+1];
        }
        length--; //Decrease length of string
    }
    if(length >0 && string[length-1]=='"'){
        string[length-1] = '\0';
    }
    return string;
}

/*Add a new entry to EntryArray*/
void append_entry(EntryArray* array, Entry* new_entry){

    //Return if new_entry is NULL
    if(new_entry == NULL){
        return;
    }
    
    //Check size
    if(array->n == array->capacity){
        // Double array_entries capacity on resize
        array->entries = realloc(array->entries, (array->capacity*=2));
        if(array->entries==NULL){
            perror("append_entry realloc");
            exit(1);
        }
    }
    // Append new entry to end of array
    array->entries[array->n++] = new_entry;
}

/*Update entry values*/
void update_entry(Entry* entry, char* field, char* value){

}

/*Parse data from data_file_name and store into entries*/
EntryArray* process_demographics(char *file_name, EntryArray* entries){
    //Open file
    FILE *file;
    if((file=fopen(file_name, "r"))==NULL){
        perror("demographics fopen");
        exit(1);
    }
    int is_first_row = 1; // Tracks whether or not we're on the first line of csv (Column Headers)
    int relevant_columns[15]; // Array that stores indices representing relevant columns
    int relevant_index = 0; //Pointer used to traverse thru relevant indices

    //Process file line-by-line
    char buffer[2048]; 
    while(fgets(buffer, sizeof(buffer), file) != NULL){
        // Null Terminate Line
        buffer[strcspn(buffer, "\n")] = '\0';

        char *string_copy = strdup(buffer); //Duplicate string using malloc so I can use strsep
        char *free_pointer = string_copy;
        int column = 0; //Represents column via index
        int parsing_index = 0; //Used to determine which field we are currently parsing

        // Construct Entry
        Entry *new_entry;
        if(!is_first_row){
            new_entry = create_entry();
        }

        // Parse Data into Entry
        char *value;
        while((value = strsep(&string_copy,",")) != NULL){ //Read through columns using strsep
            // Clean quotes off value
            value = dequote(value);

            // Check if value is something we're interested in
            if(is_first_row){
                // Uses the Header row to identify which columns are important

                // Fill relevant_columns with indices of relevant data fields
                if(relevant_field(value)){
                    relevant_columns[relevant_index++] = column;
                }

            }else{
                // Because the order of the columns never change, traversing though parsing index is safe
                if(column == relevant_columns[parsing_index]){
                    // Relevant column identified
                    printf("Row %d, Column %d : %s\n", parsing_index, column, value);
                    parsing_index++;

                    // TODO: Parsing Logic
                    // update_entry(new_entry, SignificantFields[parsing_index], value);
                }
            }
            column++; //Increment counter per token
        }
        //Free malloc'ed string
        free(free_pointer);

        is_first_row = 0; //Set to false after first iteration

        // Append new entry
        append_entry(entries, new_entry);
    }
    // Close file
    fclose(file);
    return entries;
}

/*Parse operations from file_name to perform on entries*/
void process_operations(char *file_name, EntryArray* entries){
    return;
}

int main(int argc, char* argv[]){
    
    if(validate_args(argc, argv)){
        return 1;
    }

    EntryArray *entries_array = malloc(sizeof(EntryArray)); //Stores entries post-demographics processing for use in operations processing
    entries_array->n = 0;
    entries_array->capacity = 16;
    entries_array->entries = malloc(sizeof(Entry*)*entries_array->capacity);
    if(entries_array->entries == NULL){
        perror("entries_array malloc");
        exit(1);
    }

    //Process demographics file
    process_demographics(argv[DATA_FILE_IDX], entries_array);

    //Process operations file
    process_operations(argv[OPS_FILE_IDX], entries_array);

    // Free All EntryArray Memory

    return 0;
}