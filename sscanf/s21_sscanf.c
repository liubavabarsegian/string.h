#include "s21_sscanf.h"

int s21_sscanf(const char *str, const char *format, ...) {
  const char *types = "cdieEfgGosuxXpn%%";
  va_list ap;
  va_start(ap, format);
  struct specifier spec;
  char *string = malloc((strlen(str) + 1) * sizeof(char));
  strcpy(string, str);
  int count_successes = 0;
  if (string[0] == '\0' && format[0] != '\0')
    count_successes = -1;
  int j = 0;
  char c = '\0';

  if (string[0] != '\0') {
    for (size_t i = 0; i < strlen(format) && j < strlen(string); i++) {
      specifier_init(&spec);
      if (format[i] != '%' && (format[i] == string[j])) {
      }
      else if (format[i] != '%' && (format[i] != string[j]) && !s21_isspace(string[j])) {
        break;
      }
      else if (format[i] == '%' && string[j]) {
        specifier_parsing((char *)&format[i + 1], &spec);
        i += strcspn(&format[i + 1], types) + 1;
        while (s21_isspace(string[j]) && spec.type != 'c') {
          j++;
        }
        if (format[i]) {
          c = format[i + 1];
        }
        count_successes += match_str_and_format(string, &spec, &ap, &j, c);
        j--;
      }
      j++;
    }
  }
  if (string)
    free(string);
  return count_successes;
}


void specifier_parsing(char *str, struct specifier *spec) {
  char *buff = str;
  char *buff1 = malloc(1024);
  int k = 0;
  const char *flags = "-+ #0";
  const char *numbers = "1234567890*";
  const char *length = "hlL";
  const char *types = "cdieEfgGosuxXpn%%";
  size_t spec_length = strcspn(str, types);
  spec->type = str[spec_length];
  pointer_shift(&spec_length, buff, buff1, flags);
  for (int i = 0; i < 5; i++) {
    if (strchr(buff1, flags[i]) != NULL &&
        ((flags[i] == ' ' && strchr(buff1, '+') == NULL) ||
         (flags[i] == '0' && strchr(buff1, '-') == NULL)))
      spec->flag[k++] = flags[i];
  }
  spec->flag[k] = '\0';
  k = 0;
  pointer_shift(&spec_length, buff, buff1, numbers);
  numbers_parsing(spec->width, buff1);
  if (*(buff++) == '.') {
    pointer_shift(&spec_length, buff, buff1, numbers);
    numbers_parsing(spec->precision, buff1);
  }
  memcpy(buff1, buff, strspn((const char *)buff, length));
  for (int i = 0; i < 3; i++) {
    char *c = strchr(buff1, length[i]);
    if (c != NULL) {
      if (length[i] == 'L' || (length[i] == 'h' && c < strchr(buff1, 'l')) ||
          (length[i] == 'l' && strchr(c, length[i]) == NULL &&
           spec->length[0] != 'h')) {
        spec->length[k++] = length[i];
      } else if (length[i] == 'l' && strchr(c, length[i]) != NULL &&
                 spec->length[0] != 'h') {
        spec->length[k++] = length[i];
        spec->length[k++] = length[i];
      }
    }
  }
  spec->length[k] = '\0';
  free(buff1);
}

void numbers_parsing(char *str, char *buff) {
  size_t length = strspn((const char *)buff, (const char *)"1234567890");
  if (*buff == '*' && length == 0) length = 1;
  memcpy(str, buff, length);
}

void pointer_shift(size_t *length, char *buff, char *buff1, const char *str) {
  *length = strspn((const char *)buff, str);
  memcpy(buff1, buff, *length);
  buff += *length;
}

void vararg_init(char type, va_list *ap) {
  if (type == 'c') {
    va_arg(*ap, int);
  } else if (type == 'd' || type == 'i') {
    va_arg(*ap, int);
  } else if (type == 'e' || type == 'E' || type == 'f' || type == 'g' ||
             type == 'G') {
    va_arg(*ap, double);
  } else if (type == 'o' || type == 'u' || type == 'x' || type == 'X') {
    va_arg(*ap, unsigned int);
  } else if (type == 's') {
    va_arg(*ap, char *);
  } else if (type == 'p') {
    va_arg(*ap, void *);
  } else if (type == 'n') {
    va_arg(*ap, int *);
  }
}

void specifier_init(struct specifier *spec) {
  spec->flag[0] = '\0';
  spec->width[0] = '\0';
  spec->precision[0] = '\0';
  spec->length[0] = '\0';
  spec->type = 0;
}

int read_d(char *str, va_list *ap, struct specifier *spec, int *j, char c)
{
  int success = 0;
  int i = 0;
  while (str[*j + i] != '\0' && !s21_isspace(str[*j + i]) && str[*j + i] != c && str[*j + i] != '%') {
    i++;
  }
  if (spec->width[0] != '*') {
    char *copy = malloc(i + 1);
    strncpy(copy, str + *j, i);
    copy[i] = '\0';
      if (strcmp(spec->length, "l") == 0) {
        long int *d = va_arg(*ap, long int *);
        *d = atol(copy);
      }
      else if (strcmp(spec->length, "ll") == 0){
        long long *d = va_arg(*ap, long long *);
        *d = atoll(copy);
      }
      else if (strcmp(spec->length,"h") == 0) {
        short *d = va_arg(*ap, short *);
        *d = atoi(copy);
      }
      else {
        int *d = va_arg(*ap, int *);
        *d = atoi(copy);
      }
      if (atol(copy) || atof(copy) || atoll(copy) || atoi(copy) || (copy[0] == '0'))
        success = 1;
      free(copy);
  }
  *j += i;
  return success;
}

int s21_isspace(char c) {
  return (c == ' ' || c == '\n' || c == '\t' || c == '\r');
}

int read_s(char *str, va_list *ap, struct specifier *spec, int *j)
{
  int success = 0;
  int i = 0;
  while (str[*j + i] != '\0' && str[*j + i] != '%' && !s21_isspace(str[*j + i])) {
    i++;
  }
  if (atoi(spec->width) < i && atoi(spec->width) != 0) {
    i = atoi(spec->width);
  }
  if (i > 0 && spec->width[0] != '*') {
    char *copy = malloc(i + 1);
    strncpy(copy, str + *j, i);
    copy[i] = '\0';

    char *new_str = va_arg(*ap, char *);
    strcpy(new_str, copy);
    if (new_str)
      success++;
    free(copy);
  }
  *j += i;
  return success;
}

int read_c(char *str, va_list *ap, struct specifier *spec, int *j)
{
  int success = 0;
  if (spec->width[0] != '*') {
    char *c = va_arg(*ap, char *);
    *c = str[*j];
    if (c)
      success = 1;
  }
  (*j)++;
  return success;
}

int read_u(char *str, va_list *ap, struct specifier *spec, int *j, char c) {
  int success = 0;
  int i  = 0;
  while (str[*j + i] != '\0' && !s21_isspace(str[*j + i]) && str[*j + i] != c && str[*j + i] != '%') {
    i++;
  }
  if (spec->width[0] != '0') {
    char *copy = malloc(i + 1);
    strncpy(copy, str + *j, i);
    copy[i] = '\0';
    unsigned int *u = va_arg(*ap, unsigned int *);
    *u = strtoul(copy, '\0', 10);
    if (strtoul(copy, '\0', 10)|| (copy[0] == '0'))
      success = 1;
    free(copy);
  }
  return success; 
}


int read_o(char *str, va_list *ap, struct specifier *spec, int *j, char c) {
  int i = 0;
  int success = 0;
  while (str[*j + i] != '\0' && !s21_isspace(str[*j + i]) && str[*j + i] != c && str[*j + i] != '%') {
    i++;
  }
  if (spec->width[0] != '0') {
    char *copy = malloc(i + 1);
    strncpy(copy, str + *j, i);
    copy[i] = '\0';
    int *o = va_arg(*ap, int *);
    *o = strtol(str, (char **)NULL, 8);
    if (strtol(str, (char **)NULL, 8)|| (copy[0] == '0'))
      success = 1;
    free(copy);
  }
  return success; 
}


int read_xX(char *str, va_list *ap, struct specifier *spec, int *j, char c) 
{
  int i = 0;
  int success = 0;
  while (str[*j + i] != '\0' && !s21_isspace(str[*j + i]) && str[*j + i] != c && str[*j + i] != '%') {
    i++;
  }
  if (spec->width[0] != '0') {
    char *copy = malloc(i + 1);
    strncpy(copy, str + *j, i);
    copy[i] = '\0';
    int *x = va_arg(*ap, int *);
    *x = strtol(copy, (char **)NULL, 16);
    //if X?????how to make upper case
    if (strtol(copy, (char **)NULL, 16)|| (copy[0] == '0'))
      success = 1;
    free(copy);
  }
  return success; 
}

int read_f(char *str, va_list *ap, struct specifier *spec,  int *j, char c) {
  int i = 0;
  int success = 0;
  while (str[*j + i] != '\0' && !s21_isspace(str[*j + i]) && str[*j + i] != c && str[*j + i] != '%') {
    i++;
  }
  if (spec->width[0] != '0') {
    char *copy = malloc(i + 1);
    strncpy(copy, str + *j, i);
    copy[i] = '\0';
    float *f = va_arg(*ap, float *);
    *f = atof(copy);
    if (strtol(copy, (char **)NULL, 16)|| (copy[0] == '0'))
      success = 1;
    free(copy);
  }
  return success; 
}

int read_i(char *str, va_list *ap, struct specifier *spec,  int *j, char c) {
  int i = 0;
  int success = 0;
  while (str[*j + i] != '\0' && !s21_isspace(str[*j + i]) && str[*j + i] != c && str[*j + i] != '%') {
    i++;
  }
  if (spec->width[0] != '0') {
    char *copy = malloc(i + 1);
    strncpy(copy, str + *j, i);
    copy[i] = '\0';
    int *i = va_arg(*ap, int *);
    if (copy[0] != '0') {
      *i = atoi(copy);
    } else {
      if (copy[1] == 'x') {
        *i = strtol(copy, (char **)NULL, 16);
      } else {
        *i = strtol(copy, (char **)NULL, 8);
      }
    }
    if (atoi(copy) ||  strtol(copy, (char **)NULL, 16) ||  strtol(copy, (char **)NULL, 8)|| (copy[0] == '0'))
      success = 1;
    free(copy);
  }
  return success; 
}

int match_str_and_format(char *str, struct specifier *spec, va_list *ap, int *j, char c) {
  int success = 0;
  switch (spec->type) {
    case 'd': ;
      success = read_d(str, ap, spec, j, c);
      break;
    case 'c': ;
      success = read_c(str, ap, spec, j);
      break;
    case 'i': ;
      success = read_i(str, ap, spec, j, c);
      break;
    case 'e': ;
      break;
    case 'E': ;
      break;
    case 'f': ;
      success = read_f(str, ap, spec, j, c);
      break;
    case 'g': ;
      break;
    case 'G': ;
      break;
    case 'o': ;
      success = read_o(str, ap, spec, j, c);
      break;
    case 's': ;
      success = read_s(str, ap, spec, j);
      break;
    case 'u': ;
      success = read_u(str, ap, spec, j, c);
      break;
    case 'x': ;
      success = read_xX(str, ap, spec, j, c);
      break;
    case 'X': ;
      success = read_xX(str, ap, spec, j, c);
      break;
    case 'p': ;
      break;
    case 'n': ;
      int *n = va_arg(*ap, int *);
      *n = *j;
      break;
  }
  return success;
}

// int main() {
  
//   char fstr[] = "%d %d";
//   char str[] = "  12321  -111";
//   int a1 = 0, a2 = 0, b1 = 0, b2 = 0;
//   int res1 = s21_sscanf(str, fstr, &a1, &b1);
//   int res2 = sscanf(str, fstr, &a2, &b2);

//     printf("s21:  %d  %d  \n", a1, b1);
//     printf("ss:   %d  %d  \n", a2, b2);
//     printf("s21_res:  %d\n", res1);
//     printf("ss_res:   %d\n", res2);
// }