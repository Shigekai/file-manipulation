# **Usando Makefile**

### Compilação

```bash
make

make run

make clean
```

### Execução

```bash
./exec/imageDatabase
```

# Manualmente

### Compilação

```bash
mkdir -p exec
gcc -Wall -Wextra -std=c11 -O2 -D_FILE_OFFSET_BITS=64 \
    -Iheaders -Isrc/database -Isrc/index -Isrc/io \
    -Isrc/handleImages -Isrc/menu -Isrc/algos \
    main.c \
    src/database/database.c \
    src/index/index.c \
    src/io/pgm.c \
    src/handleImages/filters.c \
    src/handleImages/handleImages.c \
    src/menu/menu.c \
    src/algos/externalSort.c \
    -o exec/imageDatabase
```

### Execução

```bash
./exec/imageDatabase
```
