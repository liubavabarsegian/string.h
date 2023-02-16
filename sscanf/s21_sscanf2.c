#include "s21_sscanf2.h"

void s21_sprintf(char *str, const char *format, ...) {
  const char* types = "cdieEfgGosuxXpn%%";
  va_list ap;
  va_start(ap, format);
  struct specifier spec;

  char *format_str = malloc(sizeof(char) * (strlen(format) + 1));
  strcpy(format_str, format);
  for (size_t i = 0; i < strlen(format); i++) {
    specifier_init(&spec);
    if (format[i] == '%') {
      // printf("%s\n", format_str);
      i += strcspn(&format[i + 1], types) + 1;
      // specifier_parsing(format_str, &spec);
      printf("%s %s %s %s %c\n", spec.flag, spec.length, spec.precision, spec.width, spec.type);
    }
  }
}

void specifier_parsing(char *str, struct specifier* spec) {
  char* buff = str;
  char* buff1 = malloc(1024);
  int k = 0;
  const char* flags = "-+ #0";
  const char* numbers = "1234567890*";
  const char* length = "hlL";
  const char* types = "cdieEfgGosuxXpn%%";
  size_t spec_length = strcspn(str, types);
  spec->type = str[spec_length];
  pointer_shift(&spec_length, buff, buff1, flags);
  for (int i = 0; i < 5; i++) {
    if (strchr(buff1, flags[i]) != NULL && ((flags[i] == ' ' && strchr(buff1, '+') == NULL) || (flags[i] == '0' && strchr(buff1, '-') == NULL)))
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
  memcpy(buff1, buff, strspn((const char*) buff, length));
  for (int i = 0; i < 3; i++) {
    char* c = strchr(buff1, length[i]);
    if (c != NULL) {
      if (length[i] == 'L' || (length[i] == 'h' && c < strchr(buff1, 'l')) || (length[i] == 'l' && strchr(c, length[i]) == NULL && spec->length[0] != 'h')) {
        spec->length[k++] = length[i];
      } else if (length[i] == 'l' && strchr(c, length[i]) != NULL && spec->length[0] != 'h') {
        spec->length[k++] = length[i];
        spec->length[k++] = length[i];
      } 
    }
  }
  spec->length[k] = '\0';
  free(buff1);
}

void numbers_parsing(char* str, char* buff) {
  size_t length = strspn((const char*) buff, (const char*) "1234567890");
  if (*buff == '*' && length == 0)
    length = 1;
  memcpy(str, buff, length);
}

void pointer_shift(size_t* length, char* buff, char* buff1, const char* str) {
  *length = strspn((const char*) buff, str);
  memcpy(buff1, buff, *length);
  buff += *length;
}

void vararg_init(char type, va_list *ap) {
  if (type == 'c') {
    va_arg(*ap, int);
  } else if (type == 'd' || type == 'i') {
    va_arg(*ap, int);
  } else if (type == 'e' || type == 'E' || type == 'f' || type == 'g' || type == 'G' ) {
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

void specifier_init(struct specifier* spec) {
  spec->flag[0] = '\0';
  spec->width[0] = '\0';
  spec->precision[0] = '\0';
  spec->length[0] = '\0';
  spec->type = 0;
}

int main() {
  // char test[] = "%s";
  char str[100];
  s21_sprintf("hey 1", "%s%f", str);
}