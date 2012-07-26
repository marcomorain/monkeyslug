static void fatal(const char* message, ...)
{
    va_list argp;
    va_start(argp, message);
    fprintf(stderr, "Fatal error: ");
    vfprintf(stderr, message, argp);
    fputs("\n", stderr);
    va_end(argp);
    exit(EXIT_FAILURE);
}

static char* read_entire_file(const char* filename)
{
    FILE* input = fopen(filename, "rb");
    if (!input) fatal("Error reading %s\n", filename);
    fseek(input, 0, SEEK_END);
    long size = ftell(input);
    char* data = malloc(size);
    rewind(input);
    fread(data, size, 1, input);
    fclose(input);
    return data;
}
