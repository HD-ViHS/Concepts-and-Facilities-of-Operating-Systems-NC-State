// Maximum number of accounts
#define ALIST_MAX 10

// Maximum length of the name of an account.
#define ANAME_MAX 12

// Minimum number of Accounts
#define ALIST_MIN 1

// ASCII value of '0'
#define ASCII_ZERO 48

// ASCII value of '9'
#define ASCII_NINE 57

// Largest value an account can hold as an int number of cents
#define MONETARY_LIMIT 1000000000

#ifndef STRUCTGUARD
#define STRUCTGUARD

// Shared structure between reset.c and account.c for holding account data
struct AccountList {
  int size;
  int balances[ALIST_MAX];
  char names[ALIST_MAX][ANAME_MAX];
};

#endif
