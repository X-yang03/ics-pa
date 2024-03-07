#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"
//test
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);
void init_regex();
uint32_t expr(char *e, bool *success);
WP* new_wp();
void free_wp(WP* wp);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) "); // start with (nemu)

  if (line_read && *line_read) {
    add_history(line_read); // cmd history
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args){
	char *si_num = strtok(NULL," ");
	if(si_num == NULL){
	printf("Illegal number of parameters.\n");
	printf("Check help si to see the Usage.\n");
	return 0;
	}
	int num = atoi(si_num);
	cpu_exec(num);
	printf("Successfully run %d instructions!\n",num);
	return 0;
	
}


static int cmd_info(char* args){
	char *arg = strtok(NULL," ");
	
	if(arg == NULL){
	printf("Illegal number of parameters.\n");
	printf("Check help info to see the Usage.\n");
	return 0;
	}
	if(strcmp(arg,"r") == 0){
	printf("eax : %x\n" , cpu.eax);	
	printf("ecx : %x\n" , cpu.ecx);
	printf("edx : %x\n" , cpu.edx);
	printf("ebx : %x\n" , cpu.ebx);
	printf("esp : %x\n" , cpu.esp);
	printf("ebp : %x\n" , cpu.ebp);
	printf("esi : %x\n" , cpu.esi);
	printf("edi : %x\n" , cpu.edi);
	printf("===================\n");

	}
	else if(strcmp(arg,"w") == 0){
	//Todo: print watchpoint
	}
	else{
	printf("Illegal parameters.\n");
	return 0;
	}
	return 0;
}



static int cmd_x(char *args){
	char *arg = strtok(NULL," ");
	if(arg == NULL){
	printf("Illegal parameters.\n");
	return 0;
	}


	int N = atoi(arg);  //string to int

	arg = strtok(NULL," ");
	if(arg == NULL){
	printf("Illegal Parameters.\n");
	return 0;
	}

  bool succ = true;
  vaddr_t addr = expr(arg,&succ);
  if(!succ)
  {
    printf("Invalid Expression!\n");
    return 1;
  }

	//vaddr_t addr = atoi(arg); //vaddr_t is actually uint32_t
	for (int i=0;i<N;i++){
		uint32_t data = vaddr_read(addr+4*i,4);
		printf("0x%08x :\t",addr+4*i);
		for(int j=0;j<4;j++){
			printf("%02x ",data&0xff);
			data = data >> 8 ;
		}
		printf("\n");
	
	}
	return 0;
}


static int cmd_p(char *args){
  if(args == NULL){
    printf("Empty Expression!\n");
    return 1;
  }
  init_regex();
  bool succ = true;
  int res = expr(args,&succ);
  if(succ){
    printf("Result: %d\n",res);
  }
  else
  {
    printf("Invalid Expression!\n");

  }
  
  return 0 ;
}

static int cmd_w(char* args){
  if(args == NULL){
    printf("Empty Expression!\n");
    return 1;
  }
  bool succ = true;
  int res = expr(args,&succ);
  if(!succ)
  {
    printf("Invalid Expression!\n");
    return 1;
  }
  WP* wp = new_wp();
  strcpy(wp->expr,args);
  wp->val = res;
  
  return 0;
}

static struct { // a func table [name,dis,handler]
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Use si N to run N instructions", cmd_si},
  { "info", "info r to show the status of regfile; info w to show the status of watchpoints" , cmd_info},
  { "x" ,"Usage: x N EXPR to see the contents of RAM from EXPR" , cmd_x},
  {"p","Calculate the value of a expression",cmd_p},
  {"w","Usage: w expr -- set a watch point over a expression",cmd_w},
  /* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
