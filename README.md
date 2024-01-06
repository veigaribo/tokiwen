# Tokiwen

Software development environment to assist in the study of basic computer programming and computer architectures.
Available (hopefully) at <https://tokiwen.veigo.dev/>.

Developed for the Análise e Desenvolvimento de Sistemas technologist degree at IFRS Campus Rio Grande.

A document explaining the workings of this project in Portuguese is available at <https://drive.google.com/file/d/1NSdduIqDJz2b0K4baEaD28eVpL57ZU6q/view>

## Some example programs:

Determines how many successive Collatz applications until some number becomes 1

```
int number;
read number;

int count = 0;

while (number != 1) {
  if (number % 2 == 0) {
    number /= 2;
  } else {
    number = number * 3 + 1;
  }

  count += 1;
}

write count;
```

Determines if some number is prime

```
int x;
read x;

int i = 2;
int primo = 1;

if (x == 1) {
  primo = 0;
  goto end;
}

while (i <= x / 2) {
  if (x % i == 0) {
    primo = 0;
    goto end;
  }

  i += 1;
}

end:
write primo;
```
