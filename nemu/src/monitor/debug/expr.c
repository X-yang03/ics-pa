#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include<stdlib.h>

enum {
  TK_NOTYPE = 256, TK_EQ,TK_NEQ, TK_DEC,TK_HEX,TK_REG

  /* TODO: Add more token types */

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ},         // equal
  {"!=",TK_NEQ},
  {"[1-9][0-9]*|0",TK_DEC}, //decimial
  {"0[xX][0-9a-fA-F]+",TK_HEX},
  {"\\-",'-'},
  {"\\*",'*'},
  {"\\/",'/'},
  {"\\(",'('},
  {"\\)",')'},
  {"\\$e[abcd]x",TK_REG},
  {"\\$e[bs]p",TK_REG},
  {"\\$e[sd]i",TK_REG},
  {"\\$eip",TK_REG}

};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  int priority; // used to find dominant op
  char str[32];
} Token;

Token tokens[32]; // at most 32 tokens in a sentence
int nr_token;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
        tokens[nr_token].type = rules[i].token_type;
        tokens[nr_token].priority = -1;

        switch (rules[i].token_type) {
          case '+': case '-':  
            tokens[nr_token].priority = 1;
            break;

          case '*': case '/':
              tokens[nr_token].priority = 2;
            break;

          case TK_DEC: case TK_HEX: case TK_REG:
            Assert(substr_len<32,"Length of numbers should be no more than 31!\n");
            //KISS protocol
            strncpy(substr_start,tokens[nr_token].str,substr_len);  //copy the string
            break;

          default: break;
        }

        nr_token++;
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

bool check_parentheses(int p,int q){
  int par = 0;
  if(tokens[p].type == '(' && tokens[q].type == ')'){
      for(int i = p + 1;i < q;i++){
        if(tokens[i].type == '(') par++;
        else if(tokens[i].type == ')') par--;
    }
    if(par == 0) return true;
  }
  return false;
}

bool check_priority(int p, int q){
    if(tokens[p].priority != -1 && tokens[q].priority != -1){
      return tokens[p].priority - tokens[q].priority;
    }
    return false;
}

int dominant_op(int p , int q){ // find dominant operator between p and q
  int in_parentheses = 0;
  int min_priority = 10;
  int _dominant = 0;
  for(int i = p; i <= q; i++){
    if(tokens[i].type == '(') in_parentheses++;
    else if (tokens[i].type == ')') in_parentheses--;

    if(in_parentheses == 0 && tokens[i].priority != -1){ // with no parentheses
        if(tokens[i].priority < min_priority){
          min_priority = tokens[i].priority;
          _dominant = i;
        }
        else if(tokens[i].priority == min_priority){
          _dominant = i;
        }

    }
  }
  return _dominant;

}

uint32_t eval(int p , int q) // tokens[p] and tokens[q] NOT CHARACTER[p] AND CHARACTER[q]!
{
  Assert(p<=q,"Bad Expression!");
  if(p == q){ // a single token
    int val = 0;
    switch (tokens[p].type)
    {
    case TK_DEC:
      return atoi(tokens[p].str);
      break;
    
    case TK_HEX:
      
      sscanf(tokens[p].str,"%x",&val);
      return val;

    case TK_REG:
      if (strcmp(tokens[p].str , "$eax") == 0) return cpu.eax;
      else if (strcmp(tokens[p].str , "$eax") == 0) return cpu.eax;
      else if (strcmp(tokens[p].str , "$ebx") == 0) return cpu.ebx;
      else if (strcmp(tokens[p].str , "$ecx") == 0) return cpu.ecx;
      else if (strcmp(tokens[p].str , "$edx") == 0) return cpu.edx;
      else if (strcmp(tokens[p].str , "$ebp") == 0) return cpu.ebp;
      else if (strcmp(tokens[p].str , "$esp") == 0) return cpu.esp;
      else if (strcmp(tokens[p].str , "$esi") == 0) return cpu.esi;
      else if (strcmp(tokens[p].str , "$edi") == 0) return cpu.edi;
      else if (strcmp(tokens[p].str , "$eip") == 0) return cpu.eip;
    default:
      break;
    }

  }
  else if(check_parentheses(p,q)){
    eval(p+1,q-1);
  }
  else{
    int op = dominant_op(p,q);
    int val1 = eval(p,op-1);
    int val2 = eval(op+1,q);
    switch (tokens[op].type)
    {
    case '+':
      return val1 + val2;
      break;
    case '-':
      return val1 - val2;
      break;
    case '*':
      return val1 * val2;
      break;
    case '/':
      return val1 / val2;
      break;
    default:
      Assert(0,"Bad operation!\n");
      break;
    }
  }

  return 0;
}

uint32_t expr(char *e, bool *success) {

  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  int par = 0;
  for(int i =0; i < nr_token; i++){
    if(tokens[i].type == '(') par++;
    else if(tokens[i].type == ')') par--;
  }
  Assert(par == 0 , "Unmatched Parentheses!\n");

  return eval(0,nr_token-1);
  /* TODO: Insert codes to evaluate the expression. */

}