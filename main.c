#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define DATA_FILE_IDX 1
#define OPS_FILE_IDX 2

//Enumerations of Subfields within Categories for Accessing Entry categories via corresponding index
const char* EducationFields[] = {"Education.Bachelor's Degree or Higher",
                                "Education.High School or Higher"};
const char* EthnicitiesFields[] = {"Ethnicities.American Indian and Alaska Native Alone",
                                    "Ethnicities.Asian Alone",
                                    "Ethnicities.Black Alone",
                                    "Ethnicities.Hispanic or Latino",
                                    "Ethnicities.Native Hawaiian and Other Pacific Islander Alone",
                                    "Ethnicities.Two or More Races",
                                    "Ethnicities.White Alone",
                                    "Ethnicities.White Alone not Hispanic or Latino"};
const char *IncomeFields[] = {"Income.Median Household Income",
                                "Income.Per Capita Income",
                                "Income.Persons Below Poverty Level"};

const char* SignificantFields[] = {"County", 
                                    "State", 
                                    "Education.Bachelor's Degree or Higher",
                                    "Education.High School or Higher",
                                    "Ethnicities.American Indian and Alaska Native Alone",
                                    "Ethnicities.Asian Alone",
                                    "Ethnicities.Black Alone",
                                    "Ethnicities.Hispanic or Latino",
                                    "Ethnicities.Native Hawaiian and Other Pacific Islander Alone",
                                    "Ethnicities.Two or More Races",
                                    "Ethnicities.White Alone",
                                    "Ethnicities.White Alone not Hispanic or Latino", 
                                    "Income.Median Household Income",
                                    "Income.Per Capita Income",
                                    "Income.Persons Below Poverty Level",
                                    "Population.2014 Population"};

typedef struct{
    char* county;
    char* state;
    float education[2];
    float ethnicities[8];
    int income[2]; 
    float incomePovertyLevel;
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

    new_entry->county = NULL;
    new_entry->state = NULL;
    for(int i=0;i<2;i++){
        new_entry->education[i] = 0.0;
    }
    for(int i=0;i<7;i++){
        new_entry->ethnicities[i] = 0.0;
    }
    for(int i=0;i<2;i++){
        new_entry->income[i] = 0;
    }
    new_entry->incomePovertyLevel = 0.0;
    new_entry->population2014 = 0;

    return new_entry;
}

/*Deletes an entry from EntryArray*/
void remove_entry(EntryArray* array, int index) {

    // Free the memory of the entry to be deleted
    free(array->entries[index]->county);  // Assuming the county field is dynamically allocated
    free(array->entries[index]->state);   // Assuming the state field is dynamically allocated
    free(array->entries[index]);          // Free the entire Entry object

    // Shift the remaining entries in the array to fill the gap
    for (int i = index; i < array->n - 1; i++) {
        array->entries[i] = array->entries[i + 1];
    }

    // Decrease the number of entries
    array->n--;
}

/*Frees all dynamically allocated memory for EntryArray*/
void free_entry_array(EntryArray* array){
    for(int i=0; i<array->n; i++){
        // Free all dynamically allocated memory within each entry, followed by the entry itself
        // printf("Freeing: %s\n", array->entries[i]->county);
        free(array->entries[i]->county);
        free(array->entries[i]->state);
        free(array->entries[i]);
    }
    free(array->entries);
    free(array);
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
    for(int i=0;i<16;i++){
        if(strcmp(field, SignificantFields[i])==0){
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

void append_entry(EntryArray* array, Entry* new_entry){
    // Return if new_entry is NULL
    if(new_entry == NULL){
        return;
    }
    
    // Check size
    if(array->n == array->capacity){
        // Double array_entries capacity on resize
        array->entries = realloc(array->entries, (array->capacity *= 2) * sizeof(Entry*));
        if(array->entries == NULL){
            perror("append_entry realloc");
            exit(1);
        }
    }
    // Append new entry to end of array
    array->entries[array->n++] = new_entry;
}


/*Converts strings to float*/
float str_to_float(char *string){
    char* endptr;
    float result = strtof(string, &endptr);
    if(*endptr!='\0'){
        perror("atof");
        exit(1);
    }
    return result;
}

/*Update entry values*/
void update_entry(Entry* entry, const char* field, char* value){
    if(strncmp(field, "County", strlen("County"))==0){
        // Code for "County" Field
        if(entry->county != NULL){
            free(entry->county);
        }
        entry->county = strdup(value);
    }else if(strncmp(field, "State", strlen("State"))==0){ // Code for "State" Field
        if(entry->state != NULL){
            free(entry->state);
        }
        entry->state = strdup(value);
    }else if(strncmp(field, "Education", strlen("Education"))==0){ // Code for "Education Field"
        // Identify subfield
        int subfield;
        for(subfield=0; subfield<2; subfield++){
            if(strcmp(field, EducationFields[subfield])==0){
                break;
            }
        }
        //Convert value to a float to store in entry
        entry->education[subfield] = str_to_float(value);
    }else if(strncmp(field, "Ethnicities", strlen("Ethnicities"))==0){ // Code for "Ethnicities" field
        // Identify subfield
        int subfield;
        for(subfield=0; subfield<8; subfield++){
            if(strcmp(field, EthnicitiesFields[subfield])==0){
                break;
            }
        }
        //Convert value to a float to store in entry
        entry->ethnicities[subfield] = str_to_float(value);
        // printf("Updated: %s to: %f\n", EthnicitiesFields[subfield], str_to_float(value)); //Debugging print
    }else if(strncmp(field, "Income", strlen("Income"))==0){ // Code for "Income" field
        // Identify subfield
        int subfield;
        for(subfield=0; subfield<3; subfield++){
            if(strcmp(field, IncomeFields[subfield])==0){
                break;
            }
        }
        if(subfield == 2){
            entry->incomePovertyLevel = str_to_float(value);
        }else{
            //Convert value to a float to store in entry
            entry->income[subfield] = atoi(value);
        }

    }else if(strncmp(field, "Population.2014", strlen("Population.2014"))==0){ // Code for "Population.2014" field
        entry->population2014 = atoi(value);
    }else{
        perror("update_entry");
        exit(1);
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
    int relevant_columns[16]; // Array that stores indices representing relevant columns
    int relevant_index = 0; //Pointer used to traverse thru relevant indices

    //Process file line-by-line
    char buffer[2048]; 
    while(fgets(buffer, sizeof(buffer), file) != NULL){
        // Null Terminate Line
        buffer[strcspn(buffer, "\n")] = '\0';

        char *string_copy = strdup(buffer); //Duplicate string using malloc so I can use strsep
        char *free_pointer = string_copy;
        int column = 0; //Represents column via index
        int field_index = 0; //Used to determine which field in SignificantField to currently parse

        // Construct Entry
        Entry *new_entry = NULL;
        if(!is_first_row){
            new_entry = create_entry();
        }

        // Parse Data into Entry
        char *value = NULL;
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
                // Because the order of the columns never change, traversing though field index is safe
                if(column == relevant_columns[field_index]){
                    // Relevant column identified
                    // printf("Field: %s | Column %d:  %s\n", SignificantFields[field_index], column, value);
                    update_entry(new_entry, SignificantFields[field_index], value);
                    // for(int i=0; i<entries->n;i++){
                    //     printf("%f\n", entries->entries[i]->ethnicities[7]);
                    // }
                    field_index++;
                }
            }
            column++; //Increment column counter per token
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

/*Return Significant Field index of valid population fields*/
int valid_pop_field(EntryArray *entries, char *field){
    if(strncmp(field, "Education.", strlen("Education."))==0){
        for(int subfield=0;subfield<2;subfield++){
            if(strcmp(field, EducationFields[subfield])==0){
                return subfield+2;
            }
        }
    }else if(strncmp(field, "Ethnicities.", strlen("Ethnicities."))==0){
        for(int subfield=0;subfield<8;subfield++){
            if(strcmp(field, EthnicitiesFields[subfield])==0){
                return subfield+4;
            }
        }
    }else if(strcmp(field, "Income.Persons Below Poverty Level")==0){
        return 14;
    }

    return -1;
}

/* Population Operation */
void population_op(EntryArray* entries, char* field) {
    int total_mode = 0;  // Determines mode of the operation
    if (field == NULL) {
        // A NULL pointer turns on total_mode
        total_mode = 1;
    }

    if (total_mode) {
        // Total Mode operation
        unsigned long long total = 0;  // Total population count

        for (int i = 0; i < entries->n; i++) {
            total += entries->entries[i]->population2014;
        }

        printf("2014 population: %llu\n", total);
    } else {
        // Field-specific operation
        double total = 0;  // Total population count
        if (strncmp(field, "Education.", strlen("Education.")) == 0) {
            int subfield;
            for (subfield = 0; subfield < 2; subfield++) {
                if (strcmp(field, EducationFields[subfield]) == 0) {
                    break;
                }
            }
            for (int i = 0; i < entries->n; i++) {
                total += round(entries->entries[i]->population2014 * (entries->entries[i]->education[subfield] / 100.0));
            }
        } else if (strncmp(field, "Ethnicities.", strlen("Ethnicities.")) == 0) {
            int subfield;
            for (subfield = 0; subfield < 8; subfield++) {
                if (strcmp(field, EthnicitiesFields[subfield]) == 0) {
                    break;
                }
            }
            for (int i = 0; i < entries->n; i++) {
                total += round(entries->entries[i]->population2014 * (entries->entries[i]->ethnicities[subfield] / 100.0));
            }
        } else if (strcmp(field, "Income.Persons Below Poverty Level") == 0) {
            for (int i = 0; i < entries->n; i++) {
                total += round(entries->entries[i]->population2014 * (entries->entries[i]->incomePovertyLevel / 100.0));
            }
        }
        printf("2014 %s population: %.6f\n", field, total);
    }
}

/*Percent Operation*/
void percent_op(EntryArray* entries, char* field){
    // Total Mode operation
    unsigned long long population_total = 0;  // Total population count

    for (int i = 0; i < entries->n; i++) {
        population_total += entries->entries[i]->population2014;
    }

    // Field-specific operation
    double result = 0;  // Total population count
    double sub_population;
    if (strncmp(field, "Education.", strlen("Education.")) == 0) {
        int subfield;
        for (subfield = 0; subfield < 2; subfield++) {
            if (strcmp(field, EducationFields[subfield]) == 0) {
                break;
            }
        }
        for (int i = 0; i < entries->n; i++) {
            sub_population = round(entries->entries[i]->population2014 * (entries->entries[i]->education[subfield] / 100.0));
            result += (sub_population/population_total)*100;
        }
    } else if (strncmp(field, "Ethnicities.", strlen("Ethnicities.")) == 0) {
        int subfield;
        for (subfield = 0; subfield < 8; subfield++) {
            if (strcmp(field, EthnicitiesFields[subfield]) == 0) {
                break;
            }
        }
        for (int i = 0; i < entries->n; i++) {
            sub_population = round(entries->entries[i]->population2014 * (entries->entries[i]->ethnicities[subfield] / 100.0));
            result += (sub_population/population_total)*100;
        }
    } else if (strcmp(field, "Income.Persons Below Poverty Level") == 0) {
        for (int i = 0; i < entries->n; i++) {
            sub_population = round(entries->entries[i]->population2014 * (entries->entries[i]->incomePovertyLevel / 100.0));
            result += (sub_population/population_total)*100;
        }
    }
    printf("2014 %s population: %.6f\n", field, result);
}

/*Filter-state Operation*/
void filter_state_op(EntryArray *entries, char* state){
    for(int i=0;i<entries->n;i++){
        if(strcmp(entries->entries[i]->state, state)!=0){
            // Remove entry if it does not match state
            remove_entry(entries, i);
            i--;
        }
    }
    printf("Filter: state == %s (%d entries)\n", state, entries->n);
}

/*Check inequality between value and target
Returns 1 if value is ge/le than target; Returns 0 if false */
int match_inequality(float value, float target, char* inequality_str){
    if(strcmp(inequality_str, "ge")==0){
        return value-target >= 0;
    }
    else if(strcmp(inequality_str, "le")==0){
        return value-target <= 0;
    }
    else{
        return -1;
    }
}

void filter_op(EntryArray *entries, char *field, char *inequality, char *number_str){    
    // Parse Arguments
    float number = str_to_float(number_str);

    // Branch for every field:
    if(strncmp(field, "Education", strlen("Education"))==0){ // Code for "Education Field"
        // Identify subfield
        int subfield;
        for(subfield=0; subfield<2; subfield++){
            if(strcmp(field, EducationFields[subfield])==0){
                break;
            }
        }
        for(int i=0; i<entries->n;i++){
            if(!match_inequality(entries->entries[i]->education[subfield], number, inequality)){
                //Remove entry if they do not match the inequality
                remove_entry(entries, i);
                i--;
            }
        }
    }else if(strncmp(field, "Ethnicities", strlen("Ethnicities"))==0){ // Code for "Ethnicities" field
        // Identify subfield
        int subfield;
        for(subfield=0; subfield<8; subfield++){
            if(strcmp(field, EthnicitiesFields[subfield])==0){
                break;
            }
        }
        
        for(int i=0; i<entries->n;i++){
            if(!match_inequality(entries->entries[i]->ethnicities[subfield], number, inequality)){
                //Remove entry if they do not match the inequality
                remove_entry(entries, i);
                i--;
            }
        }
        
    }else if(strncmp(field, "Income", strlen("Income"))==0){ // Code for "Income" field
        // Identify subfield
        int subfield;
        for(subfield=0; subfield<2; subfield++){
            if(strcmp(field, IncomeFields[subfield])==0){
                break;
            }
        }
        if(subfield==2){
            for(int i=0; i<entries->n;i++){
                if(!match_inequality(entries->entries[i]->incomePovertyLevel, number, inequality)){
                    //Remove entry if they do not match the inequality
                    remove_entry(entries, i);
                    i--;
                }
            }
        }else{
            for(int i=0; i<entries->n;i++){
                if(!match_inequality(entries->entries[i]->income[subfield], number, inequality)){
                    //Remove entry if they do not match the inequality
                    remove_entry(entries, i);
                    i--;
                }
            }
        }

        
    }else if(strncmp(field, "Population.2014", strlen("Population.2014"))==0){ // Code for "Population.2014" field
        for(int i=0; i<entries->n;i++){
            if(!match_inequality(entries->entries[i]->population2014, number, inequality)){
                //Remove entry if they do not match the inequality
                remove_entry(entries, i);
                i--;
            }
        }
    }else{
        perror("filter-state");
        exit(1);
    }

    printf("Filter: %s %s %.6f (%d entries)\n", field, inequality, number, entries->n);
}

void display_op(EntryArray* entries){
    for(int i=0; i<entries->n; i++){
        printf("%s, %s\n", entries->entries[i]->county, entries->entries[i]->state); //Print County & State
        printf("\tPopulation: %d\n", entries->entries[i]->population2014); //Print Population
        printf("\tEducation\n");
        printf("\t\t>= High School: %.6f%%\n", entries->entries[i]->education[1]); //Print Education Highschool+
        printf("\t\t>= Bachelor's: %.6f%%\n", entries->entries[i]->education[0]); //Print Education Highschool+
        printf("\tEthnicity Percentages\n");
        printf("\t\tAmerican Indian and Alaska Native: %.6f%%\n", entries->entries[i]->ethnicities[0]);
        printf("\t\tAsian Alone: %.6f%%\n", entries->entries[i]->ethnicities[1]);
        printf("\t\tBlack Alone: %.6f%%\n", entries->entries[i]->ethnicities[2]);
        printf("\t\tHispanic or Latino: %.6f%%\n", entries->entries[i]->ethnicities[3]);
        printf("\t\tNative Hawaiian and Other Pacific Islander Alone: %.6f%%\n", entries->entries[i]->ethnicities[4]);
        printf("\t\tTwo or More Races: %.6f%%\n", entries->entries[i]->ethnicities[5]);
        printf("\t\tWhite Alone: %.6f%%\n", entries->entries[i]->ethnicities[6]);
        printf("\t\tWhite Alone, not Hispanic or Latino: %.6f%%\n", entries->entries[i]->ethnicities[7]);
        printf("\tIncome\n");
        printf("\t\tMedian Household: %d\n", entries->entries[i]->income[0]);
        printf("\t\tPer Capita: %d\n", entries->entries[i]->income[1]);
        printf("\t\tBelow Poverty Level: %.6f%%\n\n", entries->entries[i]->incomePovertyLevel);
    }
}

/*Parse operations from file_name to perform on entries*/
void process_operations(char *file_name, EntryArray* entries){
    //Open file
    FILE* file = fopen(file_name, "r");
    if(file == NULL){
        perror("operations fopen");
        exit(1);
    }

    //Process file line-by-line
    char line[2048]; 
    while(fgets(line, sizeof(line), file) != NULL){
        // Null Terminate Line
        line[strcspn(line, "\n")] = '\0';

        char *string_copy = strdup(line);
        char *original_ptr = string_copy;

        int argument_index = 0;
        char* arguments[4] = {NULL}; //1: Operation, 2: 
        char *token; //Stores tokens from strsep
        while((token = strsep(&string_copy, ":"))!=NULL && argument_index < 4){
            arguments[argument_index] = token;
            argument_index++;
        }

        if (strcmp(line, "display") == 0) { // Display branch
            display_op(entries);

        } else if (strncmp(line, "filter-state:", strlen("filter-state:")) == 0) { // Filter-state branch
            filter_state_op(entries, arguments[1]);

        } else if (strncmp(line, "filter:", strlen("filter:")) == 0) { // Filter branch
            filter_op(entries, arguments[1], arguments[2], arguments[3]);

        } else if (strcmp(line, "population-total") == 0) { // Population total branch
            population_op(entries, NULL);
            
        } else if (strncmp(line, "population:", strlen("population:")) == 0) {
            population_op(entries, arguments[1]);

        } else if (strncmp(line, "percent:", strlen("percent:")) == 0) {
            percent_op(entries, arguments[1]);
            
        } else {
            printf("Error: Unsupported line '%s'.\n", line);
        }

        free(original_ptr);
    }
    fclose(file);
    
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
    printf("%d entries loaded\n", entries_array->n);

    //Process operations file
    process_operations(argv[OPS_FILE_IDX], entries_array);

    // Free All EntryArray Memory
    free_entry_array(entries_array);

    return 0;
}