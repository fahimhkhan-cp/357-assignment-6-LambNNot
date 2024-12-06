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
    Entry* entries; //Array of Entry pointers
    int size;
}EntryArray;

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

/*Removes quotes from beginning and end of string*/
char *dequote(char *string){
    size_t length = strlen(string);
    if(length > 0 && string[0]=='"'){
        string++; //Shift the string to the left
        length--; //Decrease length of string
    }
    if(length >0 && string[length-1]=='"'){
        string[length-1] = '\0';
    }
    return string;
}

void updateEntry(int field_index, char* value){
    if(field_index==0){

    }
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
        //Null Terminate Line
        buffer[strcspn(buffer, "\n")] = '\0';

        char *string_copy = strdup(buffer); //Duplicate string using malloc so I can use strsep
        char *free_pointer = string_copy;
        int column = 0; //Represents column via index
        int parsing_index = 0; //Used to determine which field we are currently parsing

        // Process fields by delimiter
        char *value;
        while((value = strsep(&string_copy,",")) != NULL){
            // Clean quotes off value
            value = dequote(value);

            // Check if value is something we're interested in
            if(is_first_row){
                // Fill relevant_columns with indices of relevant data fields
                if(relevant_field(value)){
                    relevant_columns[relevant_index++] = column;
                }

            }else{
                // Check if value is important

                // Because the order of the columns never change, traversing though parsing index is safe
                if(column == relevant_columns[parsing_index]){
                    //Relevant column identified
                    printf("Row %d, Column %d : %s\n", parsing_index, column, value);
                    parsing_index++;

                    // TODO: Parsing Logic
                    

                }
                
            }
            column++; //Increment counter per token
        }
        is_first_row = 0; //Set to false after first iteration

        //Free malloc'ed string
        free(free_pointer);
    }

    //Close file
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
    entries_array->entries = NULL;
    entries_array->size = 0;

    //Process demographics file
    process_demographics(argv[DATA_FILE_IDX], entries_array);

    //Process operations file
    process_operations(argv[OPS_FILE_IDX], entries_array);

    // Free EntryArray & associated allocated memory
    free(entries_array->entries); //Every entry is statically sized, so there is no need to free them individually
    free(entries_array);

    return 0;
}