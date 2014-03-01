#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <endian.h>

void fatal(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	exit(1);
}

int64_t find_offset(FILE* in_file)
{
	char pattern[4] = {0x55, 0xaa, 0x06, 0x43};
	int64_t out_offset = -1;
	int found = 0;
	char buffer[512];
	fseek(in_file, 0, SEEK_SET);
	uint32_t cur_offset = 0;
	do
	{
		uint32_t bytes_read = fread(&buffer, 1, 512, in_file);
		if(bytes_read == 0)
			break;
		int i = 0;
		for(i = 0; i < 4; ++i)
		{
			if(buffer[i] != pattern[i])
				break;
		}
		if(i == 4)
		{
			uint32_t swap_offset = be32toh(*((uint32_t*)&buffer[4]));
			printf("Swap offset: %u Cur Offset: %u\n", swap_offset, cur_offset);
			if(swap_offset == cur_offset)
			{
				found = 1;
				out_offset = cur_offset;
			}
		}
		cur_offset += bytes_read;
	}while(found == 0);
	return out_offset;
}

void copy_chunked(FILE* in, FILE* out, int64_t offset, int64_t size)
{
	uint32_t real_offset = (uint32_t)offset;
	fseek(in, real_offset, SEEK_SET);
	uint32_t real_size = (uint32_t)size;
	if(size < 0)
	{
		fseek(in, 0, SEEK_END);
		real_size = ftell(in);
		real_size -= real_offset;
		fseek(in, real_offset, SEEK_SET);
	}
	char buffer[512];
	while(real_size >= 512)
	{
		fread(&buffer, 1, 512, in);
		fwrite(&buffer, 1, 512, out);
		real_size -= 512;
	}
	if(real_size > 0)
	{
		fread(&buffer, 1, real_size, in);
		fwrite(&buffer, 1, real_size, out);
	}
}

void split_file(FILE* in_file, char* filename, int64_t offset)
{
	int path_len = strlen(filename);
	char* kernel_name = malloc(path_len+1+7);
	char* image_name = malloc(path_len+1+6);
	strncpy(kernel_name, filename, path_len);
	strncpy(kernel_name+path_len, ".kernel", 7);
	kernel_name[path_len+7] = 0;
	strncpy(image_name, filename, path_len);
	strncpy(image_name+path_len, ".image", 6);
	image_name[path_len+6] = 0;
	printf("Kernel path: %s\nImage path: %s\n", kernel_name, image_name);

	FILE* kernel_file = fopen(kernel_name, "wb");
	if(!kernel_file)
		fatal("Could not open kernel file for writing: %s\n", kernel_name);
	copy_chunked(in_file, kernel_file, 0, offset);
	fclose(kernel_file);

	FILE* image_file = fopen(image_name, "wb");
	if(!image_file)
		fatal("Could not open image file for writng: %s\n", image_name);
	copy_chunked(in_file, image_file, offset+8, -1);
	fclose(image_file);

	free(kernel_name);
	free(image_name);
}

int main(int argc, char* argv[])
{
	if(argc < 2)
		fatal("Usage: %s <image file>\n", argv[0]);

	FILE* in_file = fopen(argv[1], "rb");
	if(!in_file)
		fatal("Could not open input image for reading.\n");

	int64_t offset = find_offset(in_file);
	if(offset < 0)
		fatal("Could not find image split offset. Is this a JunOS combined image?\n");

	split_file(in_file, argv[1], offset);
	return 0;
}