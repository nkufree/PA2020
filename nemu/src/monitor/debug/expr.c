#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <memory/paddr.h>
enum {
  TK_NOTYPE = 256, TK_EQ,

  /* TODO: Add more token types */
	TK_AND, TK_NEQ, TK_G, TK_L, 
  TK_HEXNUM, TK_NUM, TK_REG, 
	TK_MINUS, // UNARY -
	TK_DERE, // UNARY *
};

bool expr_valid = true;

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {"0x[0-9A-Fa-f]+", TK_HEXNUM},
  {"[1-9][0-9]+|[0-9]", TK_NUM},
  {"\\$[0-9a-zA-Z]+", TK_REG},
  {"\\+", '+'},         // plus
  {"\\(", '('},
  {"\\)", ')'},
  {"\\*", '*'},
  {"/", '/'},
  {" +", TK_NOTYPE},    // spaces
  {"-", '-'},
  {"==", TK_EQ},        // equal
	{"&&", TK_AND},
	{"!=", TK_NEQ},
  {">", TK_G},
  {"<", TK_L},
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX] = {};

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
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

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

        // Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
        //     i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        if(rules[i].token_type == TK_NOTYPE)
					break;
        if(substr_len > 31)
        {
          Log("token len: %d is too long", substr_len);
          return false;
        }
        tokens[nr_token].type = rules[i].token_type;
        switch (rules[i].token_type) {
          case TK_HEXNUM:
          case TK_NUM:
          case TK_REG:
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            break;
          default: 
            break;
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

bool check_parentheses(int p, int q) {
  int size = 0;
  if(tokens[p].type != '(' || tokens[q].type != ')')
    return false;
  for(int i = p; i < q; i++)
  {
    if(tokens[i].type == '(') {
      size++;
    }
    else if(tokens[i].type == ')') {
      size--;
      if(size < 0)
      {
        Log("error expr");
        expr_valid = false;
        return false;
      }
      else if(size == 0)
        return false;
    }
  }
  if(size != 1)
  {
    Log("error expr");
    expr_valid = false;
    return false;
  }
  return true;
}

int get_main_op_pos(int p, int q) {
  int pos = -1;
  int op_priority = -1;
  bool in_parentheses = false;
  for(int i = p; i < q + 1; i++)
  {
    switch(tokens[i].type)
    {
      case '(':
        in_parentheses = true;
        break;
      case ')':
        in_parentheses = false;
        break;
      case '+':
      case '-':
        if(in_parentheses)
          break;
				if(in_parentheses || op_priority > 2)
          break;
        pos = i;
        op_priority = 2;
        break;
      case '*':
      case '/':
        if(in_parentheses || op_priority > 1)
          break;
        pos = i;
        op_priority = 1;
        break;
			case TK_MINUS:
			case TK_DERE:
        // 单目运算符取第一个进行划分
				if(in_parentheses || op_priority >= 0)
          break;
				pos = i;
        op_priority = 0;
        break;
			case TK_EQ:
			case TK_AND:
			case TK_NEQ:
      case TK_G:
      case TK_L:
				if(in_parentheses || op_priority > 3)
          break;
				pos = i;
        op_priority = 3;
        break;
      default:
        break;
    }
  }
  return pos;
}

word_t eval(int p, int q) {
  if(!expr_valid)
    return 0;
  if (p > q) {
    /* Bad expression */
    Log("error eval");
    expr_valid = false;
    return 0;
  }
  else if (p == q) {
    /* Single token.
     * For now this token should be a number.
     * Return the value of the number.
     */
    Assert(tokens[p].type == TK_HEXNUM || tokens[p].type == TK_NUM || tokens[p].type == TK_REG, 
    "error type of token %d, type is %d", p, tokens[p].type);
    word_t num;
    switch(tokens[p].type) {
      case TK_HEXNUM:
        sscanf(tokens[p].str, "0x%x", &num);
        break;
      case TK_NUM:
        sscanf(tokens[p].str, "%d", &num);
        break;
      case TK_REG:
        {
          bool success;
          num = isa_reg_str2val(tokens[p].str, &success);
          if(!success)
          {
            Log("error reg");
            return 0;
          }
        }
        break;
      default:
        break;
    }
    return num;
  }
  else if (check_parentheses(p, q) == true) {
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     */
    return eval(p + 1, q - 1);
  }
  else {
    /* We should do more things here. */
    if(!expr_valid)
      return 0;
    int op = get_main_op_pos(p, q);
		word_t val1;
		if(tokens[op].type == TK_MINUS || tokens[op].type == TK_DERE)
			val1 = 0;
		else
    	val1 = eval(p, op - 1);
    word_t val2 = eval(op + 1, q);
    switch (tokens[op].type)
    {
    case '+':
      return val1 + val2;
    case '-':
      return val1 - val2;
    case '*':
      return val1 * val2;
    case '/':
      return val1 / val2;
		case TK_MINUS:
			return -val2;
		case TK_DERE:
			return paddr_read(val2, 4);
		case TK_EQ:
			return val1 == val2;
		case TK_NEQ:
			return val1 != val2;
		case TK_AND:
			return val1 && val2;
    case TK_G:
      return val1 > val2;
    case TK_L:
      return val1 < val2;
    default:
      Log("error expr");
      expr_valid = false;
      return 0;
    }
  }
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  for(int i = 0; i < nr_token; i++)
	{
		int prev = i == 0 ? TK_NOTYPE : tokens[i - 1].type;
		if(tokens[i].type == '-' || tokens[i].type == '*')
		{
			if(i == 0 || prev == '+' || prev == '-' 
			|| prev == '*' || prev == '/' 
			|| prev == '(' || prev == TK_MINUS || prev == TK_DERE)
			{
				if(tokens[i].type == '-')
					tokens[i].type = TK_MINUS;
				else if(tokens[i].type == '*')
					tokens[i].type = TK_DERE;
			}
		}
	}

  word_t res = eval(0, nr_token - 1);
  if(!expr_valid)
    *success = false;
  *success = true;

  return res;
}
