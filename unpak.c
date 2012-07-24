// gcc 
// http://debian.fmi.uni-sofia.bg/~sergei/cgsr/docs/pak.txt
/*

Quake PAK Format

Figured out by Pete McCormick (petra@force.net)  I am not responsible for any damage this does, enjoy, and please email me any comments!
Pete

=Format=
Header
(4 bytes) signature = 'PACK' 
(4 bytes, int) directory offeset
(4 bytes, int) directory lenght

Directory
(56 bytes, char) file name
(4 bytes, int) file position
(4 bytes, int) file lenght

File at each position (? bytes, char) file data
Description - The signature must be present in all PAKs; it's the way Quake knows its a real PAK file. The directory offset is where the directory listing starts, and the lenght is its               lenght. In the actuall directory listing, the three options, 56 bytes of a name, the files position and lenght, are repeating until the running total of the length (increment               by 64) is reached. If the directory lenght mod 64 is not a even number, you know their is something wrong. And directories are just handled by a making the name something like "/foobar/yeahs.txt". Short and simple.  

Tips - Put the directory entry at the end of the file, so if you added a file, you'd just rewrite the small directory entry instead of all the data.  

Limits - Unknown file limit. Don't create too many though :) I would think around a 1000, prehaps 2000 (in which case, 2048 would be reasonible) 

Quake1 specifics on PAK files (content wise)  If the file '/gfx/pop.lmp' is present and contains a Quake logo (extract from your own pak1.pak file), Quake considers the file a registered. Otherwise, it's a shareware. If it's shareware, the ending screen in 'end1.b!
in' is displayed (character, screen buffer colour format). If registered, end2.bin is displayed. 

Quake2 specifics on PAK files (content wise)  I have no clue.

*/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct pak_directory
{
    char file_name[56];
    uint32_t file_position;
    uint32_t file_length;
};

struct pak_header
{
    char signature[4];
    uint32_t directory_offset;
    uint32_t directory_length;
    struct pak_directory directories[1];
};

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <filename.pak>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    FILE* input = fopen(argv[1], "rb");
    fseek(input, 0, SEEK_END);
    long size = ftell(input);
    char* data = malloc(size);
    rewind(input);
    fread(data, size, 1, input);
    fclose(input);
    
    struct pak_header* header = (struct pak_header*)data;

    printf("header: %c%c%c%c offset: %d length: %d\n",
        header->signature[0],
        header->signature[1],
        header->signature[2],
        header->signature[3],
        header->directory_length,
        header->directory_offset);
        
    char* cwd = getcwd(0, 0);
    puts(cwd);
    
    // Default return status
    int status = EXIT_SUCCESS;
        
    for (uint32_t i = header->directory_offset; 1;)
    {
        struct pak_directory* directory = (struct pak_directory*)&data[i];
        printf("Directory: pos: %d len: %d name: %s\n",
            directory->file_position,
            directory->file_length,
            directory->file_name);

        char* folders = strdup(directory->file_name);
        
        for (char* dir = strtok(folders, "/"); dir; dir = strtok(NULL, "/"))
        {
            puts(dir);
            mkdir(dir);
            chmod(dir, 0755);
            if (chdir(dir))
            {
                FILE* output = fopen(dir, "wb");                
                if (!output)
                {
                    fprintf(stderr, "Error opening %s for writing.\n", dir);
                    status = EXIT_FAILURE;
                    goto cleanup;
                }
                fwrite(&data[directory->file_position], directory->file_length, 1, output);
                fclose(output);
                
            }
        }
        free(folders);   
        

        
        chdir(cwd);
        
        i = directory->file_position + directory->file_length;
        

    }
    
cleanup:
    free(cwd);
    free(data);
    return status;
}




