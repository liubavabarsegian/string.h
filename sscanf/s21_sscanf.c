#include "s21_sscanf.h"

int s21_sscanf(const char *str, const char *format, ...) {
  const char *types = "cdieEfgGosuxXpn%%";
  va_list ap;
  va_start(ap, format);
  struct specifier spec;
  char *string = malloc((strlen(str) + 1) * sizeof(char));
  strcpy(string, str);
  int success = 0;
  int count_successes = 0;
  if ((string[0] == '\0' || s21_empty_str(string)) && format[0] != '\0')
    count_successes = -1;
  int j = 0;
  char c = '\0';

  if (!s21_empty_str(string) && string[0] != '\0') {
    for (size_t i = 0; i < strlen(format) && (size_t)j < strlen(string); i++) {
      specifier_init(&spec);
      if (format[i] && s21_isspace(format[i])) {
        while (s21_isspace(string[j]) && string[i]) {
          j++;
        }
      }
      while (format[i] && s21_isspace(format[i])) {
        i++;
      }
      if (format[i] != '%' && (format[i] == string[j])) {
      } else if (format[i] != '%' && (format[i] != string[j]) &&
                 !s21_isspace(string[j])) {
        break;
      } else if (format[i] == '%' && format[i + 1] == '%') {
        i++;
      } else if (format[i] == '%' && string[j] == '%') {
        i++;
        j++;
      } else if (format[i] && string[j] && format[i] == '%' && string[j]) {
        specifier_parsing((char *)&format[i + 1], &spec);
        i += strcspn(&format[i + 1], types) + 1;
        while (s21_isspace(string[j]) && spec.type != 'c') {
          j++;
        }
        if (format[i]) {
          c = format[i + 1];
        }
        success = match_str_and_format(string, &spec, &ap, &j, c);
        if (success) {
          count_successes++;
          if (spec.width[0] == '*') {
            count_successes--;
          }
        } else if (!success &&
                   (spec.type == 'i' || spec.type == 'd' || spec.type == 'u' ||
                    spec.type == 'o' || spec.type == 'x' || spec.type == 'X')) {
          break;
        }
        j--;
      }
      j++;
    }
  }
  if (string) free(string);
  return count_successes;
}

void specifier_parsing(char *str, struct specifier *spec) {
  char *buff = str;
  char *buff1 = malloc(1024);
  char *buff2 = buff1;
  int k = 0;
  const char *flags = "-+ #0";
  const char *numbers = "1234567890*";
  const char *length = "hlL";
  const char *types = "cdieEfgGosuxXpn%%";
  spec->type = str[strcspn(str, types)];
  pointer_shift(&buff, buff1, flags);
  for (int i = 0; i < 5; i++) {
    if (strchr(buff1, flags[i]) != NULL &&
        ((flags[i] == ' ' && strchr(buff1, '+') == NULL) ||
         (flags[i] == '0' && strchr(buff1, '-') == NULL) || flags[i] == '-' ||
         flags[i] == '+' || flags[i] == '#'))
      spec->flag[k++] = flags[i];
  }
  spec->flag[k] = '\0';
  k = 0;
  pointer_shift(&buff, buff1, numbers);
  numbers_parsing(spec->width, buff1);
  if (*buff == '.') {
    buff++;
    pointer_shift(&buff, buff1, numbers);
    if (*buff1 == '\0')
      strcpy(spec->precision, "0");
    else
      numbers_parsing(spec->precision, buff1);
  }
  memcpy(buff1, buff, strspn((const char *)buff, length));
  for (int i = 0; i < 3; i++) {
    char *c = strchr(buff1, length[i]);
    if (c != NULL) {
      if (length[i] == 'L' ||
          (length[i] == 'h' &&
           (c < strchr(buff1, 'l') || strchr(buff1, 'l') == NULL)) ||
          (length[i] == 'l' && strchr(c + 1, length[i]) == NULL &&
           spec->length[0] != 'h')) {
        spec->length[k++] = length[i];
      } else if (length[i] == 'l' && strchr(c + 1, length[i]) != NULL &&
                 spec->length[0] != 'h') {
        spec->length[k++] = length[i];
        spec->length[k++] = length[i];
      }
    }
  }
  spec->length[k] = '\0';
  free(buff2);
}

void numbers_parsing(char *str, char *buff) {
  size_t length = strspn((const char *)buff, "1234567890");
  if (*buff == '*' && length == 0) length = 1;
  strcpy(str, buff);
  buff[length + 1] = '\0';
}

void pointer_shift(char **buff, char *buff1, const char *str) {
  size_t length = strspn((const char *)*buff, str);
  strncpy(buff1, *buff, length);
  buff1[length] = '\0';
  *buff += length;
}

void specifier_init(struct specifier *spec) {
  spec->flag[0] = '\0';
  spec->width[0] = '\0';
  spec->precision[0] = '\0';
  spec->length[0] = '\0';
  spec->type = 0;
}

int read_d(char *str, va_list *ap, struct specifier *spec, int *j, char c) {
  int success = 0;
  int i = 0;
  while (str[*j + i] != '\0' && !s21_isspace(str[*j + i]) && str[*j + i] != c &&
         str[*j + i] != '%' && s21_isdigit(str[*j + i])) {
    if (i > 0 && (str[*j + i] == '-' || str[*j + i] == '+')) {
      break;
    }
    i++;
  }
  if (atoi(spec->width) != 0 && atoi(spec->width) < i) {
    i = atoi(spec->width);
  }
  char *copy = malloc(i + 1);
  strncpy(copy, str + *j, i);
  copy[i] = '\0';
  if (spec->width[0] != '*') {
    if (strcmp(spec->length, "l") == 0) {
      long int *d = va_arg(*ap, long int *);
      *d = s21_atol(copy);
    } else if (strcmp(spec->length, "ll") == 0) {
      long long *d = va_arg(*ap, long long *);
      *d = s21_atoll(copy);
    } else if (strcmp(spec->length, "h") == 0) {
      short *d = va_arg(*ap, short *);
      *d = atoi(copy);
    } else {
      int *d = va_arg(*ap, int *);
      *d = atoi(copy);
    }
    if ((copy) && (s21_atol(copy) || s21_atoll(copy) || atoi(copy) ||
                   ((strcmp(copy, "0") == 0 || strcmp(copy, "-0") == 0 ||
                     strcmp(copy, "+0") == 0) &&
                    atoi(copy) == 0)))
      success = 1;
  } else if (copy) {
    success = 1;
  }
  free(copy);
  *j += i;
  return success;
}

int read_s(char *str, va_list *ap, struct specifier *spec, int *j) {
  int success = 0;
  int i = 0;
  while (str[*j + i] != '\0' && !s21_isspace(str[*j + i])) {
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
    if (new_str) success++;
    free(copy);
  }
  *j += i;
  return success;
}

int read_c(char *str, va_list *ap, struct specifier *spec, int *j) {
  int success = 0;
  if (spec->width[0] != '*') {
    char *c = va_arg(*ap, char *);
    *c = str[*j];
    if (c) success = 1;
  }
  (*j)++;
  return success;
}

int read_u(char *str, va_list *ap, struct specifier *spec, int *j, char c) {
  char *end = NULL;
  int success = 0;
  int i = 0;
  while (str[*j + i] != '\0' && !s21_isspace(str[*j + i]) && str[*j + i] != c &&
         str[*j + i] != '%' && s21_isdigit(str[*j + i])) {
    if (i > 0 && (str[*j + i] == '-' || str[*j + i] == '+')) {
      break;
    }
    i++;
  }
  if (atoi(spec->width) && atoi(spec->width) < i) {
    i = atoi(spec->width);
  }
  if (spec->width[0] != '*') {
    char *copy = malloc(i + 1);
    strncpy(copy, str + *j, i);
    copy[i] = '\0';
    if (strcmp(spec->length, "l") == 0) {
      unsigned long int *u = va_arg(*ap, unsigned long int *);
      *u = s21_strtoul(copy, &end, 10);
    } else if (strcmp(spec->length, "ll") == 0) {
      unsigned long long int *u = va_arg(*ap, unsigned long long int *);
      *u = s21_strtoull(copy, &end, 10);
    } else {
      unsigned int *u = va_arg(*ap, unsigned int *);
      *u = s21_strtoul(copy, &end, 10);
    }
    if (s21_strtoul(copy, &end, 10) || strcmp(copy, "0") == 0) {
      success = 1;
    }
    free(copy);
  }
  *j += i;
  return success;
}

int read_o(char *str, va_list *ap, struct specifier *spec, int *j, char c) {
  int i = 0;
  int success = 0;
  while (str[*j] == '0') (*j)++;
  while (str[*j + i] != '\0' && !s21_isspace(str[*j + i]) && str[*j + i] != c &&
         str[*j + i] != '%' && s21_isoctal(str[*j + i])) {
    i++;
  }
  if (atoi(spec->width) && atoi(spec->width) < i) {
    i = atoi(spec->width);
  }
  if (spec->width[0] != '*') {
    if (str[*j - 1] == '0' && i == 0) {
      // i = 1;
      success++;
    }
    char *copy = malloc(i + 1);
    strncpy(copy, str + *j, i);
    copy[i] = '\0';
    if (strcmp(spec->length, "l") == 0) {
      long int *d = va_arg(*ap, long int *);
      *d = s21_strtol(copy, (char **)NULL, 8);
    } else if (strcmp(spec->length, "ll") == 0) {
      long long *d = va_arg(*ap, long long *);
      *d = s21_strtoll(copy, (char **)NULL, 8);
    } else if (strcmp(spec->length, "h") == 0) {
      short *d = va_arg(*ap, short *);
      *d = s21_strtol(copy, (char **)NULL, 8);
    } else {
      int *d = va_arg(*ap, int *);
      *d = s21_strtol(copy, (char **)NULL, 8);
    }
    if (s21_strtol(copy, (char **)NULL, 8) || (copy[0] == '0')) success = 1;
    free(copy);
  }
  *j += i;
  return success;
}

int read_xX(char *str, va_list *ap, struct specifier *spec, int *j, char c) {
  int i = 0;
  int success = 0;
  while (str[*j + i] != '\0' && !s21_isspace(str[*j + i]) && str[*j + i] != c &&
         str[*j + i] != '%' && s21_ishex(str[*j + i])) {
    i++;
  }
  if (atoi(spec->width) && atoi(spec->width) < i) {
    i = atoi(spec->width);
  }
  if (spec->width[0] != '*') {
    char *copy = malloc(i + 1);
    strncpy(copy, str + *j, i);
    copy[i] = '\0';

    if (strcmp(spec->length, "l") == 0) {
      unsigned long int *x = va_arg(*ap, unsigned long int *);
      *x = s21_strtol(copy, (char **)NULL, 16);
    } else if (strcmp(spec->length, "ll") == 0) {
      unsigned long long int *x = va_arg(*ap, unsigned long long int *);
      *x = s21_strtol(copy, (char **)NULL, 16);
    } else {
      unsigned int *x = va_arg(*ap, unsigned int *);
      *x = s21_strtol(copy, (char **)NULL, 16);
    }
    // if X?????how to make upper case
    if (s21_strtol(copy, (char **)NULL, 16) || (copy[0] == '0')) success = 1;
    free(copy);
  }
  *j += i;
  return success;
}

int read_f(char *str, va_list *ap, struct specifier *spec, int *j, char c) {
  int i = 0;
  int success = 0;
  while (str[*j + i] != '\0' && !s21_isspace(str[*j + i]) && str[*j + i] != c &&
         str[*j + i] != '%') {
    i++;
  }
  if (atoi(spec->width) && atoi(spec->width) < i) {
    i = atoi(spec->width);
  }
  if (spec->width[0] != '*') {
    char *copy = malloc(i + 1);
    strncpy(copy, str + *j, i);
    copy[i] = '\0';
    if (strcmp(spec->length, "L") == 0) {
      long double *f = va_arg(*ap, long double *);
      *f = s21_atold(copy);
    } else if (strcmp(spec->length, "l") == 0) {
      double *f = va_arg(*ap, double *);
      *f = s21_atold(copy);
    } else {
      float *f = va_arg(*ap, float *);
      *f = s21_atold(copy);
    }
    if (s21_atold(copy) || (copy[0] == '0')) success = 1;
    free(copy);
  }
  *j += i;
  return success;
}

int read_g(char *str, va_list *ap, struct specifier *spec, int *j, char c) {
  read_f(str, ap, spec, j, c);
}

int read_e(char *str, va_list *ap, struct specifier *spec, int *j, char c) {
  read_f(str, ap, spec, j, c);
}

int read_i(char *str, va_list *ap, struct specifier *spec, int *j, char c) {
  int i = 0;
  int success = 0;
  while (str[*j + i] != '\0' && !s21_isspace(str[*j + i]) && str[*j + i] != c &&
         str[*j + i] != '%') {
    i++;
  }
  if (atoi(spec->width) && atoi(spec->width) < i) {
    i = atoi(spec->width);
  }
  if (spec->width[0] != '*') {
    char *copy = malloc(i + 1);
    strncpy(copy, str + *j, i);
    copy[i] = '\0';
    int sign = 0;
    if (copy[0] == '-') {
      sign = 1;
    }
    if (copy[sign] != '0') {
      success = read_d(str, ap, spec, j, c);
    } else {
      if (copy[1 + sign] == 'x' || copy[1 + sign] == 'X') {
        success = read_xX(str, ap, spec, j, c);
      } else {
        success = read_o(str, ap, spec, j, c);
      }
    }
    if (s21_atoll(copy) || s21_strtol(copy, (char **)NULL, 16) ||
        s21_strtol(copy, (char **)NULL, 8) || (copy[0] == '0'))
      success = 1;
    free(copy);
  }
  return success;
}

int match_str_and_format(char *str, struct specifier *spec, va_list *ap, int *j,
                         char c) {
  int success = 0;
  switch (spec->type) {
    case 'd':;
      success = read_d(str, ap, spec, j, c);
      break;
    case 'c':;
      success = read_c(str, ap, spec, j);
      break;
    case 'i':;
      success = read_i(str, ap, spec, j, c);
      break;
    case 'e':
      success = read_e(str, ap, spec, j, c);
      break;
    case 'E':;
      success = read_e(str, ap, spec, j, c);
      break;
    case 'f':;
      success = read_f(str, ap, spec, j, c);
      break;
    case 'g':
      success = read_g(str, ap, spec, j, c);
      break;
    case 'G':;
      success = read_g(str, ap, spec, j, c);
      break;
    case 'o':
      success = read_o(str, ap, spec, j, c);
      break;
    case 's':;
      success = read_s(str, ap, spec, j);
      break;
    case 'u':;
      success = read_u(str, ap, spec, j, c);
      break;
    case 'x':;
      success = read_xX(str, ap, spec, j, c);
      break;
    case 'X':;
      success = read_xX(str, ap, spec, j, c);
      break;
    case 'p':;
      break;
    case 'n':;
      int *n = va_arg(*ap, int *);
      *n = *j;
      break;
  }
  return success;
}

// int main() {
// char fstr[] = "%c %c %c %c";
//   char str[] = "z ' ' /";
//   int a1 = 0, a2 = 0, b1 = 0, b2 = 0, c1 = 0, c2 = 0, d1 = 0, d2 = 0;

//   int res1 = s21_sscanf(str, fstr, &a1, &b1, &c1, &d1);
//   int res2 = sscanf(str, fstr, &a2, &b2, &c2, &d2);
//   printf("res21: %d %d %d %d %df\n", res1, a1, b1, c1, d1);
//   printf("resss: %d %d %d %d %d\n", res2, a2, b2, c2, d2);
// }