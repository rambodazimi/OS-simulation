void mem_init();
char *mem_get_value(char *var);
void mem_set_value(char *var, char *value);
void mem_set_value_enhanced(char *var_in, char value_in[]);
int touch(char* fileName);
int cd(char* dirName);
int check_mem(char *variableName);
int check_mem2(char *variableName);
int mem_store_line(char *cmd);
void run_single_process(int startPosition, int length);
void clean_single_process(int startPosition, int length);
int mem_store_page(char *ff, int numlines, char *mode);
int store_to_frame_store(char *first, char *second, char *third);
void run_single_command(char *cmd);